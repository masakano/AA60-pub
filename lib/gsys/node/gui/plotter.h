//
// Plotter :
//
#pragma once
#include "base.h"
#include <gsys/painter/stdout.h>
#include <deque>

namespace spu::gs_node::gui {
class Plotter : public Base {
public:
	struct Desc {
		std::vector<const char *> items;
		std::vector<float *> value_ptrs;
	};

	explicit Plotter(const char *name = nullptr) : Base(name) {}
	explicit Plotter(const Attrs &attrs) : Plotter() { init(attrs); }
	void init(const Attrs &attrs) override;
	void update() override;

protected:
	gs_painter::Stdout m_line;
	struct PlotterState : public State<float> {
		std::deque<float> queue;
	};
	std::vector<PlotterState> m_states;
	int32_t m_barLength = 0;
	float m_maxValue = epsilon();
	void doRender() override;
};
}  // namespace spu::gs_node::gui
