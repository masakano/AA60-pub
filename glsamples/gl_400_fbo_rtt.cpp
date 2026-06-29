//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 400 core                                                                      \n"
    "const int vertexCount = 3;                                                             \n"
    "const vec2 positions[vertexCount] = vec2[](                                            \n"
    "    vec2(-1.0,-1.0),                                                                   \n"
    "    vec2( 3.0,-1.0),                                                                   \n"
    "    vec2(-1.0, 3.0));                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);                              \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 400 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec2 texture_size = vec2(textureSize(u_diffuse, 0));                               \n"
    "    color = texture(u_diffuse, gl_FragCoord.xy / texture_size);                        \n"
    "    //color = texelFetch(u_diffuse, ivec2(gl_FragCoord.xy), 0);                        \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[3];
	SpuShader m_shader;
	uint32_t u_diffuse;
	std::vector<Rectf> m_viewports;
	uint32_t m_arrayId;
	uint32_t m_frameId;

	App(const char *name) : BaseApp(name, true, c_orange) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		m_viewports = makeViewports(2, 2);
		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_diffuse", &u_diffuse},
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			Attrs attrs = {
			        {"a.0",   1},
			        {"nelem", 3},
			};
			m_arrayId = spu_array_new(attrs);
		}
		// texture
		{
			for (auto &texture_id: m_textureIds) {
				Attrs attrs = {
				        {"target",      GL_TEXTURE_2D              },
				        {"iformat",     GL_RGBA8                   },
				        {"width",       int32_t(viewport(0).sx) / 2},
				        {"height",      int32_t(viewport(0).sy) / 2},
				        {"auto_mipmap", 0                          },
				        {"base_level",  0                          },
				        {"max_level",   0                          },
				        {"min_filter",  GL_NEAREST                 },
				        {"mag_filter",  GL_NEAREST                 },
				};
				texture_id = spu_texture_new(attrs);
			}
		}
		// framebuffer
		{
			auto r = Vec4f(1.0, 0.0, 0.0, 1.0);
			auto g = Vec4f(0.0, 1.0, 0.0, 1.0);
			auto b = Vec4f(0.0, 0.0, 1.0, 1.0);

			Attrs attrs = {
			        {"color0",    m_textureIds[0]},
			        {"color1",    m_textureIds[1]},
			        {"color2",    m_textureIds[2]},
			        {"viewport0", viewport(0)    },
			        {"bgcolor0",  r              },
			        {"bgcolor1",  g              },
			        {"bgcolor2",  b              },
			};
			m_frameId = spu_frame_new(attrs);
		}
	}

	void render() override
	{
		// Pass 1
		spu_frame_clear(m_frameId);

		// Pass 2
		{
			for (auto i = 0; i < 3; i++) {
				spu_frame_set(-1, "viewport0", m_viewports[i]);
				u_diffuse = m_textureIds[i];
				m_shader.use();
				spu_array_draw(m_arrayId, GL_TRIANGLES);
			}
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_400_fbo_rtt");
}  // namespace
}  // namespace spu
