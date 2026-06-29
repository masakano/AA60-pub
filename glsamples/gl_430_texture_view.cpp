//
// App :
//
#include "base_app.h"
#include <ssys/half_float.h>

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
    "out vec2 g_texcoord;                                                                   \n"
    "out int g_instance;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    g_texcoord = a_texcoord;                                                           \n"
    "    g_instance = int(gl_InstanceID);                                                   \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 1.0);                                \n"
    "}                                                                                      \n"
};

const char *c_geom = {
    "#version 420 core                                                                      \n"
    "layout(triangles, invocations = 1) in;                                                 \n"
    "layout(triangle_strip, max_vertices = 4) out;                                          \n"
    "in gl_PerVertex                                                                        \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "} gl_in[];                                                                             \n"
    "in vec2 g_texcoord[];                                                                  \n"
    "in int g_instance[];                                                                   \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    for(int i = 0; i < gl_in.length(); ++i)                                            \n"
    "    {                                                                                  \n"
    "        gl_Position = gl_in[i].gl_Position;                                            \n"
    "        f_texcoord = g_texcoord[i];                                                    \n"
    "        gl_ViewportIndex = g_instance[i];                                              \n"
    "        EmitVertex();                                                                  \n"
    "    }                                                                                  \n"
    "    EndPrimitive();                                                                    \n"
    "}                                                                                      \n"
};
	
