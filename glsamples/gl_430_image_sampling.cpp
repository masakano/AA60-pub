//
// App :
//
#include "base_app.h"
namespace spu {
namespace {
/* clang-format off */
const char *c_vert = {
    "#version 430                                                                           \n"
    "#define COMMON        0                                                                \n"
    "uniform mat4 u_worldscreen;                                                            \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "out vec2 f_texcoord;                                                                   \n"
    "flat out int f_instance;                                                               \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_instance = int(gl_InstanceID);                                                   \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position =                                                                      \n"
    "             u_worldscreen * vec4(a_position.x - 2 + float(gl_InstanceID)*2, a_position.y, 0, 1);\n"
    "}                                                                                      \n"
};
	
const char *c_frag = {
    "#version 430                                                                           \n"
    "#extension GL_ARB_shader_image_size : require                                          \n"
    "layout(binding = 0, rgba8) uniform image2D u_diffuse[3];                               \n"
    "in vec2 f_texcoord;                                                                    \n"
    "flat in int f_instance;                                                                \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "vec4 fetchBilinear(in vec2 interpolant, in ivec2 texelcoords[4])                       \n"
    "{                                                                                      \n"
    "    vec4 texel00, texel10, texel11, texel01;                                           \n"
    "    // since inidirect index not allowed                                               \n"
    "    if (f_instance == 0) {                                                             \n"
    "        texel00 = imageLoad(u_diffuse[0], texelcoords[0]);                             \n"
    "        texel10 = imageLoad(u_diffuse[0], texelcoords[1]);                             \n"
    "        texel11 = imageLoad(u_diffuse[0], texelcoords[2]);                             \n"
    "        texel01 = imageLoad(u_diffuse[0], texelcoords[3]);                             \n"
    "    }                                                                                  \n"
    "    if (f_instance == 1) {                                                             \n"
    "        texel00 = imageLoad(u_diffuse[1], texelcoords[0]);                             \n"
    "        texel10 = imageLoad(u_diffuse[1], texelcoords[1]);                             \n"
    "        texel11 = imageLoad(u_diffuse[1], texelcoords[2]);                             \n"
    "        texel01 = imageLoad(u_diffuse[1], texelcoords[3]);                             \n"
    "    }                                                                                  \n"
    "    if (f_instance == 2) {                                                             \n"
    "        texel00 = imageLoad(u_diffuse[2], texelcoords[0]);                             \n"
    "        texel10 = imageLoad(u_diffuse[2], texelcoords[1]);                             \n"
    "        texel11 = imageLoad(u_diffuse[2], texelcoords[2]);                             \n"
    "        texel01 = imageLoad(u_diffuse[2], texelcoords[3]);                             \n"
    "    }                                                                                  \n"
    "    vec4 texel0 = mix(texel00, texel01, interpolant.y);                                \n"
    "    vec4 texel1 = mix(texel10, texel11, interpolant.y);                                \n"
    "    return mix(texel0, texel1, interpolant.x);                                         \n"
    "}                                                                                      \n"
    "vec4 imageBilinear(in vec2 texcoord)                                                   \n"
    "{                                                                                      \n"
    "    ivec2 size;                                                                        \n"
    "    if (f_instance == 0) {                                                             \n"
    "        size = imageSize(u_diffuse[0]);                                                \n"
    "    }                                                                                  \n"
    "    if (f_instance == 1) {                                                             \n"
    "        size = imageSize(u_diffuse[1]);                                                \n"
    "    }                                                                                  \n"
    "    if (f_instance == 2) {                                                             \n"
    "        size = imageSize(u_diffuse[2]);                                                \n"
    "    }                                                                                  \n"
    "    vec2 texelcoord = texcoord * size - 0.5;                                           \n"
    "    ivec2 texelIndex = ivec2(texelcoord);                                              \n"
    "    ivec2 texelcoords[] = ivec2[4](                                                    \n"
    "        texelIndex + ivec2(0, 0),                                                      \n"
    "        texelIndex + ivec2(1, 0),                                                      \n"
    "        texelIndex + ivec2(1, 1),                                                      \n"
    "        texelIndex + ivec2(0, 1));                                                     \n"
    "    return fetchBilinear(                                                              \n"
    "        fract(texelcoord),                                                             \n"
    "        texelcoords);                                                                  \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = imageBilinear(f_texcoord);                                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse[3];

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
                                {"u_diffuse",     &u_diffuse[0] }
                        };
			loadShader(m_shader, shader_attrs, unif_attrs);

			// debug for explicit slot
			auto binding = 1;
			spu_shader_set(m_shader.id(), "u_diffuse.binding", &binding);
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
			        {"iformat",    GL_RGBA8               },
			        {"mag_filter", GL_LINEAR              },
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			};
			u_diffuse[0] = loadDDS("kueken7_bgra8_srgb.dds", attrs);
			u_diffuse[1] = u_diffuse[0];
			u_diffuse[2] = u_diffuse[0];
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 1000.0);
		u_worldscreen = getCamera().worldscreen();

		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES, 0, 0, 3);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_430_image_sampling");
}  // namespace
}  // namespace spu
