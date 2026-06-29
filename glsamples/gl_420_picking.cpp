//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 420 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec3 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 1.0);                                \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "layout(binding = 1, r32f) writeonly uniform imageBuffer u_depth;                       \n"
    "uvec2 pickingcoord = uvec2(320, 240);                                                  \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    if(all(equal(pickingcoord, uvec2(gl_FragCoord.xy))))                               \n"
    "    {                                                                                  \n"
    "        imageStore(u_depth, 0, vec4(gl_FragCoord.z, 0, 0, 0));                         \n"
    "        color = vec4(1, 0, 1, 1);                                                      \n"
    "    }                                                                                  \n"
    "    else                                                                               \n"
    "        color = texture(u_diffuse, f_texcoord.st);                                     \n"
//    "   imageStore(u_depth, 0, vec4(2, 0, 0, 0));                                           \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[2];  // 0:diffuse 1:picking

	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_depth;
	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, c_orange) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			        {"u_diffuse",     &u_diffuse    },
			        {"u_depth",       &u_depth      },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array #0
		{
			Attrs attrs = {
			        {"shader_id",    m_shader.id()},
			        {"a.a_position", 2            },
			        {"a.a_texcoord", 2            },
			};
			m_arrayId = squareQuadsArray<v2fv2f_t>(attrs);
		}
		// texture #0
		{
			Attrs attrs = {
			        {"mag_filter", GL_LINEAR              },
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			};
			m_textureIds[0] = loadDDS("kueken7_bgra8_srgb.dds", attrs);
		}

		// texture #0
		{
			Attrs attrs = {
			        {"target",  GL_TEXTURE_BUFFER},
			        {"iformat", GL_R32F          },
			        {"size",    16               },
			};
			m_textureIds[1] = spu_texture_new(attrs);
		}
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = true;
			// renderstate.use();
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

		u_worldscreen = getCamera().worldscreen();
		u_diffuse = m_textureIds[0];
		u_depth = m_textureIds[1];
		m_shader.use();

		spu_array_draw(m_arrayId, GL_TRIANGLES);

#if 1
		// use spu_texture_recv
		float buf[4];

		spu_texture_recv(m_textureIds[1], buf, GL_R32F);
		spu_printf(0, "Depth: %2.3f %2.3f %2.3f %2.3f\n", buf[0], buf[1], buf[2], buf[3]);
#else
		// use spu_texture_map
		float *ptr = (float *)spu_texture_map(m_textureIds[1], GL_MAP_READ_BIT);
		float PickedDepth = *ptr;
		spu_texture_unmap(m_textureIds[1]);
		spu_printf(0, "Depth: %2.3f\n", PickedDepth);
#endif
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_420_picking");
}  // namespace
}  // namespace spu
