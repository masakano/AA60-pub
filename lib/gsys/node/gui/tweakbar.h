//
// Tweakbar :
//
#pragma once

#include "button.h"
#include "menu.h"
#include "slider.h"
#include "color_slider.h"
#include "plotter.h"
#include "birdview.h"
#include "texview.h"
#include "panel.h"

namespace spu::gs_node::gui {

class Tweakbar : public Panel {
public:
	static constexpr int32_t e_class_depth = Panel::e_class_depth + 1;

	explicit Tweakbar(const char *name = nullptr);  // implies init()

	~Tweakbar();

	Base *addMenus(
	        const char *title, const std::vector<std::vector<Menu::Item>> &items,
	        const std::vector<int32_t *> &value_ptrs);

	Base *addButtons(
	        const char *title, const std::vector<const char *> &items,
	        const std::vector<int32_t *> &value_ptrs);

	Base *addPlotters(
	        const char *title, const std::vector<const char *> &items,
	        const std::vector<float *> &value_ptrs);

	Base *addSliders(
	        const char *title, const std::vector<const char *> &items,
	        const std::vector<float *> &value_ptrs, const std::vector<Vec2f> &value_minmaxs,
	        const std::vector<float> &value_powers);

	Base *addColorSlider(
	        const char *title, const std::vector<const char *> &items,
	        const std::vector<Vec4f *> &value_ptrs);

	Base *addTexviews(
	        const char *title, const std::vector<uint32_t> &texture_ids,
	        const std::vector<uint32_t> &layers = {0}, const std::vector<uint32_t> &levels = {0},
	        int column = 0);

	template<class birdview_t = Birdview> Base *addBirdview(const char *title, const Attrs &attrs = Attrs())
	{
		Attrs def_attrs = {
		        {"name",   title},
		        {"parent", this },
		};
		auto *birdview = new birdview_t(def_attrs + attrs);
		return birdview;
	}
	void remove(const std::string &name);

	void clearStdNode();
	void addStdButton(const char *item, int32_t *value_ptr);
	void addStdSlider(
	        const char *text, float value_min, float value_max, float *value_ptr, float value_power = 1.0);
	void addStdColorSlider(const char *text, Vec4f *value_ptr);
	void addStdTexview(uint32_t texture_id, int32_t column = 1, int32_t layer = -1);

	void bake();
	void setName(const std::string &name) override { setName(name, 24); }
	void setName(const std::string &name, uint32_t width);

	static void organize();
	static void sortAndDrawAll();
	static void saveAll();
	static void loadAll();

protected:
	template<class node_t> Base *addKeyValues(const typename node_t::Desc &desc, const char *title);

	template<class node_t> struct StdNode {
		std::string title;
		typename node_t::Desc desc;
		GsNode *node = nullptr;
		void clear() { desc = typename node_t::Desc(); }
	};
	StdNode<Button> m_stdButton;
	StdNode<Slider> m_stdSlider;
	StdNode<ColorSlider> m_stdColorSlider;
	StdNode<Texview> m_stdTexview;

	const Tweakbar *isHeaderOverwrap() const;
	inline static Rectf ms_viewport = {0, 0, 0, 0};
	static std::vector<Tweakbar *> &ms_tbars()
	{
		static std::vector<Tweakbar *> v;
		return v;
	}

private:
	friend struct ClassCreator<Tweakbar>;
	static void startup(const Attrs &attrs);
	static void shutdown();
	static ClassCreator<Tweakbar> ms_classCreator;
};
}  // namespace spu::gs_node::gui
