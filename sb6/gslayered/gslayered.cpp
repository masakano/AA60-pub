//
// App :
//
#include "base_app.h"
namespace spu::gslayered {
class App : public BaseApp {
public:
	void render() override;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	struct {
		Mat4f u_viewscreen;
		Mat4f u_modelview[16];
	} u_transform;
	SpuArray m_array;
	SpuFrame m_frame;
	SpuTexture m_depthTexture;
	SpuShader m_gslayersShader;
	SpuShader m_showlayersShader;
	sb6::Object m_object;
	uint32_t u_color_texture;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shader
	{
		Attrs unif_attrs = {
		        {"u_color_texture", &u_color_texture},
		};
		loadShader(m_showlayersShader, "gslayered/showlayers.us", Attrs(), unif_attrs);
	}
	{
		Attrs unif_attrs = {
		        {"BLOCK", &u_transform},
		};
		loadShader(m_gslayersShader, "gslayered/gslayers.us", Attrs(), unif_attrs);
	}
	// array
	{
		m_array.init({
		        {"nelem", 4}
                });
	}
	// fbo
	{
		Attrs attrs = {
		        {"target",  GL_TEXTURE_2D_ARRAY},
		        {"iformat", GL_RGBA8           },
		        {"width",   256                },
		        {"height",  256                },
		        {"depth",   16                 },
		};
		u_color_texture = spu_texture_new(attrs);
	}
	{
		Attrs attrs = {
		        {"target",      GL_TEXTURE_2D_ARRAY  },
		        {"iformat",     GL_DEPTH_COMPONENT32F},
		        {"width",       256                  },
		        {"height",      256                  },
		        {"depth",       16                   },
		        {"min_filter",  GL_LINEAR            },
		        {"auto_mipmap", 0                    },
		};
		m_depthTexture.init(attrs);
	}
	{
		Rectf viewport = {0, 0, 256, 256};
		Attrs attrs = {
		        {"viewport0", viewport           },
		        {"color0",    u_color_texture    },
		        {"depth",     m_depthTexture.id()},
		        {"bgcolor0",  c_black            },
		        {"bgdepth",   1.0                },
		};
		m_frame.init(attrs);
	}
	// bg
	{
		Attrs attrs = {
		        {"bgcolor0", c_gray},
		        {"bgdepth",  1.0   },
		};
		BaseApp::set(attrs);
	}
	// model
	{
		const char *sym[] = {
		        "a.a_position",
		        "a.a_normal",
		        nullptr,
		};
		m_object.load("torus.sbm", Attrs(), sym, m_gslayersShader.id());
	}
}

void App::render()
{
	// transform
	{
		auto t = getSeconds().current();

		sb6::Composition composition;
		composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
		u_transform.u_viewscreen = composition.viewscreen();
		for (auto i = 0; i < 16; i++) {
			auto fi = float(i + 12) / 16.0f;

			u_transform.u_modelview[i]
			        = c_unit.rot("XZ", -t * 30.0 * fi, -t * 25.0 * fi).trans({0.0, 0.0, -4.0});
		}
	}
	// fbo
	{
		m_frame.begin();
		m_frame.clear();
		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.depth_func = GL_LEQUAL;
		renderstate.use();
		m_gslayersShader.use();
		m_object.draw();
		m_frame.end();
	}
	// blit
	{
		m_showlayersShader.use();
		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = false;
		renderstate.use();
		m_array.draw(GL_TRIANGLE_FAN, 0, 4, 16);
	}
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gslayered");
}  // namespace spu::gslayered
