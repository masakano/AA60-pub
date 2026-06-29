//
// Tweakbar :
//
#include <gsys/canvas/copy.h>
#include <gsys/node/gui/tweakbar.h>
#include <utility>

#include "pack_tweakbar.h"

namespace spu::gs_node::gui {

template<class node_t> Base *Tweakbar::addKeyValues(const typename node_t::Desc &desc, const char *title)
{
	Attrs attrs = {
	        {"name",   title},
	        {"desc",   &desc},
	        {"parent", this },
	};
	auto node = new node_t(attrs);
	return node;
}

Tweakbar::Tweakbar(const char *name) : Panel(name)
{
	ms_tbars().insert(ms_tbars().begin(), this);
	setProperty(e_lazy, 0);  // safety
}

Tweakbar::~Tweakbar() { vector_remove(ms_tbars(), this); }

void Tweakbar::setName(const std::string &name, uint32_t width) { GsNode::setName(padstr(name, width)); }

Base *Tweakbar::addMenus(
        const char *title, const std::vector<std::vector<Menu::Item>> &items,
        const std::vector<int32_t *> &value_ptrs)
{
	return addKeyValues<Menu>({items, value_ptrs}, title);
}

Base *Tweakbar::addButtons(
        const char *title, const std::vector<const char *> &items, const std::vector<int32_t *> &value_ptrs)
{
	return addKeyValues<Button>({items, value_ptrs}, title);
}

Base *Tweakbar::addPlotters(
        const char *title, const std::vector<const char *> &items, const std::vector<float *> &value_ptrs)
{
	return addKeyValues<Plotter>({items, value_ptrs}, title);
}

Base *Tweakbar::addSliders(
        const char *title, const std::vector<const char *> &items, const std::vector<float *> &value_ptrs,
        const std::vector<Vec2f> &value_minmaxs, const std::vector<float> &value_powers)
{
	return addKeyValues<Slider>({items, value_ptrs, value_minmaxs, value_powers}, title);
}

Base *Tweakbar::addColorSlider(
        const char *title, const std::vector<const char *> &items, const std::vector<Vec4f *> &value_ptrs)
{
	return addKeyValues<ColorSlider>({items, value_ptrs}, title);
}

Base *Tweakbar::addTexviews(
        const char *title, const std::vector<uint32_t> &texture_ids, const std::vector<uint32_t> &layers,
        const std::vector<uint32_t> &levels, int column)
{
	return addKeyValues<Texview>({texture_ids, layers, levels, column}, title);
}

void Tweakbar::clearStdNode()
{
	m_stdButton.clear();
	m_stdSlider.clear();
	m_stdColorSlider.clear();
	m_stdTexview.clear();
}

void Tweakbar::addStdButton(const char *item, int32_t *value_ptr)
{
	auto &desc = m_stdButton.desc;
	desc.items.push_back(item);
	desc.value_ptrs.push_back(value_ptr);
}

void Tweakbar::addStdSlider(
        const char *text, float value_min, float value_max, float *value_ptr, float value_power)
{
	auto &desc = m_stdSlider.desc;
	desc.items.push_back(text);
	desc.value_ptrs.push_back(value_ptr);
	desc.value_minmaxs.emplace_back(value_min, value_max);
	desc.value_powers.emplace_back(value_power);
}
void Tweakbar::addStdColorSlider(const char *text, Vec4f *value_ptr)
{
	auto &desc = m_stdColorSlider.desc;
	desc.items.push_back(text);
	desc.value_ptrs.push_back(value_ptr);
}

void Tweakbar::addStdTexview(uint32_t texture_id, int32_t column, int32_t layer)
{
	auto target = 0;
	auto max_level = 0;
	auto depth = 0;

	spu_texture_get(texture_id, "max_level", &max_level);
	spu_texture_get(texture_id, "target", &target);
	spu_texture_get(texture_id, "depth", &depth);

	auto &desc = m_stdTexview.desc;
	desc.column = column;

	if (target == GL_TEXTURE_CUBE_MAP_ARRAY) {
		depth /= 6;  // OpennGL confusion
	}
	if (depth == 1 || layer != -1) {
		for (auto level = 0; level <= max_level; level++) {
			desc.ids.push_back(texture_id);
			desc.layers.push_back(layer);
			desc.levels.push_back(level);
		}
	}
	else {
		for (auto layer = 0; layer <= depth; layer++) {
			desc.ids.push_back(texture_id);
			desc.layers.push_back(layer);
			desc.levels.push_back(0);
		}
	}
	// addKeyValues<Texview>(m_stdTexview.desc, "texview");
}

void Tweakbar::remove(const std::string &name)
{
	auto child_nodes = moveChildren();
	for (auto ip = child_nodes.begin(); ip != child_nodes.end(); ++ip) {
		if ((*ip)->name().find(name) == 0) {
			delete *ip;
			child_nodes.erase(ip);
			addChildren(child_nodes);
			return;
		}
	}
	addChildren(child_nodes);
}

void Tweakbar::bake()
{
	auto remove_child = [&](GsNode **node) {
		if (*node) {
			auto child_nodes = moveChildren();
			vector_remove(child_nodes, *node);
			delete *node;
			addChildren(child_nodes);
			*node = nullptr;
		}
	};
	remove_child(&m_stdButton.node);
	remove_child(&m_stdSlider.node);
	remove_child(&m_stdColorSlider.node);
	remove_child(&m_stdTexview.node);

	if (!m_stdButton.desc.items.empty()) {
		addKeyValues<Button>(m_stdButton.desc, "buttons");
		m_stdButton.node = getChildren().back();
	}
	if (!m_stdSlider.desc.items.empty()) {
		addKeyValues<Slider>(m_stdSlider.desc, "sliders");
		m_stdSlider.node = getChildren().back();
	}
	if (!m_stdColorSlider.desc.items.empty()) {
		addKeyValues<ColorSlider>(m_stdColorSlider.desc, "colors");
		m_stdColorSlider.node = getChildren().back();
	}
	if (!m_stdTexview.desc.ids.empty()) {
		addKeyValues<Texview>(m_stdTexview.desc, "texview");
		m_stdTexview.node = getChildren().back();
	}

	Panel::bakeTextInternal<State<int32_t>, int32_t>({});  // no text
	getWindowModifier().fold(true);

	// for (auto &child: selectChildren<Base *>()) {
	for (auto &child: select_objects<Base *>(getChildren())) {
		child->getWindowModifier().unfold(true);
	}

	if (getProperty(e_render)) {
		organize();
	}
	update();
}

const Tweakbar *Tweakbar::isHeaderOverwrap() const
{
	auto r0 = getNodefrag(&m_canvas) * headerRange();
	for (auto &tbar: ms_tbars()) {
		if (tbar != this) {
			auto r1 = tbar->getNodefrag(&tbar->m_canvas) * tbar->headerRange();
			if (r0.intersect(r1)) {
				return tbar;
			}
		}
	}
	return nullptr;
}

void Tweakbar::organize()
{
	if (equal(ms_viewport, Rectf())) return;

	PackNode<Tweakbar> pnode;
	pnode.m_rect = ms_viewport;
	auto modifier = [&](const PackNode<Tweakbar>::Rect rect) {
		auto &viewport = rect.ptr->getComposition().getViewports().at(0);
		viewport.ox = ms_viewport.sx - (rect.ox + rect.sx);
		viewport.oy = ms_viewport.sy - (rect.oy + rect.sy);
	};

	for (auto &tbar: ms_tbars()) {
		tbar->adjustViewport(1.0);
		if (tbar->getProperty(e_render)) {
			PackNode<Tweakbar>::Rect rect;
			const auto &viewport = tbar->m_canvas.viewport(0);

			rect.sx = viewport.sx + 4;
			rect.sy = viewport.sy + 4;
			rect.ptr = tbar;
			pnode.insert(rect, modifier);
		}
	}
	pnode.dispose();
}

void Tweakbar::sortAndDrawAll()
{
	if (ms_tbars().empty()) {
		return;
	}

	auto *current = GsCanvas::getCurrent();
	if (!equal(current->viewport(0), ms_viewport)) {
		ms_viewport = current->viewport(0);
		for (auto &tbar: ms_tbars()) {
			tbar->organize();
		}
	}

	// sort
	{
		auto *front = ms_tbars().back();
		if (!front->isFocus()) {
			for (auto tp = begin(ms_tbars()); tp != end(ms_tbars()); ++tp) {
				if ((*tp)->isFocus() && (*tp)->getProperty(e_render)) {
					auto *front = *tp;  // copy
					ms_tbars().erase(tp);
					ms_tbars().insert(end(ms_tbars()), front);
					break;
				}
			}
		}

// #define MAINTENANCE
#ifdef MAINTENANCE
		spu_printf(0, "sortAndDrawAll:\n");
		for (auto &tbar: ms_tbars()) {
			auto &props = tbar->getProperties();
			spu_printf(
			        0, "\t%d:%d:%d %s\n", tbar->isFocus(), props.at(e_fold), props.at(e_render),
			        tbar->name().c_str());
		}
#endif
	}

	// arrange and grab
	{
		auto *front = ms_tbars().back();
		if (front->isFocus()) {
			auto *gesture = getGesture();
			if (gesture->prev().mouse_L && !gesture->curr().mouse_L) {
				auto *tbar = front->isHeaderOverwrap();
				if (tbar) {
					auto &c = front->getComposition();
					c.getViewports().at(0).oy -= c_charheight * c_charscale + 2;
				}
			}
			grab();
		}
		else {
			ungrab();
		}
	}

	// draw
	{
		SpuScopedRenderstate renderstate(true);

		// printf("alpha: \n");
		for (auto &tbar: ms_tbars()) {
			if (tbar->getProperty(e_render)) {
				auto is_focus = tbar == ms_tbars().back() && tbar->isFocus();
				auto is_top = &tbar == &ms_tbars().back();
				auto target_alpha = is_focus ? 1.00f : is_top ? 0.95f : 0.50f;

				tbar->setBgAlpha(target_alpha);
				tbar->update();
				tbar->render();
			}
		}
	}
}

void Tweakbar::saveAll()
{
	File file("test.txt", "w");
	for (auto &tbar: ms_tbars()) {
		tbar->save(file);
	}
}

void Tweakbar::loadAll()
{
	File file;
	if (file.open("test.txt", "r", false)) {
		for (auto &tbar: ms_tbars()) {
			tbar->load(file);
		}
	}
	else {
		aux_message(0, "%s: not found (ignores)\n", "test.txt");
	}
}
void Tweakbar::startup(const Attrs &) { /*printf("Tweakbar::startup...\n");*/ }
void Tweakbar::shutdown()
{
	ms_tbars().clear();  // don't delete instance
}

// class startup
GsObject::ClassCreator<Tweakbar> Tweakbar::ms_classCreator;
}  // namespace spu::gs_node::gui
