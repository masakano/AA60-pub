//
// Camera :
//
#include <gsys/node/camera.h>

namespace spu::gs_node {

namespace {

constexpr int16_t c_shortcut_adjust = 'a';
constexpr int16_t c_shortcut_save = 's';
constexpr int16_t c_shortcut_load = 'l';
constexpr float c_lerp_close = 0.001;

bool is_pick(const SpuGesture *gesture) { return !gesture->prev().mouse_R && gesture->curr().mouse_R; }
bool is_drop(const SpuGesture *gesture) { return gesture->prev().mouse_R && !gesture->curr().mouse_R; }
bool is_drag(const SpuGesture *gesture) { return gesture->prev().mouse_R && gesture->curr().mouse_R; }
bool is_shift(const SpuGesture *gesture) { return gesture->curr().key_shift; }
}  // namespace

void Camera::init(const Attrs &attrs)
{
	ICamera::init(attrs);
	auto *canvas = getCanvas();
	if (canvas) {  // init before set()
		getTargetSubstance() = getASubstance() = canvas->viewworld(0);
	}
	setProperty(e_active, 1);
	setProperty(e_lazy, 0);
	set(attrs);
}

bool Camera::doSync(bool is_nonblock)
{
	if (!is_nonblock) {
		auto &target_substance = getTargetSubstance();
		target_substance = getCanvas()->viewworld(0);
		auto up = Vec3f(m_referencePlane.eq);
		target_substance.q = target_substance.q.stabilize(up);
		getASubstance() = target_substance;
		recordAnchor();
	}
	return GsNode::doSync(is_nonblock);
}

void Camera::set(const Attrs &attrs)
{
	ICamera::set(attrs);
	attrs.peek("watch_distance", "deprecated");
	attrs.apply("mode", m_mode);
	attrs.apply("watch_node", m_watch.node);
	attrs.apply("watch_range", m_watch.range);

	attrs.apply<vec4f_t>("reference_plane", m_referencePlane.eq);
	attrs.apply("intertia", m_inertia);
	m_referencePlane.eq /= length(Vec3f(m_referencePlane.eq));  // safety

	if (attrs.peek({"eye", "dir", "up", "fovy", "aspect", "near", "far"})) {
		attrs.report("attrs");
		aux_error(true, "set them via canvas then call sync(false)\n");
	}

	if (attrs.get("auto_adjust", false)) {
		autoAdjust();
	}
}

void Camera::recordAnchor()
{
	m_anchor.cursor = Vec2f(getGesture()->curr().cursor);
	m_anchor.substance = getASubstance();
	m_anchor.composition = *getCanvas();

	if (m_watch.node) {
		m_anchor.point = Range3f(m_watch.node->points()).center();
	}
	else if (m_watch.range) {
		m_anchor.point = m_watch.range->center();
	}
	else {
		auto depth = 0.0f;
		getDepthCanvas()->peek(m_anchor.cursor.x, m_anchor.cursor.y, "depth", &depth);
		if (depth < 1.0) {
			auto *canvas = getCanvas();
			auto fragscreen = canvas->fragscreen(0);
			auto worldscreen = canvas->worldscreen(0);
			auto fragworld = worldscreen.inverse() * fragscreen;
			m_anchor.point = fragworld.pers3(Vec3f(m_anchor.cursor, depth));
		}
		else {
			m_anchor.point = Vec3f(0);  // last resort
		}
	}
}

void Camera::update()
{
	ICamera::update();

	auto camera_substance = Transformf(getASubstance());
	auto *gesture = getGesture();
	auto &target_substance = getTargetSubstance();

	// deactivate
	if (!getProperty(e_active)) {
		/* do nothing */
	}
	// wheel
	else if (gesture->wheel()) {
		recordAnchor();
		doWheel();
	}
	// pick
	else if (is_pick(gesture)) {
		recordAnchor();
		m_action = is_shift(gesture) ? e_translate : e_rotate;
	}
	// drop
	else if (m_action && is_drop(gesture)) {
		recordAnchor();
		m_action = e_none;
	}
	// drag
	else if (m_action && is_drag(gesture)) {
		if (gesture->pressed(c_shortcut_adjust)) {
			m_action = 0;
			autoAdjust();
		}
		else if (gesture->pressed(c_shortcut_save)) {
			m_action = 0;
			doSave();
		}
		else if (gesture->pressed(c_shortcut_load)) {
			m_action = 0;
			doLoad();
		}
		else {
			doDrag();
			camera_substance = target_substance;  // immediate
		}
	}
	else if (
	        distance(camera_substance.t, target_substance.t) > c_lerp_close
	        || distance(camera_substance.q, target_substance.q) > c_lerp_close) {
		auto dq0 = camera_substance;
		auto dq1 = target_substance;
		camera_substance = lerp(dq0, dq1, m_inertia);
	}

	getASubstance() = camera_substance;
	auto &worldviews = getCanvas()->getWorldviews();

	worldviews.push_front(camera_substance.inverse());
	worldviews.pop_back();
}

void Camera::autoAdjust()
{
	auto up = Vec3f(m_referencePlane.eq);
	auto dir = m_anchor.substance.q * -ez();
	auto watch_range = Range3f(Vec3f(-1.0), Vec3f(+1.0));  // last resort
	if (m_watch.node) {
		watch_range = Range3f(m_watch.node->points());
	}
	else if (m_watch.range) {
		watch_range = *m_watch.range;
	}

	auto composition = Composition(*getCanvas());
	composition.adjustCamera(0, watch_range, up, dir);

	auto viewworld = composition.viewworld(0);
	auto eye = Vec3f(viewworld.c[3]);

	float max_d;
	m_referencePlane.support(watch_range.points(), nullptr, &max_d);

	Vec3f base;
	auto d = m_referencePlane.distance(eye, &base);
	if (d < max_d) {
		eye = base + max_d * Vec3f(m_referencePlane.eq);
	}
	dir = normalize(-eye);
	viewworld.set_orientation(&eye, &dir, &up);
 	getTargetSubstance() = viewworld;
	recordAnchor();
}

void Camera::doSave()
{
	aux_message(0, "save to 'camera.dat'\n");
	const char *c_path = "camera.dat";
	serialize_to_file(c_path, *this);
}

void Camera::doLoad()
{
	aux_message(0, "load from 'camera.dat'\n");
	const char *c_path = "camera.dat";
	deserialize_from_file(c_path, *this);
}

void Camera::doWheel()
{
	auto *gesture = getGesture();
	auto footstep = m_referencePlane.distance(m_anchor.substance.t);
	auto scale = is_shift(gesture) ? 0.50f : 0.05f;
	scale = gesture->wheel() > 0 ? -scale : scale;

	auto eye = m_anchor.substance.t;
	auto dir = m_anchor.substance.q * -ez();
	auto up = Vec3f(m_referencePlane.eq);

	auto d = dot(dir, up);
	auto r = 0.5f * cosf(std::abs(d) * pi()) + 0.5f;  // coserp

	dir = normalize(dir - r * d * up);
	eye = eye + dir * scale * footstep;
	getTargetSubstance() = Transformf(eye, m_anchor.substance.q);
}

void Camera::doDrag()
{
	auto fragscreen = getCanvas()->fragscreen(0);
	auto anchor_screen = fragscreen.ortho3(m_anchor.cursor);
	auto cursor_screen = fragscreen.ortho3(Vec2f(getGesture()->curr().cursor));
	auto &target_substance = getTargetSubstance();

	if (m_action == e_translate) {
		auto anchor_worldscreen = m_anchor.composition.worldscreen(0);
		auto anchor_ray = unproject(anchor_screen, anchor_worldscreen);
		auto current_ray = unproject(cursor_screen, anchor_worldscreen);
		auto target_plane = getTargetPlane(anchor_ray);

		auto current_t = 0.0f;
		auto anchor_t = 0.0f;

		if (target_plane.intersect(current_ray, &current_t)
		    && target_plane.intersect(anchor_ray, &anchor_t)) {
			auto current_point = current_ray.eye + current_ray.dir * current_t;
			auto anchor_point = anchor_ray.eye + anchor_ray.dir * anchor_t;
			auto delta = current_point - anchor_point;
			target_substance = Transformf(m_anchor.substance.t - delta, m_anchor.substance.q);
		}
	}
	else if (m_action == e_rotate) {
		auto delta_screen = cursor_screen - anchor_screen;
		auto axis = Vec3f(-delta_screen.y, delta_screen.x, 0);
		auto radian = length(axis) * pi();
		auto up = Vec3f(m_referencePlane.eq);

		if (m_mode == e_orbital) {
			auto rot = Quatf(radian, m_anchor.substance.q * axis).conj();
			auto trt = Transformf(m_anchor.point, rot, -m_anchor.point);
			target_substance = trt * m_anchor.substance;
			target_substance.q = target_substance.q.stabilize(up);
		}
		else if (m_mode == e_fps) {
			auto rot = Quatf(radian, axis);
			target_substance = m_anchor.substance * Transformf(ezero(), rot);
			target_substance.q = target_substance.q.stabilize(up);
		}
		else if (m_mode == e_swipe) {
			// do nothing
		}
		else if (m_mode == e_birdview) {
			// screen space
			auto rot = Quatf(radian, axis);
			target_substance = m_anchor.substance * Transformf(ezero(), rot);
		}
		else {
			aux_error(true, "unknown mode %s\n", m_mode.c_str());
		}
	}
}

Plane3f Camera::getTargetPlane(const Line3f &anchor_ray) const
{
	if (m_mode == e_fps || m_mode == e_birdview) {
		auto max_t = distance(m_anchor.point, m_anchor.substance.t);
		auto t = 0.0f;
		m_referencePlane.intersect(anchor_ray, &t);

		if (t > max_t || t < 0) t = max_t;
		auto anchor_point = anchor_ray.eye + t * anchor_ray.dir;
		return Plane3f(anchor_point, anchor_ray.dir);
	}
	else {
		auto anchor_point = m_anchor.point;
		return Plane3f(anchor_point, anchor_ray.dir);
	}
	return m_referencePlane;
}
}  // namespace spu::gs_node

namespace spu {
template<> size_t serialize(uint8_t *heap, bool is_dry, const gs_node::Camera &o)
{
	auto *hp = heap;
	hp += serialize(hp, is_dry, o.m_referencePlane.eq);
	hp += serialize(hp, is_dry, o.getTargetSubstance());
	return hp - heap;
}
template<> size_t deserialize(const uint8_t *heap, gs_node::Camera &o)
{
	auto *hp = heap;
	hp += deserialize(hp, o.m_referencePlane.eq);
	hp += deserialize(hp, o.getTargetSubstance());
	return hp - heap;
}
}  // namespace spu
