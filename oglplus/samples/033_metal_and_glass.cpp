//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/brushed_metal.hpp>
#include <shapes/plane.hpp>
#include <shapes/wicker_torus.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldscreen;                                                            \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform mat4 u_nodeshadow;                                                             \n"
    "uniform mat4 u_texture_matrix;                                                         \n"
    "uniform vec3 u_eye_position;                                                           \n"
    "uniform vec3 u_light_position;                                                         \n"
    "uniform vec4 u_clip_plane;                                                             \n"
    "uniform float u_clip_direction;                                                        \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "in vec3 a_tangent;                                                                     \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out gl_PerVertex {                                                                     \n"
    "       vec4 gl_Position;                                                               \n"
    "       float gl_ClipDistance[];                                                        \n"
    "};                                                                                     \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_tangent;                                                                    \n"
    "out vec3 f_bitangent;                                                                  \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out vec3 f_view_dir;                                                                   \n"
    "out vec2 f_texcoord;                                                                   \n"
    "out vec4 f_position_shadow;                                                            \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position =                                                                   \n"
    "               u_nodeworld * a_position;                                               \n"
    "       gl_ClipDistance[0] =                                                            \n"
    "               u_clip_direction* dot(u_clip_plane, gl_Position);                       \n"
    "       f_light_dir = u_light_position - gl_Position.xyz;                               \n"
    "       f_view_dir = u_eye_position - gl_Position.xyz;                                  \n"
    "       f_normal =  (u_nodeworld * vec4(a_normal, 0.0)).xyz;                            \n"
    "       f_tangent = (u_nodeworld * vec4(a_tangent, 0.0)).xyz;                           \n"
    "       f_bitangent = cross(f_normal, f_tangent);                                       \n"
    "       f_texcoord = (u_texture_matrix * vec4(a_texcoord, 0, 1)).xy;                    \n"
    "       f_position_shadow = u_nodeshadow* gl_Position;                                  \n"
    "       gl_Position = u_worldscreen * gl_Position;                                      \n"
    "}                                                                                      \n"
};

// frag #0
const char *c_frag_shadow =  {
    "#version 330                                                                           \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_tangent;                                                                     \n"
    "in vec3 f_bitangent;                                                                   \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_view_dir;                                                                    \n"
    "in vec2 f_texcoord;                                                                    \n"
    "in vec4 f_position_shadow;                                                             \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "}                                                                                      \n"
};

// frag #1
const char *c_frag_light =  {
    "#version 330                                                                           \n"
    "uniform vec3 u_color;                                                                  \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_tangent;                                                                     \n"
    "in vec3 f_bitangent;                                                                   \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_view_dir;                                                                    \n"
    "in vec2 f_texcoord;                                                                    \n"
    "in vec4 f_position_shadow;                                                             \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float opacity = 1.0 - abs(dot(                                                  \n"
    "               normalize(f_normal),                                                    \n"
    "               normalize(f_view_dir)                                                   \n"
    "       ));                                                                             \n"
    "       final_color = vec4(u_color, 0.4 + sqrt(opacity)*0.6);                           \n"
    "}                                                                                      \n"
};

