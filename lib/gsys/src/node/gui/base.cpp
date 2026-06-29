//
// Base :
//
#include <gsys/node/gui/base.h>
#include <smath/geometry.h>

#include <gsys/painter/stdout.h>

namespace spu::gs_node::gui {

void Base::WindowModifier::fold(bool is_immediate)
{
	m_count++;
	m_speed = -4.0f;  // ad-hoc
	if (is_immediate) m_rate = 0.0;
}

void Base::WindowModifier::unfold(bool is_immediate)
{
	m_count++;
	m_speed = 4.0f;  // ad-hoc
	if (is_immediate) m_rate = 1.0;
}

void Base::WindowModifier::reverse(bool is_immediate)
{
	if (m_speed < 0.0 || m_rate == 1.0) {
		fold(is_immediate);
	}
	if (m_speed > 0.0 || m_rate == 0.0) {
		unfold(is_immediate);
	}
}

void Base::WindowModifier::update()
{
	if (m_speed != 0.0) {
		m_count++;
		m_rate += m_speed * getSeconds().delta();
		if (m_rate < 0.0) m_rate = 0.0, m_speed = 0.0;
		if (m_rate > 1.0) m_rate = 1.0, m_speed = 0.0;
	}
}

Base::Base(const char *name) : GsNode(name), m_windowModifier(this)
{
	setProperty(e_front, 0);
	m_windowModifier.fold(true);
}

void Base::init(const Attrs &attrs)
{
	assert(getChildren().empty());

	// parent
	GsNode::init(attrs);

	// pad
	if (ms_gesture.native() == nullptr) {
		ms_gesture.init(nullptr);
	}

	// parent
	{
		auto parent = dynamic_cast<Base *>(attrs.get<GsObject *>("parent", nullptr));
		if (parent) {
			auto parent_columns = parent->name().length();
			setName(padstr(name(), parent_columns));
			parent->addChildren({this});
		}
	}

	// colors
	{
		m_colors = {
		        {e_title,      {1.00, 1.00, 0.33, 0.75}},
                        {e_disabled,   {0.20, 0.20, 0.20, 0.20}},
		        {e_inactive,   {0.50, 0.75, 0.75, 0.50}},
                        {e_active,     {1.00, 1.00, 1.00, 1.00}},
		        {e_featured,   {0.50, 1.00, 0.50, 1.00}},
                        {e_background, {0.01, 0.01, 0.02, 1.00}},
		};
	}

	// graphics
	{
		newGraphic(e_guide);
		newGraphic(e_bar);
		newGraphic(e_button);
	}
	// guide
	{
		auto &graphic = getGraphic(e_guide);
		graphic.drawcall.flags.depth_test = false;
		graphic.drawcall.flags.blend = true;
		graphic.drawcall.ub_material.invisible = false;
		graphic.vertices.resize(1);  // fixed length
	}

	// bar
	{
		auto &graphic = getGraphic(e_bar);
		graphic.drawcall.flags.depth_test = false;
		graphic.drawcall.flags.blend = true;
		graphic.drawcall.ub_material.invisible = false;
	}

	// button
	{
		auto &graphic = getGraphic(e_button);
		graphic.drawcall.albedomap.init("ball.png");
		graphic.drawcall.flags.depth_test = false;
		graphic.drawcall.flags.blend = true;
		graphic.drawcall.ub_material.invisible = false;
	}

	// text
	{
		clearText();
		updateText();
	}

	// start with unfold
	{
		m_windowModifier.unfold(true);
	}
}

void Base::update()
{
	auto &range = getARange();
	auto cursor = getCursor(ms_gesture.curr().cursor);

	if (std::isnan(cursor.x) || std::isnan(cursor.y)) {
		aux_message(0, "invalid cursor (ignored)\n", GsCanvas::getCurrent());  // need fix
		return;
	}
	if (m_count && ms_gesture.curr().swap_count == m_count) {
		aux_message(0, "redundant update() call. (count=%d)\n", m_count);
	}
	m_count = ms_gesture.curr().swap_count;

	// fold / unfold
	{
		auto *gesture = getGesture();
		auto is_inside = headerRange().inside(cursor);
		if (is_inside && (!gesture->prev().mouse_R && gesture->curr().mouse_R)) {
			m_windowModifier.reverse(false);
		}
	}

	// guide
	{
		auto &graphic = getGraphic(e_guide);
		auto &drawcall = graphic.drawcall;
		auto &vertex = graphic.vertices.at(0);

		auto line = std::floor(cursor.y / c_charheight + 0.5f);  // note line < 0
		auto span = range.span();

		auto tline = -int32_t(line);
		drawcall.ub_material.invisible
		        = !(tline >= 0 && tline < int32_t(m_columns.size()) && m_columns.at(tline) > 0);

		auto center = Vec2f(range.center().x, line * c_charheight);
		vertex.c = Vec4f(0.25, 0.25, 0.50, 0.50);
		vertex.t = Vec4f(0, 0, 1, 1);
		vertex.p = Vec3f(center.x, center.y, 0);
		vertex.s = Vec4f(-span.x / 2, -1.0, +span.x / 2, +1.0);
	}

	if (m_windowModifier.speed() || m_windowModifier.rate() == 1.0) {  // not folded
		for (auto &node: getChildren()) {
			node->update();
		}
	}
}

void Base::doRender()
{
	auto &nodeworlds = instancedNodeworlds().at(0);  // slot #0 only
	// auto &props = properties();
	auto is_transit = m_windowModifier.speed() != 0.0;
	auto is_fold = m_windowModifier.rate() == 0.0;
	auto current = GsCanvas::getCurrent();
	auto c_save = Composition(*current);

	m_windowModifier.update();

	// sicssor
	if (is_fold || is_transit) {
		current->getScissors().at(0) = Rectf(scissorRange());
		current->sync(0);  // necessary
	}

	// disable guide
	{
		auto &graphic = getGraphic(e_guide);
		auto cursor = getCursor(getGesture()->curr().cursor);
		graphic.drawcall.ub_material.invisible |= !(isFocus() && getARange().inside(cursor));
	}

	// graphic
	{
		auto &drawcalls = ms_sprite->getDrawcalls();
		auto vertices = ms_sprite->getVerticesView();

		drawcalls.clear();
		vertices.clear();

		for (auto &graphic: m_graphics) {
			graphic.drawcall.coms[0].first = vertices.size();
			graphic.drawcall.coms[0].count = graphic.vertices.size();
			drawcalls.push_back(graphic.drawcall);
			for (auto &vertex: graphic.vertices) {
				vertices.push_back(vertex);
			}
		}
		ms_sprite->update();
		ms_sprite->setProperty(e_debug_render, getProperty(e_debug_render));  // debugdraw override
		ms_sprite->render(nodeworlds);
	}

	// text
	{
		auto &graphic = m_textGraphic;

		ms_text->setTextscreen(current->worldscreen(0));
		ms_text->getVerticesView() = graphic.vertices;
		ms_text->update();
		ms_text->render(nodeworlds);
	}

	if (!is_fold || is_transit) {
		for (const auto &node: getChildren()) {
			node->render(nodeworlds);  // use 1st slot
		}
	}
	if (is_fold || is_transit) {
		*(Composition*)current = c_save;
		current->sync(0);  // necessary
	}
#if 0
	// debug
	{
		auto *painter = gs_painter::Stdout::get();
		painter->begin();
		painter->addPrim(getARange(), nodeworlds.at(0));
		painter->end();
		painter->renderMode(GL_LINES);
	}
#endif
}

int32_t Base::getColumns(const char *title, Base *parent) const
{
	auto columns = title ? strlen(title) : name().length();
	if (parent) columns = std::max(columns, parent->name().length());
	return columns;
}

void Base::save(File &file) const
{
	// for (auto &child: selectChildren<Base *>()) {
	for (auto &child: select_objects<Base *>(getChildren())) {
		child->save(file);
	}
}
void Base::load(File &file)
{
	// for (auto &child: selectChildren<Base *>()) {
	for (auto &child: select_objects<Base *>(getChildren())) {
		child->load(file);
	}
}

bool Base::isFocus(const GsCanvas *current) const  // support window folding
{
	if (current == nullptr) current = GsCanvas::getCurrent();
	auto cursor = Vec2f(ms_gesture.native()->cursor);  // unmasked
	auto viewport_range = Range2f(current->viewport(0));
	return viewport_range.inside(cursor);
}

bool Base::isInside(const Vec2f &cursor) const
{
	return getSeconds().count() % 60 == 0 || getARange().inside(cursor);
}

SpuGesture *Base::getGesture()
{
	return &ms_gesture;  // static must be here in DLL
}

void Base::changeState(const std::vector<const void *> &ptrs, const hash32_t &state)
{
	// for (auto &child: selectChildren<Base *>()) {
	for (auto &child: select_objects<Base *>(getChildren())) {
		child->changeState(ptrs, state);
	}
}

void Base::setTextColor(const Vec4f &color) { ms_text->setColor(color); }

void Base::putText(const std::string &text)
{
	ms_text->puts(text.c_str());
	for (auto &c: text) {
		if (c == '\n') {
			m_columns.push_back(0);
		}
		else if (isalnum(c)) {
			m_columns.back()++;
		}
	}
}

void Base::clearText()
{
	getARange().invalidate();
	m_columns.clear();
	m_columns.push_back(0);

	ms_text->begin();
	ms_text->addTilt({0.0, -0.5});
	setTextColor(m_colors[e_title]);
	putText(name() + "\n");
	setTextColor(m_colors[e_active]);
}

void Base::updateText()
{
	m_textGraphic.vertices = ms_text->getVerticesView();
	getARange().expand(ms_text->getRange());
}

std::string Base::padstr(const std::string &item, int32_t length)
{
	auto pad = length - int32_t(item.length());
	return pad > 0 ? item + std::string(pad, ' ') : item;
}

std::vector<std::string> Base::normalizestr(
        const std::vector<const char *> &items, const int32_t indent, const int32_t length)
{
	std::vector<std::string> results;
	const auto prefix = std::string(indent, ' ');
	for (auto &item: items) {
		auto padded_item = padstr(item, length - indent);
		results.push_back(prefix + padded_item);
	}
	return results;
}

bool Base::grab()
{
	if (SpuGesture::getGrab() == nullptr) {
		SpuGesture::setGrab(&ms_gesture);
	}
	ms_gesture.update();
	return SpuGesture::getGrab() == &ms_gesture;
}

bool Base::ungrab()
{
	if (SpuGesture::getGrab() == &ms_gesture) {
		SpuGesture::setGrab(nullptr);
	}
	ms_gesture.update();
	return SpuGesture::getGrab() == nullptr;
}

Mat4f Base::getNodefrag(const GsCanvas *current) const
{
	if (current == nullptr) current = GsCanvas::getCurrent();
	return current->screenfrag(0) * current->worldscreen(0) * getASubstance();
}

Vec2f Base::getCursor(const int16_t icursor[2]) const
{
	return getNodefrag().inverse().ortho3(Vec3f(icursor[0], icursor[1], 0));
}

Base::Graphic &Base::getGraphic(const hash32_t &name)
{
	for (auto &graphic: m_graphics) {
		if (graphic.name == name) {
			return graphic;
		}
	}
	aux_error(true, "graphic '%s' not found\n", name.c_str());
}

Base::Graphic &Base::newGraphic(const hash32_t &name)
{
	Graphic graphic;
	graphic.name = name;
	m_graphics.push_back(graphic);
	return m_graphics.back();
}

Range2f Base::windowRange() const
{
	auto range = Range2f(getARange());
	auto y0 = range.p1.y - c_charheight;  // header only
	auto y1 = range.p0.y;

	range.p0.y = coserp(y0, y1, m_windowModifier.rate());
	return range;
}

Range2f Base::scissorRange(const GsCanvas *current) const
{
	if (current == nullptr) current = GsCanvas::getCurrent();
	auto rangeA = Range2f(getNodefrag(current) * windowRange());
	auto rangeB = Range2f(current->viewport(0));

	auto p0 = max(rangeA.p0, rangeB.p0);
	auto p1 = p0 + min(rangeA.span(), rangeB.span());
	return {p0, p1};
}

Range2f Base::headerRange() const
{
	auto range = Range2f(getARange());
	range.p0.y = range.p1.y - c_charheight;
	return range;
}

void Base::startup(const Attrs &)
{
	assert(ms_sprite == nullptr);
	assert(ms_text == nullptr);

	ms_sprite = new gs_painter::Sprite(Attrs());
	ms_text = new gs_painter::Text(Attrs());
}

void Base::shutdown()
{
	delete ms_text;
	delete ms_sprite;
	ms_text = nullptr;
	ms_sprite = nullptr;
}
GsObject::ClassCreator<Base> Base::ms_classCreator;

}  // namespace spu::gs_node::gui
