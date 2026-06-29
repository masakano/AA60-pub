//
// Atlas :
//
#include <gsys/canvas/atlas.h>
#include <gsys/node.h>

namespace spu::gs_canvas {

void Atlas::init(const Attrs &attrs)
{
	auto nx = attrs.get("nx", 1);
	auto ny = attrs.get("ny", 1);

	GsCanvas::init(attrs);

	auto &viewport = getViewports().at(0);
	auto sx = viewport.sx / nx;
	auto sy = viewport.sy / ny;

	for (auto y = 0; y < ny; y++) {
		for (auto x = 0; x < nx; x++) {
			auto ox = x * sx;
			auto oy = y * sy;
			auto local_composition = Composition(*this);
			local_composition.getViewports().at(0) = Rectf(ox, oy, sx, sy);
			m_localCompositions.emplace_back(local_composition);
		}
	}
}

void Atlas::subBegin(int32_t atlas_index, [[maybe_unused]] bool is_clear)
{
	m_mainComposition = Composition(*this);
	*(Composition*)this = m_localCompositions.at(atlas_index);
	sync(0);
}

void Atlas::subEnd() { *(Composition *)(this) = m_mainComposition; }

Vec4f Atlas::getScreentexc(uint32_t atlas_index) const
{
	auto &local_viewport = m_localCompositions.at(atlas_index).viewport(0);
	auto &master_viewport = viewport(0);
	auto shift = Mat4f::fragtexc(master_viewport) * Mat4f::texcfrag(local_viewport);
	auto screentexc = shift * Mat4f::screentexc();
	return Vec4f(screentexc.c[0].x, screentexc.c[1].y, screentexc.c[3].x, screentexc.c[3].y);
}

}  // namespace spu::gs_canvas