// frag #2
const char *c_frag_glass =  {
    "#version 330                                                                           \n"
    "uniform sampler2DShadow u_frame_shadow_texture;                                        \n"
    "uniform vec3 u_color;                                                                  \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_tangent;                                                                     \n"
    "in vec3 f_bitangent;                                                                   \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_view_dir;                                                                    \n"
    "in vec2 f_texcoord;                                                                    \n"
    "in vec4 f_position_shadow;                                                             \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 light_color = vec3(1.0, 1.0, 1.0);                                         \n"
    "       vec3 position_shadowtex = (                                                     \n"
    "               f_position_shadow.xyz /                                                 \n"
    "               f_position_shadow.w) * 0.5 + 0.5;                                       \n"
    "       if (                                                                            \n"
    "               position_shadowtex.x >= 0.0 &&                                          \n"
    "               position_shadowtex.x <= 1.0 &&                                          \n"
    "               position_shadowtex.y >= 0.0 &&                                          \n"
    "               position_shadowtex.y <= 1.0 &&                                          \n"
    "               position_shadowtex.z <= 1.0                                             \n"
    "       )                                                                               \n"
    "       {                                                                               \n"
    "               float shadow = 0.0;                                                     \n"
    "               const int sn = 12;                                                      \n"
    "               const float o = 1.0/128.0;                                              \n"
    "               for (int s=0; s!=sn; ++s)                                               \n"
    "               {                                                                       \n"
    "                       float r = float(s)/sn;                                          \n"
    "                       float a = 2.0*3.14151*r;                                        \n"
    "                       shadow += texture(                                              \n"
    "                               u_frame_shadow_texture,                                 \n"
    "                               position_shadowtex+                                     \n"
    "                               vec3(cos(a)*o*r, sin(a)*o*r, 0.0)                       \n"
    "                       );                                                              \n"
    "               }                                                                       \n"
    "               light_color *= shadow / sn;                                             \n"
    "       }                                                                               \n"
    "       vec3 reflection = reflect(                                                      \n"
    "               -normalize(f_light_dir),                                                \n"
    "               normalize(f_normal)                                                     \n"
    "       );                                                                              \n"
    "       float specular = pow(max(dot(                                                   \n"
    "               normalize(reflection),                                                  \n"
    "               normalize(f_view_dir))+0.05, 0.0), 128);                                \n"
    "       float diffuse = pow(pow(dot(                                                    \n"
    "               normalize(f_normal),                                                    \n"
    "               normalize(f_light_dir))+0.3, 2.0), 2.0);                                \n"
    "       float ambient = 0.3;                                                            \n"
    "       final_color = vec4(                                                             \n"
    "               u_color * ambient +                                                     \n"
    "               light_color * u_color * diffuse +                                       \n"
    "               light_color * specular,                                                 \n"
    "               0.4 + min(specular, 1.0)*0.3                                            \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

// frag #3
const char *c_frag_metal =  {
    "#version 330                                                                           \n"
    "uniform vec3 u_color1;                                                                 \n"
    "uniform vec3 u_color2;                                                                 \n"
    "uniform sampler2D u_metal_texture;                                                     \n"
    "uniform sampler2DShadow u_frame_shadow_texture;                                        \n"
    "uniform sampler2D u_glass_shadow_texture;                                              \n"
    "uniform int u_with_glass_shadow;                                                       \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_tangent;                                                                     \n"
    "in vec3 f_bitangent;                                                                   \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_view_dir;                                                                    \n"
    "in vec2 f_texcoord;                                                                    \n"
    "in vec4 f_position_shadow;                                                             \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 sample = texture(u_metal_texture, f_texcoord).rgb;                         \n"
    "       vec3 light_color = vec3(1.0, 1.0, 1.0);                                         \n"
    "       vec3 position_shadowtex = (                                                     \n"
    "               f_position_shadow.xyz /                                                 \n"
    "               f_position_shadow.w                                                     \n"
    "       ) * 0.5 + 0.5;                                                                  \n"
    "       if (                                                                            \n"
    "               position_shadowtex.x >= 0.0 &&                                          \n"
    "               position_shadowtex.x <= 1.0 &&                                          \n"
    "               position_shadowtex.y >= 0.0 &&                                          \n"
    "               position_shadowtex.y <= 1.0 &&                                          \n"
    "               position_shadowtex.z <= 1.0                                             \n"
    "       )                                                                               \n"
    "       {                                                                               \n"
    "               float shadow = 0.0;                                                     \n"
    "               const int sn = 12;                                                      \n"
    "               const float o = 1.0/128.0;                                              \n"
    "               for (int s=0; s!=sn; ++s)                                               \n"
    "               {                                                                       \n"
    "                       float r = float(s)/sn;                                          \n"
    "                       float a = 4.0*3.14151*r;                                        \n"
    "                       shadow += texture(                                              \n"
    "                               u_frame_shadow_texture,                                 \n"
    "                               position_shadowtex+                                     \n"
    "                               vec3(cos(a)*o*r, sin(a)*o*r, 0.0)                       \n"
    "                       );                                                              \n"
    "               }                                                                       \n"
    "               light_color *= (shadow / sn);                                           \n"
    "               if (u_with_glass_shadow != 0)                                           \n"
    "               {                                                                       \n"
    "                       vec4 filter_color = texture(                                    \n"
    "                               u_glass_shadow_texture,                                 \n"
    "                               position_shadowtex.st                                   \n"
    "                       );                                                              \n"
    "                       light_color = mix(                                              \n"
    "                               light_color,                                            \n"
    "                               light_color*filter_color.rgb,                           \n"
    "                               filter_color.a                                          \n"
    "                       );                                                              \n"
    "               }                                                                       \n"
    "       }                                                                               \n"
    "       vec3 normal = normalize(                                                        \n"
    "               2.0*f_normal +                                                          \n"
    "               (sample.r - 0.5)*f_tangent +                                            \n"
    "               (sample.g - 0.5)*f_bitangent                                            \n"
    "       );                                                                              \n"
    "       vec3 reflection = reflect(                                                      \n"
    "               -normalize(f_light_dir),                                                \n"
    "               normal                                                                  \n"
    "       );                                                                              \n"
    "       float specular = pow(max(dot(                                                   \n"
    "               normalize(reflection),                                                  \n"
    "               normalize(f_view_dir)                                                   \n"
    "       )+0.02, 0.0), 16+sample.b*48)*pow(0.4+sample.b*1.6, 4.0);                       \n"
    "                                                                                       \n"
    "       normal = normalize(f_normal*3.0 + normal);                                      \n"
    "                                                                                       \n"
    "       float diffuse = pow(max(dot(                                                    \n"
    "               normalize(normal),                                                      \n"
    "               normalize(f_light_dir)                                                  \n"
    "       ), 0.0), 2.0);                                                                  \n"
    "                                                                                       \n"
    "       float ambient = (dot(                                                           \n"
    "               normalize(f_normal),                                                    \n"
    "               vec3(0.0, 1.0, 0.0)                                                     \n"
    "       )*0.25 + 0.75) * 0.3;                                                           \n"
    "                                                                                       \n"
    "       vec3 u_color = mix(u_color1, u_color2, sample.b);                               \n"
    "       final_color = vec4(                                                             \n"
    "               u_color * ambient +                                                     \n"
    "               light_color * u_color * diffuse +                                       \n"
    "               light_color * specular,                                                 \n"
    "               1.0                                                                     \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class Uniforms {
public:
	Mat4f u_worldscreen;
	Mat4f u_nodeworld;
	Mat4f u_nodeshadow;
	Mat4f u_texture_matrix;
	Vec3f u_eye_position;
	Vec3f u_light_position;
	Vec4f u_clip_plane;
	float u_clip_direction;
	Vec3f u_color;
	Vec3f u_color1;
	Vec3f u_color2;
	uint32_t u_metal_texture;
	uint32_t u_frame_shadow_texture;
	uint32_t u_glass_shadow_texture;
	int32_t u_with_glass_shadow;

	Uniforms()
	{
		m_attrs = {
		        {"u_worldscreen",          &u_worldscreen         },
		        {"u_nodeworld",            &u_nodeworld           },
		        {"u_nodeshadow",           &u_nodeshadow          },
		        {"u_texture_matrix",       &u_texture_matrix      },
		        {"u_eye_position",         &u_eye_position        },
		        {"u_light_position",       &u_light_position      },
		        {"u_clip_plane",           &u_clip_plane          },
		        {"u_clip_direction",       &u_clip_direction      },
		        {"u_color",                &u_color               },
		        {"u_color1",               &u_color1              },
		        {"u_color2",               &u_color2              },
		        {"u_metal_texture",        &u_metal_texture       },
		        {"u_frame_shadow_texture", &u_frame_shadow_texture},
		        {"u_glass_shadow_texture", &u_glass_shadow_texture},
		        {"u_with_glass_shadow",    &u_with_glass_shadow   },
		};
	}

	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class MetalGlassArray : public shapes::Array {
public:
	enum {
		e_frame = 0,
		e_frame_shadow,
		e_glass,
		e_glass_shadow,
	};
	Uniforms &m_unifs;

	explicit MetalGlassArray(Uniforms &unifs) : m_unifs(unifs)
	{
		Attrs metal_shader_attrs = {
		        {"vert", c_vert      },
		        {"frag", c_frag_metal},
		};
		Attrs metal_shadow_shader_attrs = {
		        {"vert", c_vert       },
		        {"frag", c_frag_shadow},
		};
		Attrs glass_shader_attrs = {
		        {"vert", c_vert      },
		        {"frag", c_frag_glass},
		};
		Attrs glass_light_shader_attrs = {
		        {"vert", c_vert      },
		        {"frag", c_frag_light},
		};

		setMaxShaderType(4);  // 0:metal 1:metal_shadow 2:galss 3:glass_shadow
		initShader(metal_shader_attrs, (Attrs)unifs, 0);
		initShader(metal_shadow_shader_attrs, (Attrs)unifs, 1);
		initShader(glass_shader_attrs, (Attrs)unifs, 2);
		initShader(glass_light_shader_attrs, (Attrs)unifs, 3);
	}

	void drawFrame()
	{
		setShaderType(e_frame);
		drawSolid();
	}

	void drawFrameShadow()
	{
		setShaderType(e_frame_shadow);
		drawSolid();
	}

	void drawGlass()
	{
		setShaderType(e_glass);
		drawLight(false);
	}

	void drawGlassShadow()
	{
		setShaderType(e_glass_shadow);
		drawLight(true);
	}

private:
	void drawSolid()
	{
		auto driver = [](uint32_t phase) noexcept { return (phase <= 3); };
		setDriver(driver);
		draw(nullptr);
	}

	void drawLight(bool is_reverse)
	{
		SpuScopedRenderstate renderstate(true);
		renderstate.flags.clip_distance0 = true;
		renderstate.flags.blend = true;

		for (auto i = 0; i < 2; i++) {
			auto c = is_reverse ? 1 - i : i;
			m_unifs.u_clip_direction = (c == 0) ? -1 : +1;
			for (auto j = 0; j < 4; j++) {
				int32_t p = is_reverse ? 3 - j : j;
				renderstate.cull_face = p % 2 ? GL_FRONT : GL_BACK;

				auto driver = [&p](uint32_t phase) noexcept {
					if (p == 0 || p == 3) {
						return (phase == 4);
					}
					return (phase > 4);
				};
				useShader();
				renderstate.use();
				setDriver(driver);
				draw(nullptr);
			}
		}
	}
};

class App : public SpuPage {
public:
	static constexpr auto c_shadow_tex_side = 1024;

	SpuFrame m_frameShadow;
	SpuFrame m_glassShadow;

	Uniforms m_unifs;
	MetalGlassArray m_torus;
	MetalGlassArray m_plane;
	SpuTexture m_metalTexture;

	App(const char *name) : SpuPage(name, true), m_torus(m_unifs), m_plane(m_unifs) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		// array
		{
			auto torus_shape = shapes::WickerTorus();
			m_torus.initArray(torus_shape, {"position", "normal", "tangent", "texcoord"});

			auto plane_shape = shapes::Plane(Vec3f(9, 0, 0), Vec3f(0, 0, -9));
			m_plane.initArray(plane_shape, {"position", "normal", "tangent", "texcoord"});
		}

		// metal texture
		{
			auto image = images::BrushedMetalUByte(512, 512, 5120, -3, +3, 32, 128);

			Attrs attrs = {
			        {"target",     GL_TEXTURE_2D          },
			        {"iformat",    GL_RGB8                },
			        {"width",      image.width()          },
			        {"height",     image.height()         },
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter", GL_LINEAR              },
			        {"wrap_s",     GL_REPEAT              },
			        {"wrap_t",     GL_REPEAT              },
			        {"data",       image.data()           },
			};
			m_metalTexture.init(attrs);
			m_unifs.u_metal_texture = m_metalTexture.id();
		}

		// frame shadow (depth)
		{
			auto viewport = Rectf(0, 0, c_shadow_tex_side, c_shadow_tex_side);
			Attrs attrs = {
			        {"viewport0",          viewport                 },
			        {"depth.target",       GL_TEXTURE_2D            },
			        {"depth.iformat",      GL_DEPTH_COMPONENT32F    },
			        {"depth.min_filter",   GL_LINEAR                },
			        {"depth.mag_filter",   GL_LINEAR                },
			        {"depth.wrap_s",       GL_CLAMP_TO_EDGE         },
			        {"depth.wrap_t",       GL_CLAMP_TO_EDGE         },
			        {"depth.compare_mode", GL_COMPARE_REF_TO_TEXTURE},
			        {"depth.auto_mipmap",  0                        },
			};
			m_frameShadow.init(attrs);
			m_unifs.u_frame_shadow_texture = m_frameShadow.getBuffer("depth").id();
		}

		// grass shadow (color)
		{
			Vec4f bgcolor = eone<Vec4f>();
			auto viewport = Rectf(0, 0, c_shadow_tex_side, c_shadow_tex_side);
			Attrs attrs = {
			        {"viewport0",          viewport        },
			        {"color0.target",      GL_TEXTURE_2D   },
			        {"color0.iformat",     GL_RGBA8        },
			        {"color0.min_filter",  GL_LINEAR       },
			        {"color0.mag_filter",  GL_LINEAR       },
			        {"color0.wrap_s",      GL_CLAMP_TO_EDGE},
			        {"color0.wrap_t",      GL_CLAMP_TO_EDGE},
			        {"color0.auto_mipmap", 0               },
			        {"bgcolor0",           bgcolor         },
			};
			m_glassShadow.init(attrs);
			m_unifs.u_glass_shadow_texture = m_glassShadow.getBuffer("color0").id();
		}

		// renderstate
		{
			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.cull_face = true;
			renderstate.flags.depth_test = true;
			renderstate.flags.clip_distance0 = false;

			renderstate.cull_face = GL_BACK;

			renderstate.blend_func = {
			        GL_SRC_ALPHA,
			        GL_ONE_MINUS_SRC_ALPHA,
			        GL_SRC_ALPHA,
			        GL_ONE_MINUS_SRC_ALPHA,
			};

			renderstate.poly_offset = {1.0, 1.0};
			renderstate.use();
		}
	}

	void renderFrameShadowMap(
	        const Vec3f &light_position, const Mat4f &torus_matrix, const Mat4f &nodeshadow)
	{
		m_frameShadow.begin();
		m_frameShadow.clear();

		m_unifs.u_worldscreen = nodeshadow;
		m_unifs.u_eye_position = light_position;
		m_unifs.u_nodeworld = torus_matrix;

		SpuScopedRenderstate renderstate(true);
		renderstate.flags.fill_offset = true;
		renderstate.use();
		m_torus.drawFrameShadow();
		m_frameShadow.end();
	}

	void renderGlassShadowMap(
	        const Vec3f &light_position, const Vec3f &torus_center, const Mat4f &torus_matrix,
	        const Mat4f &nodeshadow)
	{
		m_glassShadow.begin();
		m_glassShadow.clear();

		m_unifs.u_worldscreen = nodeshadow;
		m_unifs.u_eye_position = light_position;
		m_unifs.u_nodeshadow = nodeshadow;
		m_unifs.u_light_position = light_position;
		m_unifs.u_nodeworld = (torus_matrix);

		auto n = normalize(light_position - torus_center);
		auto clip_plane = Plane3f(torus_center, n);

		m_unifs.u_clip_plane = clip_plane.eq;

		m_unifs.u_color = {0.6, 0.4, 0.1};

		SpuScopedRenderstate renderstate(true);
		renderstate.flags.depth_test = false;
		renderstate.use();

		m_torus.drawGlassShadow();
		m_glassShadow.end();
	}

	void renderImage(const Vec3f &torus_center, const Mat4f &torus_matrix, const Mat4f &nodeshadow)
	{
		auto &renderstate = SpuPage::getRenderstate();
		renderstate.cull_face = GL_BACK;
		renderstate.use();

		auto esec = getSeconds().current();
		auto viewscreen = math::perspective(viewport(0), 60, 1, 60);
		auto worldview = Mat4f::orbiting(ezero(), esec, 8, -3, 15, 0, 24, 45, 40, 20);
		auto eye_position = Vec3f(worldview.unitary_inverse().c[3]);

		m_unifs.u_nodeshadow = nodeshadow;
		m_unifs.u_worldscreen = viewscreen * worldview;
		m_unifs.u_eye_position = eye_position;

		auto n = normalize(eye_position - torus_center);
		auto clip_plane = Plane3f(torus_center, n);

		// Render the plane
		m_unifs.u_nodeworld = math::unit();

		m_unifs.u_texture_matrix = Mat4f(9, 0, 0, 0, 0, 9, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

		m_unifs.u_color1 = {1.0, 0.9, 0.8};
		m_unifs.u_color2 = {0.9, 0.8, 0.6};
		m_unifs.u_with_glass_shadow = 1;

		m_plane.drawFrame();

		// Render the torus
		m_unifs.u_nodeworld = (torus_matrix);

		m_unifs.u_texture_matrix = Mat4f(16, 0, 0, 0, 0, 4, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

		m_unifs.u_color1 = {0.9, 0.9, 0.9};
		m_unifs.u_color2 = {0.3, 0.3, 0.3};
		m_unifs.u_with_glass_shadow = 0;

		// metal-part
		m_torus.drawFrame();

		//  glass part
		m_unifs.u_color = {0.6, 0.4, 0.1};
		m_unifs.u_clip_plane = clip_plane.eq;  // need check
		m_torus.drawGlass();
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto light_position = Vec3f(16.0, 10.0, 9.0);
		auto torus_center = Vec3f(0.0, 1.5, 0.0);
		auto torus_matrix = math::unit().trans(torus_center)
		                  * math::unit().rot("z", -esec / 16.0 * math::two_pi());
		auto shadow_viewport = Rectf(0, 0, 1, 1);

		auto nodeshadow = math::perspective(shadow_viewport, 10, 1, 80)
		                * math::lookat(light_position, torus_center, ey());

		m_unifs.u_light_position = light_position;

		renderFrameShadowMap(light_position, torus_matrix, nodeshadow);
		renderGlassShadowMap(light_position, torus_center, torus_matrix, nodeshadow);
		renderImage(torus_center, torus_matrix, nodeshadow);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("033_metal_and_glass");
}  // namespace
}  // namespace spu::oglplus
