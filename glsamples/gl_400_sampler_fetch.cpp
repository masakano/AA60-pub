//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 400 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 400 core                                                                      \n"
    "uniform sampler2DArray u_diffuse;                                                      \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "vec4 trilinearLod(in sampler2DArray u_sampler, in int layer, in float level, in vec2 texcoord)\n"
    "{                                                                                      \n"
    "    int levelMin = int(ceil(level));                                                   \n"
    "    int levelMax = int(floor(level));                                                  \n"
    "    vec2 sizeMin = textureSize(u_sampler, levelMin).xy - 1;                            \n"
    "    vec2 sizeMax = textureSize(u_sampler, levelMax).xy - 1;                            \n"
    "    vec2 texcoordMin = texcoord * sizeMin;                                             \n"
    "    vec2 texcoordMax = texcoord * sizeMax;                                             \n"
    "    ivec3 coordMin = ivec3(texcoord * sizeMin, layer);                                 \n"
    "    ivec3 coordMax = ivec3(texcoord * sizeMax, layer);                                 \n"
    "    vec4 texelMin00 = texelFetch(u_sampler, coordMin + ivec3(0, 0, layer), levelMin);  \n"
    "    vec4 texelMin10 = texelFetch(u_sampler, coordMin + ivec3(1, 0, layer), levelMin);  \n"
    "    vec4 texelMin11 = texelFetch(u_sampler, coordMin + ivec3(1, 1, layer), levelMin);  \n"
    "    vec4 texelMin01 = texelFetch(u_sampler, coordMin + ivec3(0, 1, layer), levelMin);  \n"
    "    vec4 texelMax00 = texelFetch(u_sampler, coordMax + ivec3(0, 0, layer), levelMax);  \n"
    "    vec4 texelMax10 = texelFetch(u_sampler, coordMax + ivec3(1, 0, layer), levelMax);  \n"
    "    vec4 texelMax11 = texelFetch(u_sampler, coordMax + ivec3(1, 1, layer), levelMax);  \n"
    "    vec4 texelMax01 = texelFetch(u_sampler, coordMax + ivec3(0, 1, layer), levelMax);  \n"
    "    vec4 texelMin0 = mix(texelMin00, texelMin01, fract(texcoordMin.y));                \n"
    "    vec4 texelMin1 = mix(texelMin10, texelMin11, fract(texcoordMin.y));                \n"
    "    vec4 texelMin  = mix(texelMin0, texelMin1, fract(texcoordMin.x));                  \n"
    "    vec4 texelMax0 = mix(texelMax00, texelMax01, fract(texcoordMax.y));                \n"
    "    vec4 texelMax1 = mix(texelMax10, texelMax11, fract(texcoordMax.y));                \n"
    "    vec4 texelMax  = mix(texelMax0, texelMax1, fract(texcoordMax.x));                  \n"
    "    return mix(texelMax, texelMin, fract(level));                                      \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec2 level = textureQueryLod(u_diffuse, f_texcoord);                               \n"
    "    color = trilinearLod(u_diffuse, 0, level.x, f_texcoord);                           \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;

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
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			Attrs attrs = {
			        {"shader_id",    m_shader.id()},
			        {"a.a_position", 2            },
			        {"a.a_texcoord", 2            },
			};
			m_arrayId = squareQuadsArray<v2fv2f_t>(attrs);
		}

		// texture
		{
			Attrs attrs = {
			        {"target", GL_TEXTURE_2D_ARRAY},
			};
			u_diffuse = loadDDS("kueken7_rgba_dxt5_unorm.dds", attrs);
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

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_400_sampler_fetch");
}  // namespace
}  // namespace spu
