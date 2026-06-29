//
// Texview :
//
#pragma once
#include "base.h"
#include <gsys/canvas/copy.h>

namespace spu::gs_node::gui {
class Texview : public Base {
public:
	struct Desc {
		std::vector<uint32_t> ids;
		std::vector<uint32_t> layers;
		std::vector<uint32_t> levels;
		int32_t column = 0;  // 0: auto
	};

	explicit Texview(const char *name = nullptr) : Base(name) {}
	explicit Texview(const Attrs &attrs) : Texview() { init(attrs); }
	void init(const Attrs &attrs) override;
	void update() override;

protected:
	gs_canvas::Copy m_canvas;
	Desc m_desc;
	float m_aspect = 1.0;
	void doRender() override;
};
}  // namespace spu::gs_node::gui
