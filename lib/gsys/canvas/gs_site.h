//
// GsSite :
//
#pragma once

#include <gsys/canvas/gs_page.h>
#include <spu++/spu_backoffice.h>

namespace spu::gs_canvas {
class GsSite : public SpuBackofficeManager<GsCanvas, GsCanvas> {
public:
	GsSite(const Attrs &attrs) { init(attrs); }
	static void main(const Attrs &attrs);
};
}  // namespace spu::gs_canvas