const char *c_frag = {
    "#version 420 core                                                                      \n"
    "#extension GL_ARB_texture_query_levels : require                                       \n"
    "#extension GL_ARB_fragment_layer_viewport : require                                    \n"
    "uniform sampler2DArray u_diffuse[2];                                                   \n"
    "uniform sampler2DArray u_diffuse_d;                                                    \n"
    "vec4 textureNearest(                                                                   \n"
    "    in sampler2DArray sampler, const vec2 texcoord)                                    \n"
    "{                                                                                      \n"
    "    //int lodNearest = int(round(textureQueryLod(sampler, texcoord).x)); // something wrong\n"
    "    int lodNearest = 0;                                                                \n"
    "    ivec2 texture_size = textureSize(sampler, lodNearest).xy;                          \n"
    "    ivec2 texelcoord = ivec2(texture_size * texcoord);                                 \n"
    "    return texelFetch(sampler, ivec3(texelcoord, 0), lodNearest);                      \n"
    "}                                                                                      \n"
    "vec4 fetchBilinear(                                                                    \n"
    "    in sampler2DArray sampler, in vec2 Interpolant, in ivec2 texelcoords[4], in int lod)\n"
    "{                                                                                      \n"
    "    vec4 texel00 = texelFetch(sampler, ivec3(texelcoords[0], 0), lod);                 \n"
    "    vec4 texel10 = texelFetch(sampler, ivec3(texelcoords[1], 0), lod);                 \n"
    "    vec4 texel11 = texelFetch(sampler, ivec3(texelcoords[2], 0), lod);                 \n"
    "    vec4 texel01 = texelFetch(sampler, ivec3(texelcoords[3], 0), lod);                 \n"
    "    vec4 texel0 = mix(texel00, texel01, Interpolant.y);                                \n"
    "    vec4 texel1 = mix(texel10, texel11, Interpolant.y);                                \n"
    "    return mix(texel0, texel1, Interpolant.x);                                         \n"
    "}                                                                                      \n"
    "vec4 textureBilinear(                                                                  \n"
    "    in sampler2DArray sampler, const vec2 texcoord)                                    \n"
    "{                                                                                      \n"
    "    int lod = int(round(textureQueryLod(sampler, texcoord).x));                        \n"
    "    ivec2 size = textureSize(sampler, lod).xy;                                         \n"
    "    vec2 texelcoord = texcoord * size - 0.5;                                           \n"
    "    ivec2 texelIndex = ivec2(texelcoord);                                              \n"
    "    ivec2 texelcoords[] = ivec2[4](                                                    \n"
    "        texelIndex + ivec2(0, 0),                                                      \n"
    "        texelIndex + ivec2(1, 0),                                                      \n"
    "        texelIndex + ivec2(1, 1),                                                      \n"
    "        texelIndex + ivec2(0, 1));                                                     \n"
    "    return fetchBilinear(                                                              \n"
    "        sampler,                                                                       \n"
    "        fract(texelcoord),                                                             \n"
    "        texelcoords,                                                                   \n"
    "        lod);                                                                          \n"
    "}                                                                                      \n"
    "vec4 textureBilinearLod(                                                               \n"
    "    in sampler2DArray sampler, const vec2 texcoord, in int lod)                        \n"
    "{                                                                                      \n"
    "    ivec2 size = textureSize(sampler, lod).xy;                                         \n"
    "    vec2 texelcoord = texcoord * size - 0.5;                                           \n"
    "    ivec2 texelIndex = ivec2(texelcoord);                                              \n"
    "    ivec2 texelcoords[] = ivec2[4](                                                    \n"
    "        texelIndex + ivec2(0, 0),                                                      \n"
    "        texelIndex + ivec2(1, 0),                                                      \n"
    "        texelIndex + ivec2(1, 1),                                                      \n"
    "        texelIndex + ivec2(0, 1));                                                     \n"
    "    return fetchBilinear(                                                              \n"
    "        sampler,                                                                       \n"
    "        fract(texelcoord),                                                             \n"
    "        texelcoords,                                                                   \n"
    "        lod);                                                                          \n"
    "}                                                                                      \n"
    "vec4 textureTrilinear(                                                                 \n"
    "    in sampler2DArray sampler, const vec2 texcoord)                                    \n"
    "{                                                                                      \n"
    "    float lod = textureQueryLod(sampler, texcoord).x;                                  \n"
    "    int lodMin = int(floor(lod));                                                      \n"
    "    int lodMax = int(ceil(lod));                                                       \n"
    "    vec4 texelMin = textureBilinearLod(sampler, texcoord, lodMin);                     \n"
    "    vec4 texelMax = textureBilinearLod(sampler, texcoord, lodMax);                     \n"
    "    return mix(texelMin, texelMax, fract(lod));                                        \n"
    "}                                                                                      \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    if(textureQueryLevels(u_diffuse[gl_ViewportIndex]) == 1) {                         \n"
    "        color = textureNearest(u_diffuse[gl_ViewportIndex], f_texcoord.st);            \n"
    "    }                                                                                  \n"
    "    else {                                                                             \n"
    "        color = textureTrilinear(u_diffuse[gl_ViewportIndex], f_texcoord.st);          \n"
    "    }                                                                                  \n"
    "    //color = texture(u_diffuse[0], vec3(f_texcoord.st, 0));                           \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[3];  // 0:texture 1:view_A 2:view_B
	uint32_t m_arrayId;
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse[2];
	uint32_t u_diffuse_d;

	App(const char *name) : BaseApp(name, true, c_orange) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"geom", c_geom},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			        {"u_diffuse",     &u_diffuse[0] }, // This does now work?
			        {"u_diffuse_d",   &u_diffuse_d  }, // for debug
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
			        {"iformat", GL_RGBA8}
                        };  // force to use GL_RGBA8

			m_textureIds[0] = spu_inventory_new("texture", "fish.jpg", attrs);
			spu_inventory_sync(/*"texture",*/ m_textureIds[0], false);  // necessary

			m_textureIds[1]
			        = spu_texture_alias(m_textureIds[0], GL_TEXTURE_2D_ARRAY, GL_RGBA8, 1, 0);

			m_textureIds[2] = spu_texture_alias(
			        m_textureIds[0], GL_TEXTURE_2D_ARRAY, GL_SRGB8_ALPHA8, 1, 0);
		}
	}

	void render() override
	{
		auto w = viewport(0).sx / 2;
		auto h = viewport(0).sy;
		auto viewport0 = Rectf(0, 0, w, h);
		auto viewport1 = Rectf(w, 0, w, h);

		getCamera().setViewscreen(viewport0, 180.0 * 0.25, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		u_diffuse[0] = m_textureIds[1];
		u_diffuse[1] = m_textureIds[2];
		u_diffuse_d = m_textureIds[0];  // for debug

		m_shader.use();

		Attrs attrs = {
		        {"viewport0", viewport0},
		        {"viewport1", viewport1},
		};
		spu_frame_set(-1, attrs);
		spu_array_draw(m_arrayId, GL_TRIANGLES, 0, 0, 2);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_430_texture_view");
}  // namespace
}  // namespace spu
