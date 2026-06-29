//
// Window :
//
#include <gsys/node/dui.h>
#include <spu++/spu++.h>

namespace spu::gs_node::dui {

Window::Window(float, float)
{
	auto *current = GsCanvas::getCurrent();
	ms_viewport = current->viewport(0);

	auto sx = ms_viewport.sx;
	auto sy = ms_viewport.sy;

	Range2f range;
	range.p0 = {0, 0};
	range.p1 = {sx, sy};
	getARange() = range;
}

void Window::update()
{
	ms_gesture.update();
	Container::update();
}

void Window::doRender()
{
	SpuScopedRenderstate renderstate;

	renderstate.flags.depth_test = false;
	renderstate.flags.stencil_test = false;
	renderstate.flags.cull_face = false;
	renderstate.flags.blend = true;
	renderstate.use();

	auto *current = GsCanvas::getCurrent();
	auto c_save = Composition(*current);
	auto current_viewport = current->viewport(0);

	// stick to left-upper
	auto oy = current_viewport.sy - ms_viewport.sy;
	auto sx = ms_viewport.sx;
	auto sy = ms_viewport.sy;

	current->getViewports().at(0) = {0, oy, sx, sy};
	current->getWorldviews().front() = Mat4f();
	current->getViewscreen() = Mat4f::projection(0, sx, 0, sy, -1, 1, false);
	Container::doRender();

	*(Composition*)current = c_save;
}
}  // namespace spu::gs_node::dui
