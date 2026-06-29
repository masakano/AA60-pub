//
// Postproc :
//
#include "postproc.h"
#include <gsys/canvas/temporal.h>
#include <gsys/canvas/copy.h>
#include <gsys/canvas/bloom.h>
#include <gsys/canvas/glare.h>
#include <gsys/canvas/lensflare1.h>
#include <gsys/canvas/lensflare2.h>
#include <gsys/canvas/lensflare3.h>
#include <gsys/canvas/ssao.h>
#include <gsys/canvas/gauss.h>
#include <gsys/canvas/tonemap.h>
#include <gsys/canvas/dof.h>
#include <gsys/canvas/fog.h>

namespace spu::gs_canvas::gs_demo_page {

void Postproc::initSsaoCanvas()
{
	Attrs init_attrs = {
	        {"color0.target",      GL_TEXTURE_2D   },
	        {"color0.iformat",     GL_RGBA32F      },
	        {"color0.wrap_s",      GL_CLAMP_TO_EDGE},
	        {"color0.wrap_t",      GL_CLAMP_TO_EDGE},
	        {"color0.min_filter",  GL_LINEAR       },
	        {"color0.mag_filter",  GL_LINEAR       },
	        {"color0.max_level",   0               },
	        {"color0.auto_mipmap", 0               },
	};
	m_ssaoCanvas = new gs_canvas::SSAO(init_attrs);
	m_ssaoCanvas->startInspector();
}

void Postproc::initPostprocCanvas(const Attrs &attrs, gs_canvas::Shadowmap *shadowmap)
{
	constexpr const char *canvas_name_candidates
	        = "?tonemap:copy:dof:fog:edge:lpf:bloom:glare:lensflare1:lensflare2:lensflare3:cb:temporal";

	auto canvas_name = attrs.get("canvas.name", canvas_name_candidates);

	struct Property {
		const char *path;
		std::function<GsCanvas *(const Attrs &attrs)> creator;
	};

	const char *edge_path = "canvas/copy/fir_depth_laplacian.us";
	const char *cb_path = "canvas/copy/cross_bilateral.us";

	using namespace gs_canvas;
	std::map<std::string, Property> properties = {
	        {"copy",       {nullptr, [](const Attrs &a) { return new Copy(a); }}      },
	        {"tonemap",    {nullptr, [](const Attrs &a) { return new Tonemap(a); }}   },
	        {"bloom",      {nullptr, [](const Attrs &a) { return new Bloom(a); }}     },
	        {"glare",      {nullptr, [](const Attrs &a) { return new Glare(a); }}     },
	        {"lensflare1", {nullptr, [](const Attrs &a) { return new Lensflare1(a); }}},
	        {"lensflare2", {nullptr, [](const Attrs &a) { return new Lensflare2(a); }}},
	        {"lensflare3", {nullptr, [](const Attrs &a) { return new Lensflare3(a); }}},
	        {"lpf",        {nullptr, [](const Attrs &a) { return new Gauss2D(a); }}   },
	        {"dof",        {nullptr, [](const Attrs &a) { return new Dof(a); }}       },
	        {"fog",        {nullptr, [](const Attrs &a) { return new Fog(a); }}       },
	        {"temporal",   {nullptr, [](const Attrs &a) { return new Temporal(a); }}  },
	        {"edge",       {edge_path, [](const Attrs &a) { return new Copy(a); }}    },
	        {"cb",         {cb_path, [](const Attrs &a) { return new Copy(a); }}      },
	};
	const auto &prop = properties.at(canvas_name);

	Attrs init_attrs = {
	        {"shadowmap", shadowmap},
	};
	if (prop.path) {
		init_attrs.emplace_back("path", prop.path);
	}

	m_postprocCanvas = prop.creator(init_attrs);
	m_postprocCanvas->startInspector();
}

void Postproc::postproc(GsPage *page)
{
	if (m_ssaoCanvas) {
		m_ssaoCanvas->u_color0 = page->getBuffer("color0").id();
		m_ssaoCanvas->u_depth = page->getBuffer("depth").id();
		// m_ssaoCanvas->takeover(page, true);
		m_ssaoCanvas->takeover(page);
		m_ssaoCanvas->begin();
		m_ssaoCanvas->render();
		m_ssaoCanvas->end();
		m_postprocCanvas->u_color0 = m_ssaoCanvas->getBuffer("color0").id();
	}
	else {
		m_postprocCanvas->u_color0 = page->getBuffer("color0").id();
	}
	m_postprocCanvas->u_depth = page->getBuffer("depth").id();
	// m_postprocCanvas->takeover(page, true);
	m_postprocCanvas->takeover(page);
	m_postprocCanvas->begin();
	m_postprocCanvas->render();
	m_postprocCanvas->end();
}
}  // namespace spu::gs_canvas::gs_demo_page
