//
// Stdout :
//
#include <gsys/canvas.h>
#include <gsys/decorator/instance.h>
#include <gsys/painter/stdout.h>
#include <smath/color_chart.h>

namespace spu::gs_painter {

void Stdout::init(const Attrs &attrs)
{
	// init
	{
		Attrs def_attrs = {
		        {"path",         "painter/stdout/radiance.us"},
		        {"a.a_position", 4                           },
		        {"a.a_color",    4                           },
		};
		GsPainter::init(def_attrs + attrs);
		getDecorators().push_back(new gs_decorator::Instance(this, attrs));
	}

	// shader
	{
		Attrs unif_attrs = {
		        {"u_albedo",          &u_albedo         },
                        {"u_min_alpha",       &u_min_alpha      },
		        {"u_point_size",      &u_point_size     },
                        {"u_is_point_sprite", &u_is_point_sprite},
		        {"u_albedomap",       &u_albedomap      },
		};
		addUniforms(unif_attrs);
	}

	// material
	{
		auto &drawcall = getADrawcall();
		drawcall.flags.depth_test = true;
		drawcall.ub_material.min_alpha = 0.1;
		drawcall.depth_func = GL_LEQUAL;
		drawcall.poly_offset = {-1.0, -1.0};
		drawcall.flags.point_sprite = true;
		drawcall.flags.line_offset = true;
		drawcall.flags.blend = true;
		drawcall.albedomap.init("ball.png");
	}
	// text
	{
		m_text.init(Attrs());
		m_text.getADrawcall().flags.depth_test = true;
	}
	// pin
	setIsKeepInHost(true);
}

void Stdout::begin()
{
	m_color = eone<Vec4f>();
	m_strings.clear();

	m_subIndices[GL_POINTS].clear();
	m_subIndices[GL_LINES].clear();
	m_subIndices[GL_TRIANGLES].clear();

	getVerticesView().clear();
	getIndices().clear();
}

void Stdout::end()
{
	auto &indices = getIndices();
	indices.clear();

	m_subFirsts[GL_POINTS] = indices.size();
	vector_cat(indices, m_subIndices[GL_POINTS]);

	m_subFirsts[GL_LINES] = indices.size();
	vector_cat(indices, m_subIndices[GL_LINES]);

	m_subFirsts[GL_TRIANGLES] = indices.size();
	vector_cat(indices, m_subIndices[GL_TRIANGLES]);

	send();
}

void Stdout::draw(
        uint32_t mode, const std::vector<Mat4f> &nodeworlds, uint32_t first, uint32_t count,
        uint32_t instance_count, uint32_t target, uint32_t base_vertex, uint32_t base_instance)
{
	auto &coms = getDrawcalls().at(0).coms;
	coms.resize(1);
	coms[0].mode = mode;
	coms[0].first = m_subFirsts.at(mode) + first;
	coms[0].count = count ? count : m_subIndices.at(mode).size();
	coms[0].instance_count = std::max<uint32_t>(nodeworlds.size(), instance_count);
	coms[0].target = target;
	coms[0].base_vertex = base_vertex;
	coms[0].base_instance = base_instance;
	GsPainter::render(nodeworlds);
}

void Stdout::doRender()
{
	if (!m_strings.empty()) {
		auto *current = GsCanvas::getCurrent();
		auto worldscreen = current->worldscreen(0);
		auto screentext = m_text.textscreen().inverse();
		auto worldtext = screentext * worldscreen;

		m_text.begin();
		for (auto &str: m_strings) {
			auto pt = worldtext.pers3(str.p);
			m_text.getPen() = pt;
			m_text.puts(str.s.c_str());
		}
		m_text.end();
		m_text.render();
	}
	auto &drawcall = getADrawcall();
	auto &com = drawcall.coms[0];

	u_is_point_sprite = 0;
	u_min_alpha = drawcall.ub_material.min_alpha;
	u_point_size = drawcall.ub_material.point_size;
	u_albedomap = drawcall.albedomap.id();
	u_albedo = drawcall.ub_material.albedo;

	if (com.mode == GL_POINTS) u_is_point_sprite = true;
	if (com.mode == GL_TRIANGLES) u_albedo.a *= m_fillAlpha;
	com.instance_count = instanceCount();

	GsPainter::doRender();
}

void Stdout::set(const Attrs &attrs)
{
	m_text.set(attrs);
	GsPainter::set(attrs);
}

void Stdout::setColor(const Vec4f &color) { m_color = color; }

void Stdout::setColor(const char *name, float alpha) { setColor(Vec4f(color_by_name(name), alpha)); }

void Stdout::addElements(
        const std::vector<int32_t> &point_indices, const std::vector<int32_t> &line_indices,
        const std::vector<std::vector<int32_t>> &faces, const std::vector<Vec3f> &points,
        const Mat4f &nodeworld)
{
	auto vertices = getVerticesView();
	auto base = vertices.size();
	for (auto &p: points) {
		vertices.push_back(Vertex(nodeworld.ortho3(p), m_color));
	}

	for (auto &index: point_indices) {
		m_subIndices[GL_POINTS].push_back(base + index);
	}
	for (auto &index: line_indices) {
		m_subIndices[GL_LINES].push_back(base + index);
	}
	for (auto &face: faces) {
		auto &line_indices = m_subIndices[GL_LINES];
		for (auto i = 0u; i < face.size(); i++) {
			line_indices.push_back(face[i] + base);
			line_indices.push_back(face[(i + 1) % face.size()] + base);
		}
		auto &triangle_indices = m_subIndices[GL_TRIANGLES];
		for (auto i = 1u; i < face.size() - 1; i++) {
			triangle_indices.push_back(face[0] + base);
			triangle_indices.push_back(face[i] + base);
			triangle_indices.push_back(face[i + 1] + base);
		}
	}
}

void Stdout::addPrim(const Vec3f &point, const Mat4f &nodeworld)
{
	std::vector<int32_t> point_indices = {0};
	addElements(point_indices, {}, {}, {point}, nodeworld);
}

void Stdout::addPrim(const std::vector<Vec3f> &points, const Mat4f &nodeworld)
{
	std::vector<int32_t> point_indices(points.size());
	std::iota(std::begin(point_indices), std::end(point_indices), 0);

	addElements(point_indices, {}, {}, points, nodeworld);
}

void Stdout::addPrim(const Segment3f &segment, const Mat4f &nodeworld)
{
	std::vector<int32_t> line_indices = {0, 1};
	std::vector<Vec3f> points = {segment.p0, segment.p1};
	addElements({}, line_indices, {}, points, nodeworld);
}

void Stdout::addPrim(const Mat4f &frustum, const Mat4f &nodeworld) { addPrim(frustum.convex3f(), nodeworld); }

void Stdout::addPrim(const Convex2f &convex2, const Mat4f &nodeworld)
{
	std::vector<int32_t> face(convex2.points().size());
	std::iota(std::begin(face), std::end(face), 0);
	addElements({}, {}, {face}, convex2.points(), nodeworld);
}

void Stdout::addPrim(const Range2f &range2, const Mat4f &nodeworld)
{
	auto points2 = range2.points();  // Vec2f
	std::vector<Vec3f> points(std::begin(points2), std::end(points2));
	addPrim(Convex2f(points, false), nodeworld);
}

void Stdout::addPrim(const Range3f &range3, const Mat4f &nodeworld)
{
	if (range3.valid()) {
		addPrim(Mat4f(range3), nodeworld);
	}
}

void Stdout::addPrim(const Convex3f &convex3, const Mat4f &nodeworld)
{
	for (const auto &convex: convex3.convexes()) {
		addPrim(convex, nodeworld);
	}
}

void Stdout::addPrim(const Mesh &mesh, const Mat4f &nodeworld)
{
	auto &mesh_faces = mesh.getFaces();
	auto &mesh_vertices = mesh.getVertices();

	std::vector<std::vector<int32_t>> faces;
	for (auto &mesh_face: mesh_faces) {
		faces.push_back(mesh_face);
	}
	std::vector<Vec3f> points(std::begin(mesh_vertices), std::end(mesh_vertices));
	addElements({}, {}, faces, points, nodeworld);
}

void Stdout::addJoint(const Vec3f &p0, const Vec3f &p1, const Mat4f &nodeworld)
{
	const auto dz = distance(p0, p1);
	if (dz < epsilon()) return;

	const auto d = powf(dz, 0.25f) * 0.1f;
	auto local = Mat4f::direction_matrix(p0, normalize(p1 - p0));

	std::vector<std::vector<Vec3f>> points;
	for (auto a0 = 0.0f; a0 < radians(360.0f); a0 += radians(120.0f)) {
		auto a1 = a0 + radians(120.0f);
		auto p0 = Vec3f(cosf(a0), sinf(a0), 1) * d;
		auto p1 = Vec3f(cosf(a1), sinf(a1), 1) * d;
		auto p2 = Vec3f(0, 0, dz);
		auto p3 = Vec3f(0, 0, 0);
		points.push_back({p0, p1, p2});
		points.push_back({p0, p1, p3});
	}
	addPrim(Convex3f(points), nodeworld * local);
}

void Stdout::addAxis(const Mat4f &nodeworld)
{
	auto o = Vec3f(0, 0, 0);
	auto x = ex();
	auto y = ey();
	auto z = ez();

	auto color_save = m_color;

	setColor("red");
	addPrim(Segment3f(o, x), nodeworld);

	setColor("green");
	addPrim(Segment3f(o, y), nodeworld);

	setColor("blue");
	addPrim(Segment3f(o, z), nodeworld);

	m_color = color_save;
}

void Stdout::addFloor(float range, float step0, float step1, float step1_alpha, bool is_yup)
{
	//m_text.setCharScale(step0 * 0.05);

	Mat4f nodeworld;
	std::vector<const char *> colors = {"red", "green", "blue"};

	// auto nodeworld = is_yup ? Mat4f().rot("x", pi() / 2) : Mat4f();
	if (is_yup) {
		nodeworld = Mat4f().rot("x", pi() / 2);
		colors = {"red", "blue", "green"};
	}

	auto add_line = [&](float x0, float y0, float z0, float x1, float y1, float z1) {
		addPrim(Segment3f(Vec3f(x0, y0, z0), Vec3f(x1, y1, z1)), nodeworld);
	};

	auto add_text = [&](float x, float y, float z, float value) {
		puts(Vec3f(x, y, z), string_printf("%g", value), nodeworld);
	};

	auto color_save = m_color;

	setColor(colors[0]);
	add_line(-range, 0, 0, +range, 0, 0);

	setColor(colors[1]);
	add_line(0, -range, 0, 0, +range, 0);

	setColor(colors[2]);
	add_line(0, 0, 0, 0, 0, step1);

	for (auto y = step0; y <= range; y += step0) {
		m_color = color_save;
		add_line(-range, +y, 0, +range, +y, 0);
		add_line(-range, -y, 0, +range, -y, 0);

		m_color.a = step1_alpha;
		for (auto y1 = y - step0 + step1; y1 < y; y1 += step1) {
			add_line(-range, +y1, 0, +range, +y1, 0);
			add_line(-range, -y1, 0, +range, -y1, 0);
		}
		add_text(0, +y, 0, +y);
		add_text(0, -y, 0, -y);
	}
	for (auto x = step0; x <= range; x += step0) {
		m_color = color_save;
		add_line(+x, -range, 0, +x, +range, 0);
		add_line(-x, -range, 0, -x, +range, 0);

		m_color.a = step1_alpha;
		for (auto x1 = x - step0 + step1; x1 < x; x1 += step1) {
			add_line(+x1, -range, 0, +x1, +range, 0);
			add_line(-x1, -range, 0, -x1, +range, 0);
		}

		add_text(+x, 0, 0, +x);
		add_text(-x, 0, 0, -x);
	}
	m_color = color_save;
}

void Stdout::puts(const Vec3f &position, const std::string &str, const Mat4f &nodeworld)
{
	auto p = nodeworld.pers3(position);
	m_strings.push_back({p, str});
}

Stdout *Stdout::get() { return ms_painter; }

void Stdout::startup(const Attrs &)
{
	assert(ms_painter == nullptr);
	ms_painter = new Stdout(Attrs());
}

void Stdout::shutdown()
{
	delete ms_painter;
	ms_painter = nullptr;
}

GsObject::ClassCreator<Stdout> Stdout::ms_classCreator;
}  // namespace spu::gs_painter
