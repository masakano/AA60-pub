//
// Panel :
//
#pragma once
#include "base.h"

namespace spu::gs_node::gui {

class Panel : public Base {
public:
	static constexpr hash32_t e_bg = "bg";

	explicit Panel(const char *name = nullptr);  // implies init()

	Composition &getComposition() { return m_canvas; }
	const Composition &getComposition() const { return m_canvas; }

	void setBgAlpha(float target_alpha)
	{
		for (auto &b: getGraphic(e_bg).vertices) {
			b.c.a = lerp(b.c.a, target_alpha, 0.075f);
		}
	}

	void adjustViewport(float scale);
	void update() override;

	bool isFocus(const GsCanvas *current = nullptr) const override
	{
		return Base::isFocus(current ? current : &m_canvas) || m_isPicking;
	}

protected:
	GsCanvas m_canvas;
	Rectf m_anchor;
	float m_scale = 1.0;
	uint32_t m_modifiedCount = 0;
	bool m_isPicking = false;

	void autoArrange();
	void arrangeBackgroundGraphic();

	bool alignSide(Base *node, const Base *left, const Base *upper, int32_t max_column);
	void doRender() override;
	void doDebugRender() override;  // jusst set canvas
private:
	void init(const Attrs &attrs) override { Base::init(attrs); }
};
}  // namespace spu::gs_node::gui
