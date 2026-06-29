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
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "uniform ivec2 u_offset;                                                                \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "vec4 catmullRom(const vec4 A, const vec4 B, const vec4 C, const vec4 D, float S)       \n"
    "{                                                                                      \n"
    "    mat4 CatmullRom = mat4(                                                            \n"
    "        vec4(-1, 2,-1, 0),                                                             \n"
    "        vec4( 3,-5, 0, 2),                                                             \n"
    "        vec4(-3, 4, 1, 0),                                                             \n"
    "        vec4( 1,-1, 0, 0));                                                            \n"
    "    vec4 expo = vec4(S * S * S, S * S, S, 1);                                          \n"
    "    return 0.5 * expo * CatmullRom * mat4(                                             \n"
    "        A[0], B[0], C[0], D[0],                                                        \n"
    "        A[1], B[1], C[1], D[1],                                                        \n"
    "        A[2], B[2], C[2], D[2],                                                        \n"
    "        A[3], B[3], C[3], D[3]);                                                       \n"
    "}                                                                                      \n"
    "vec4 textureCatmullrom(const sampler2D sampler, const vec2 texcoord, const vec2 offset)\n"
    "{                                                                                      \n"
    "    vec4 texel00 = textureOffset(sampler, texcoord + offset, ivec2(-1,-1));            \n"
    "    vec4 texel10 = textureOffset(sampler, texcoord + offset, ivec2( 0,-1));            \n"
    "    vec4 texel20 = textureOffset(sampler, texcoord + offset, ivec2( 1,-1));            \n"
    "    vec4 texel30 = textureOffset(sampler, texcoord + offset, ivec2( 2,-1));            \n"
    "    vec4 texel01 = textureOffset(sampler, texcoord + offset, ivec2(-1, 0));            \n"
    "    vec4 texel11 = textureOffset(sampler, texcoord + offset, ivec2( 0, 0));            \n"
    "    vec4 texel21 = textureOffset(sampler, texcoord + offset, ivec2( 1, 0));            \n"
    "    vec4 texel31 = textureOffset(sampler, texcoord + offset, ivec2( 2, 0));            \n"
    "    vec4 texel02 = textureOffset(sampler, texcoord + offset, ivec2(-1, 1));            \n"
    "    vec4 texel12 = textureOffset(sampler, texcoord + offset, ivec2( 0, 1));            \n"
    "    vec4 texel22 = textureOffset(sampler, texcoord + offset, ivec2( 1, 1));            \n"
    "    vec4 texel32 = textureOffset(sampler, texcoord + offset, ivec2( 2, 1));            \n"
    "    vec4 texel03 = textureOffset(sampler, texcoord + offset, ivec2(-1, 2));            \n"
    "    vec4 texel13 = textureOffset(sampler, texcoord + offset, ivec2( 0, 2));            \n"
    "    vec4 texel23 = textureOffset(sampler, texcoord + offset, ivec2( 1, 2));            \n"
    "    vec4 texel33 = textureOffset(sampler, texcoord + offset, ivec2( 2, 2));            \n"
    "    vec2 splinecoord = fract(textureSize(sampler, 0) * texcoord);                      \n"
    "    vec4 row0 = catmullRom(texel00, texel10, texel20, texel30, splinecoord.x);         \n"
    "    vec4 row1 = catmullRom(texel01, texel11, texel21, texel31, splinecoord.x);         \n"
    "    vec4 row2 = catmullRom(texel02, texel12, texel22, texel32, splinecoord.x);         \n"
    "    vec4 row3 = catmullRom(texel03, texel13, texel23, texel33, splinecoord.x);         \n"
    "    return catmullRom(row0, row1, row2, row3, splinecoord.y);                          \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    ivec2 texture_size = textureSize(u_diffuse, 0);                                    \n"
    "    color = textureCatmullrom(u_diffuse, f_texcoord, vec2(u_offset) / vec2(texture_size));\n"
    "}                                                                                      \n"
};

