//
// Gauss2D :
//
#include "gauss_2d_inspector.h"

namespace spu::gs_canvas {

void Gauss2D::init(const Attrs &attrs)
{
	attrs.peek("path.pregauss", "use 'path.first' instead");

	// 2nd gauss
	Gauss1D::init(attrs);
	m_firstPath = attrs.get("path.first", "");
	if (m_firstPath.empty()) {
		m_firstPath = attrs.get("path", "");
	}
	u_footstep = {1, 0, 0, 1};

	m_firstGauss.dispose();
	m_firstGauss.getShader().dispose();
}

void Gauss2D::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new Gauss2DInspector(this);
	}
}

Gauss1D &Gauss2D::lazyInitFirstGauss()
{
	if (m_firstGauss.id() == 0) {
		auto color0 = getBuffer("color0").id();
		auto iformat = 0u;
		spu_texture_get(color0, "iformat", &iformat);

		Attrs attrs = {
		        {"viewport0",          viewport(0)     },
		        {"color0.target",      GL_TEXTURE_2D   },
		        {"color0.iformat",     iformat         },
		        {"color0.auto_mipmap", 0               },
		        {"color0.max_level",   0               },
		        {"color0.wrap_s",      GL_CLAMP_TO_EDGE},
		        {"color0.wrap_t",      GL_CLAMP_TO_EDGE},
		};
		if (!m_firstPath.empty()) {
			attrs.emplace_back("path", m_firstPath.c_str());
		}
		m_firstGauss.init(attrs);
	}
	return m_firstGauss;
}

void Gauss2D::set(const Attrs &attrs)
{
	lazyInitFirstGauss();
	m_firstGauss.set(attrs);
	Gauss1D::set(attrs);
}
void Gauss2D::render()
{
	lazyInitFirstGauss();

	// 1st gauss
	{
		m_firstGauss.u_footstep = {u_footstep.z, u_footstep.w, 0, 0};  // use yz
		m_firstGauss.u_variance = u_variance;                          // use main variance
		m_firstGauss.u_color0 = u_color0;                              // use main color
		m_firstGauss.ub_connect = ub_connect;                          // use main connect
		m_firstGauss.begin();
		m_firstGauss.render();
		m_firstGauss.end();
	}

	// 2nd gauss
	{
		auto u_color0_save = u_color0;
		auto ub_connect_save = ub_connect;

		u_color0 = m_firstGauss.getBuffer("color0").id();
		ub_connect.sources[0].layer = 0;
		ub_connect.sources[0].gain = 1.0;
		ub_connect.sources[0].bias = 0.0;
		Gauss1D::render();

		u_color0 = u_color0_save;
		ub_connect = ub_connect_save;
	}
}
}  // namespace spu::gs_canvas
