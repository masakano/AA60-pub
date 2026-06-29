//
// Sb6Multisample :
//
#pragma once
#include <spu++/spu++.h>

namespace spu {
class Sb6Multisample {
public:
	void init(const Rectf &viewport)
	{
		// shader
		{
			m_shader.init("multisample_copy.us");
			Attrs uniform_attrs = {
			        {"u_color0", &u_color0},
			        {"u_zoom",   &u_zoom  },
			};
			m_shader.addUniforms(uniform_attrs);
		}

		// frame
		{
			// Rectf viewport(0, 0, 1280, 720);
			auto c_multisample = 16;
			Attrs capture_attrs = {
			        {"viewport0",                  viewport                 },
			        {"color0.multisample",         c_multisample            },
			        {"color0.target",              GL_TEXTURE_2D_MULTISAMPLE},
			        {"color0.iformat",             GL_RGBA8                 },
			        {"color0.max_level",           0                        },
			        {"color0.auto_mipmap",         0                        },
			        {"color0.resolve.target",      GL_TEXTURE_2D            },
			        {"color0.resolve.iformat",     GL_RGBA8                 },
			        {"color0.resolve.min_filter",  GL_NEAREST               },
			        {"color0.resolve.mag_filter",  GL_NEAREST               },
			        {"color0.resolve.max_level",   0                        },
			        {"color0.resolve.auto_mipmap", 0                        },
			        {"depth.multisample",          c_multisample            },
			        {"depth.target",               GL_RENDERBUFFER          },
			        {"depth.iformat",              GL_DEPTH_COMPONENT32F    },
			};
			m_frame.init(capture_attrs);
			u_color0 = m_frame.getBuffer("color0.resolve").id();
		}
	}
	void begin()
	{
		// auto &renderstate = m_capture.getRenderstate();  // not capture renderstate
		m_renderstate.flags.multisample = m_isMultisample;
		m_renderstate.flags.sample_shading = m_isSampleShading;
		m_renderstate.flags.fill = m_isFill;
		m_renderstate.flags.blend = true;
		m_renderstate.flags.depth_test = true;
		m_renderstate.flags.cull_face = true;
		m_renderstate.min_sample_shading = m_minSampleShading;

		m_frame.begin();
		m_frame.clear();
		m_renderstate.use();
	}
	void end() { m_frame.end(); }

	void render()
	{
		m_drawRenderstate.use();
		m_shader.use();
		spu_array_draw(0, GL_TRIANGLE_STRIP);
	}

	void menu()
	{
		ImGui::Checkbox("multisample", &m_isMultisample);
		ImGui::Checkbox("sample shading", &m_isSampleShading);
		ImGui::Checkbox("fill", &m_isFill);

		ImGui::SliderFloat("zoom", &u_zoom, 1.0, 16.0);
		if (m_isMultisample && m_isSampleShading) {
			ImGui::SliderFloat("min sample", &m_minSampleShading, 0.0, 1.0);
		}
	}

private:
	SpuRenderstate m_renderstate;
	SpuRenderstate m_drawRenderstate;
	SpuFrame m_frame;
	SpuShader m_shader;
	bool m_isSampleShading = 1;
	bool m_isMultisample = 1;
	bool m_isFill = 1;
	float m_minSampleShading = 1.0;
	uint32_t u_color0 = 0;
	float u_zoom = 1.0;
};
}  // namespace spu