const char *c_frag_bicubic = {
    "#version 330                                                                           \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "vec4 catmullRom(const vec4 A, const vec4 B, const vec4 C, const vec4 D, float S)       \n"
    "{                                                                                      \n"
    "    mat4 CatmullRom = mat4(                                                            \n"
    "        vec4(-1.0, 2.0,-1.0, 0.0),                                                     \n"
    "        vec4( 3.0,-5.0, 0.0, 2.0),                                                     \n"
    "        vec4(-3.0, 4.0, 1.0, 0.0),                                                     \n"
    "        vec4( 1.0,-1.0, 0.0, 0.0));                                                    \n"
    "    vec4 expo = vec4(S * S * S, S * S, S, 1);                                          \n"
    "    return 0.5 * expo * CatmullRom * mat4(                                             \n"
    "        A[0], B[0], C[0], D[0],                                                        \n"
    "        A[1], B[1], C[1], D[1],                                                        \n"
    "        A[2], B[2], C[2], D[2],                                                        \n"
    "        A[3], B[3], C[3], D[3]);                                                       \n"
    "}                                                                                      \n"
    "vec4 textureCatmullrom(const sampler2D sampler, const vec2 texcoord)                   \n"
    "{                                                                                      \n"
    "    ivec2 size = textureSize(sampler, 0) - ivec2(1);                                   \n"
    "    ivec2 texelcoord = ivec2(size * texcoord);                                         \n"
    "    vec4 texel00 = texelFetchOffset(sampler, texelcoord, 0, ivec2(-1,-1));             \n"
    "    vec4 texel10 = texelFetchOffset(sampler, texelcoord, 0, ivec2( 0,-1));             \n"
    "    vec4 texel20 = texelFetchOffset(sampler, texelcoord, 0, ivec2( 1,-1));             \n"
    "    vec4 texel30 = texelFetchOffset(sampler, texelcoord, 0, ivec2( 2,-1));             \n"
    "    vec4 texel01 = texelFetchOffset(sampler, texelcoord, 0, ivec2(-1, 0));             \n"
    "    vec4 texel11 = texelFetchOffset(sampler, texelcoord, 0, ivec2( 0, 0));             \n"
    "    vec4 texel21 = texelFetchOffset(sampler, texelcoord, 0, ivec2( 1, 0));             \n"
    "    vec4 texel31 = texelFetchOffset(sampler, texelcoord, 0, ivec2( 2, 0));             \n"
    "    vec4 texel02 = texelFetchOffset(sampler, texelcoord, 0, ivec2(-1, 1));             \n"
    "    vec4 texel12 = texelFetchOffset(sampler, texelcoord, 0, ivec2( 0, 1));             \n"
    "    vec4 texel22 = texelFetchOffset(sampler, texelcoord, 0, ivec2( 1, 1));             \n"
    "    vec4 texel32 = texelFetchOffset(sampler, texelcoord, 0, ivec2( 2, 1));             \n"
    "    vec4 texel03 = texelFetchOffset(sampler, texelcoord, 0, ivec2(-1, 2));             \n"
    "    vec4 texel13 = texelFetchOffset(sampler, texelcoord, 0, ivec2( 0, 2));             \n"
    "    vec4 texel23 = texelFetchOffset(sampler, texelcoord, 0, ivec2( 1, 2));             \n"
    "    vec4 texel33 = texelFetchOffset(sampler, texelcoord, 0, ivec2( 2, 2));             \n"
    "    vec2 splinecoord = fract(size * texcoord);                                         \n"
    "    vec4 row0 = catmullRom(texel00, texel10, texel20, texel30, splinecoord.x);         \n"
    "    vec4 row1 = catmullRom(texel01, texel11, texel21, texel31, splinecoord.x);         \n"
    "    vec4 row2 = catmullRom(texel02, texel12, texel22, texel32, splinecoord.x);         \n"
    "    vec4 row3 = catmullRom(texel03, texel13, texel23, texel33, splinecoord.x);         \n"
    "    return catmullRom(row0, row1, row2, row3, splinecoord.y);                          \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = textureCatmullrom(u_diffuse, f_texcoord);                                  \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shaders[2];  // 0:offset 1:bicubic
	uint32_t m_arrayIds[2];
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	Vec4i u_offset;
	std::vector<Rectf> m_viewports;

	App(const char *name) : BaseApp(name, true, c_orange) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		m_viewports = makeViewports(2, 1);

		// program
		{
			const char *verts[] = {
			        c_vert,
			        c_vert,
			};

			const char *frags[] = {
			        c_frag,
			        c_frag_bicubic,
			};

			for (auto &shader: m_shaders) {
				int32_t i = &shader - &m_shaders[0];
				Attrs shader_attrs = {
				        {"vert", verts[i]},
				        {"frag", frags[i]},
				};
				Attrs unif_attrs = {
				        {"u_worldscreen", &u_worldscreen},
				        {"u_diffuse",     &u_diffuse    },
				        {"u_offset",      &u_offset     },
				};
				loadShader(shader, shader_attrs, unif_attrs);
			}
		}
		// array
		{
			for (auto i = 0; i < 2; i++) {
				Attrs attrs = {
				        {"shader_id",    m_shaders[i].id()},
				        {"a.a_position", 2                },
				        {"a.a_texcoord", 2                },
				};
				m_arrayIds[i] = squareQuadsArray<v2fv2f_t>(attrs, Vec2f(1.0, 0.5));
			}
		}

		// texture
		{
			Attrs attrs = {
			        {"min_filter", GL_NEAREST_MIPMAP_NEAREST},
			        {"mag_filter", GL_NEAREST               },
			        {"wrap_s",     GL_CLAMP_TO_EDGE         },
			        {"wrap_t",     GL_CLAMP_TO_EDGE         },
			};
			u_diffuse = loadDDS("kueken7_bgra8_srgb.dds", attrs);
		}
	}

	void render() override
	{
		u_offset = {63, 107, 0, 0};

		for (auto i = 0; i < 2; i++) {
			spu_frame_set(-1, "viewport0", m_viewports[i]);

			getCamera().setViewscreen(viewport(0), 45, 0.1, 1000.0);
			u_worldscreen = getCamera().worldscreen();

			m_shaders[i].use();
			spu_array_draw(m_arrayIds[i], GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_texture_offset");
}  // namespace
}  // namespace spu
