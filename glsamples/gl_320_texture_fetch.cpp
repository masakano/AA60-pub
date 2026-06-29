//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position,0.0,1.0);                             \n"
    "}                                                                                      \n"
};
const char *c_frag = {
    "#version 330                                                                           \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec2 size = textureSize(u_diffuse, 0) - 1;                                         \n"
    "    vec2 texcoord = f_texcoord * size;                                                 \n"
    "    ivec2 coord = ivec2(texcoord);                                                     \n"
    "    vec4 texel00 = texelFetch(u_diffuse, coord + ivec2(0, 0), 0);                      \n"
    "    vec4 texel10 = texelFetch(u_diffuse, coord + ivec2(1, 0), 0);                      \n"
    "    vec4 texel11 = texelFetch(u_diffuse, coord + ivec2(1, 1), 0);                      \n"
    "    vec4 texel01 = texelFetch(u_diffuse, coord + ivec2(0, 1), 0);                      \n"
    "    vec2 samplecoord = fract(texcoord.xy);                                             \n"
    "    vec4 texel0 = mix(texel00, texel01, samplecoord.y);                                \n"
    "    vec4 texel1 = mix(texel10, texel11, samplecoord.y);                                \n"
    "    color = mix(texel0, texel1, samplecoord.x);                                        \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;

	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, c_sky) {}

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
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			const std::vector<v2fv2f_t> c_vertices = squareTriangles<v2fv2f_t>();

			Attrs attrs = {
			        {"shader_id",    m_shader.id()    },
                                {"a.a_position", 2                },
                                {"a.a_texcoord", 2                },
			        {"data",         c_vertices.data()},
                                {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
		}
		// texture
		{
			u_diffuse = loadDDS("kueken7_rgba_dxt5_unorm.dds", Attrs());
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_texture_fetch");
}  // namespace
}  // namespace spu
