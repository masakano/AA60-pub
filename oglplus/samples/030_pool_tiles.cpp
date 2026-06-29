//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/load.hpp>
#include <images/filtered.hpp>
#include <images/random.hpp>
#include <shapes/plane.hpp>
#include <shapes/spiral_sphere.hpp>

namespace spu::oglplus {

namespace {
/* clang-format off */
const char *c_plane_vert =  {
    "#version 330                                                                           \n"
    "uniform vec3 u_light_position;                                                         \n"
    "uniform vec3 u_eye_position;                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out vec3 f_view_dir;                                                                   \n"
    "out vec4 f_refl_texcoord;                                                              \n"
    "out vec2 f_tile_texcoord;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld* a_position;                                          \n"
    "       f_light_dir = normalize(u_light_position - gl_Position.xyz);                    \n"
    "       f_view_dir = normalize(u_eye_position - gl_Position.xyz);                       \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "       f_refl_texcoord = gl_Position;                                                  \n"
    "       f_tile_texcoord = a_texcoord;                                                   \n"
    "}                                                                                      \n"
};

const char *c_plane_frag =  {
    "#version 330                                                                       \n"
    "uniform sampler2D u_rand_texture;                                                  \n"
    "uniform sampler2D u_pict_texture;                                                  \n"
    "uniform sampler2D u_tile_texture;                                                  \n"
    "uniform sampler2D u_norm_texture;                                                  \n"
    "uniform sampler2D u_lightmap;                                                      \n"
    "uniform uint u_tile_count;                                                         \n"
    "uniform float u_aspect;                                                            \n"
    "in vec3 f_light_dir;                                                               \n"
    "in vec3 f_view_dir;                                                                \n"
    "in vec4 f_refl_texcoord;                                                           \n"
    "in vec2 f_tile_texcoord;                                                           \n"
    "out vec4 final_color;                                                              \n"
    "void main()                                                                        \n"
    "{                                                                                  \n"
    "       vec3 normal = texture(                                                      \n"
    "               u_norm_texture,                                                     \n"
    "               f_tile_texcoord * u_tile_count                                      \n"
    "       ).rgb;                                                                      \n"
    "       vec3 light_refl = reflect(                                                  \n"
    "               -normalize(f_light_dir),                                            \n"
    "               normalize(normal)                                                   \n"
    "       );                                                                          \n"
    "       float diffuse = max(dot(                                                    \n"
    "               normal,                                                             \n"
    "               f_light_dir                                                         \n"
    "       ), 0.0);                                                                    \n"
    "       float specular = max(dot(                                                   \n"
    "               light_refl,                                                         \n"
    "               f_view_dir                                                          \n"
    "       ), 0.0);                                                                    \n"
    "       float plaster_light = 0.3 + max(diffuse, 0.0);                              \n"
    "       float tile_light = 0.3 + pow(diffuse, 2.0)*0.9 + pow(specular, 4.0)*2.5;    \n"
    "       vec2 refl_coord = f_refl_texcoord.xy;                                       \n"
    "       refl_coord /= f_refl_texcoord.w;                                            \n"
    "       refl_coord *= 0.5;                                                          \n"
    "       refl_coord += vec2(u_aspect*0.5, 0.5);                                      \n"
    "       refl_coord += vec2(normal.x, normal.z)*0.5;                                 \n"
    "       vec3 refl_color = texture(                                                  \n"
    "               u_lightmap,                                                         \n"
    "               refl_coord                                                          \n"
    "       ).rgb;                                                                      \n"
    "       vec3 tile_props = texture(                                                  \n"
    "               u_tile_texture,                                                     \n"
    "               f_tile_texcoord * u_tile_count                                      \n"
    "       ).rgb;                                                                      \n"
    "       float pict = texture(u_pict_texture, f_tile_texcoord).a;                    \n"
    "       float rand = texture(u_rand_texture, f_tile_texcoord).r;                    \n"
    "       float light_vs_dark =                                                       \n"
    "               mix( 0.1, 0.9, pict)+                                               \n"
    "               mix(-0.1, 0.1, rand);                                               \n"
    "       vec3 tile_color = mix(                                                      \n"
    "               vec3(0.1, 0.1, 0.5),                                                \n"
    "               vec3(0.4, 0.4, 0.9),                                                \n"
    "               light_vs_dark                                                       \n"
    "       );                                                                          \n"
    "       vec3 plaster_color = vec3(0.9, 0.9, 0.9);                                   \n"
    "       final_color = vec4(                                                         \n"
    "               mix(                                                                \n"
    "                       plaster_color * plaster_light,                              \n"
    "                       tile_color * tile_light,                                    \n"
    "                       tile_props.b                                                \n"
    "               ) * 0.5 +								                                                   \n"
    "               refl_color * tile_props.g * 0.6,                                    \n"
    "               1.0                                                                 \n"
    "       );                                                                          \n"
    "}                                                                                  \n"
};

const char *c_shape_vert =  {
    "#version 330                                                                           \n"
    "uniform vec3 u_light_position;                                                         \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out vec3 f_light_refl;                                                                 \n"
    "out vec3 f_view_dir;                                                                   \n"
    "out vec3 f_view_refl;                                                                  \n"
    "out vec3 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position =                                                                   \n"
    "               u_nodeworld * a_position;                                               \n"
    "       f_light_dir = u_light_position - gl_Position.xyz;                               \n"
    "       f_normal = mat3(u_nodeworld)*a_normal;                                          \n"
    "       f_light_refl = reflect(                                                         \n"
    "               -normalize(f_light_dir),                                                \n"
    "               normalize(f_normal)                                                     \n"
    "       );                                                                              \n"
    "       f_view_dir = (                                                                  \n"
    "               vec4(0.0, 0.0, 1.0, 1.0) * u_worldview                                  \n"
    "       ).xyz;                                                                          \n"
    "       f_view_refl = reflect(                                                          \n"
    "               -normalize(f_view_dir),                                                 \n"
    "               normalize(f_normal)                                                     \n"
    "       );                                                                              \n"
    "       f_color = vec3(0.3, 0.3, 0.7);                                                  \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen * u_worldview * gl_Position;                                \n"
    "}                                                                                      \n"
};

const char *c_shape_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2D u_pict_texture;                                                      \n"
    "uniform sampler2D u_tile_texture;                                                      \n"
    "uniform uint u_tile_count;                                                             \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_light_refl;                                                                  \n"
    "in vec3 f_view_dir;                                                                    \n"
    "in vec3 f_view_refl;                                                                   \n"
    "in vec3 f_color;                                                                       \n"
    "out vec4 final_color;                                                                  \n"

    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float lt_dist = length(f_light_dir);                                            \n"
    "       float diffuse = dot(                                                            \n"
    "               normalize(f_normal),                                                    \n"
    "               normalize(f_light_dir)                                                  \n"
    "       ) / lt_dist;                                                                    \n"
    "       float specular = dot(                                                           \n"
    "               normalize(f_light_refl),                                                \n"
    "               normalize(f_view_dir)                                                   \n"
    "       );                                                                              \n"
    "       vec3 light_color = vec3(1.0, 1.0, 1.0);                                         \n"
    "       vec2 refl_tex_coord = -vec2(                                                    \n"
    "               f_view_refl.x,                                                          \n"
    "               f_view_refl.z                                                           \n"
    "       );                                                                              \n"
    "       /*refl_tex_coord *= 0.25;*/                                                     \n"
    "       refl_tex_coord += vec2(0.5, 0.5);                                               \n"
    "       float pict = texture(u_pict_texture, refl_tex_coord).a;                         \n"
    "       float light_vs_dark = mix( 0.1, 0.9, pict);                                     \n"
    "       vec3 tile_color = mix(                                                          \n"
    "               vec3(0.2, 0.2, 0.6),                                                    \n"
    "               vec3(0.5, 0.5, 0.9),                                                    \n"
    "               light_vs_dark                                                           \n"
    "       );                                                                              \n"
    "       vec3 plaster_color = vec3(0.7, 0.7, 0.7);                                       \n"
    "       vec3 floor_color = mix(                                                         \n"
    "               plaster_color,                                                          \n"
    "               tile_color,                                                             \n"
    "               texture(u_tile_texture, refl_tex_coord*u_tile_count).b                  \n"
    "       );                                                                              \n"
    "       vec3 refl_color = mix(                                                          \n"
    "               vec3(0.5, 0.5, 0.4),                                                    \n"
    "               floor_color,                                                            \n"
    "               pow(max((-f_view_refl.y-0.5)*2.0, 0.0), 2.0)                            \n"
    "       );                                                                              \n"
    "       final_color = vec4(                                                             \n"
    "               f_color * 0.4 +                                                         \n"
    "               refl_color * 0.3 +                                                      \n"
    "               (light_color + f_color)*pow(max(2.5*diffuse, 0.0), 3) +                 \n"
    "               light_color * pow(max(specular, 0.0), 64),                              \n"
    "               1.0                                                                     \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class Uniforms {
public:
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_eye_position;
	Vec3f u_light_position;
	float u_aspect = 1.0;
	uint32_t u_tile_count = 0u;
	uint32_t u_pict_texture = 0u;
	uint32_t u_tile_texture = 0u;
	uint32_t u_rand_texture = 0u;
	uint32_t u_norm_texture = 0;
	uint32_t u_lightmap = 0;

	Uniforms()
	{
		m_attrs = {
		        {"u_viewsceen",      &u_viewsceen     },
                        {"u_worldview",      &u_worldview     },
		        {"u_nodeworld",      &u_nodeworld     },
                        {"u_aspect",         &u_aspect        },
		        {"u_tile_count",     &u_tile_count    },
                        {"u_eye_position",   &u_eye_position  },
		        {"u_light_position", &u_light_position},
                        {"u_pict_texture",   &u_pict_texture  },
		        {"u_tile_texture",   &u_tile_texture  },
                        {"u_rand_texture",   &u_rand_texture  },
		        {"u_norm_texture",   &u_norm_texture  },
                        {"u_lightmap",       &u_lightmap      },
		};
	}

	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class PlaneArray : public shapes::Array {
public:
	PlaneArray()
	{
		shapes::Plane plane_shape(ezero(), Vec3f(7, 0, 0), Vec3f(0, 0, -7), 48, 48);
		initArray(plane_shape, {"position", "texcoord"});
	}
};

class ReflectFrame : public SpuFrame {
public:
	void setup(int32_t texture_side)
	{
		auto border = Vec4f(0.5, 0.5, 0.4, 0.0);
		auto view_rect = Rectf(0, 0, texture_side, texture_side);
		Attrs attrs = {
		        {"viewport0",          view_rect            },
		        {"depth.target",       GL_RENDERBUFFER      },
		        {"depth.iformat",      GL_DEPTH_COMPONENT32F},
		        {"color0.target",      GL_TEXTURE_2D        },
		        {"color0.min_filter",  GL_LINEAR            },
		        {"color0.mag_filter",  GL_LINEAR            },
		        {"color0.wrap_s",      GL_CLAMP_TO_BORDER   },
		        {"color0.wrap_t",      GL_CLAMP_TO_BORDER   },
		        {"color0.border",      border               },
		        {"color0.iformat",     GL_RGB8              },
		        {"color0.auto_mipmap", 0                    },
		};
		SpuFrame::init(attrs);
	}
};

class App : public SpuPage {
public:
	static constexpr auto c_tile_texture_side = 64;

	Uniforms m_unifs;
	Vec4f m_bgcolor = {0.5, 0.5, 0.4, 0.0};

	ReflectFrame m_frame;
	PlaneArray m_plane;
	shapes::Array m_shape;

	SpuTexture m_randTexture;
	SpuTexture m_pictTexture;
	SpuTexture m_tileTexture;
	SpuTexture m_normTexture;

	App(const char *name) : SpuPage(name, true) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs plane_shader_attrs = {
		        {"frag", c_plane_frag},
		        {"vert", c_plane_vert},
		};

		Attrs shape_shader_attrs = {
		        {"frag", c_shape_frag},
		        {"vert", c_shape_vert},
		};

		m_plane.initShader(plane_shader_attrs, Attrs(m_unifs));

		m_shape.initShader(shape_shader_attrs, Attrs(m_unifs));
		m_shape.initArray(shapes::SpiralSphere(), {"position", "normal", "tangent", "texcoord"});

		Vec3f light_pos(3.0, 2.5, 2.0);
		m_unifs.u_light_position = {3.0, 2.5, 2.0};
		m_unifs.u_tile_count = c_tile_texture_side;

		{
			auto image = images::RandomRedUByte(c_tile_texture_side, c_tile_texture_side);

			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D },
                                {"iformat",     GL_R8         }, // need functional
			        {"width",       image.width() },
                                {"height",      image.height()},
			        {"min_filter",  GL_NEAREST    },
                                {"mag_filter",  GL_NEAREST    },
			        {"wrap_s",      GL_REPEAT     },
                                {"wrap_t",      GL_REPEAT     },
			        {"data",        image.data()  },
                                {"auto_mipmap", 0             },
			};
			m_randTexture.init(attrs);
			m_unifs.u_rand_texture = m_randTexture.id();
		}
		{
			auto image = images::LoadTexture("pool_pictogram");

			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D },
                                {"iformat",     GL_RGBA8      }, // need functional
			        {"width",       image.width() },
                                {"height",      image.height()},
			        {"min_filter",  GL_LINEAR     },
                                {"mag_filter",  GL_LINEAR     },
			        {"wrap_s",      GL_REPEAT     },
                                {"wrap_t",      GL_REPEAT     },
			        {"data",        image.data()  },
                                {"auto_mipmap", 0             },
			};
			m_pictTexture.init(attrs);
			m_unifs.u_pict_texture = m_pictTexture.id();
		}

		{
			auto tile_image = images::LoadTexture("small_tile");

			Attrs attrs = {
			        {"target",     GL_TEXTURE_2D          },
			        {"iformat",    GL_RGBA8               }, // need functional
			        {"width",      tile_image.width()     },
			        {"height",     tile_image.height()    },
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter", GL_LINEAR              },
			        {"wrap_s",     GL_REPEAT              },
			        {"wrap_t",     GL_REPEAT              },
			        {"data",       tile_image.data()      },
			};
			m_tileTexture.init(attrs);
			m_unifs.u_tile_texture = m_tileTexture.id();
		}

		{
			auto tile_image = images::LoadTexture("small_tile");

			auto norm_image = images::TransformComponents(
			        images::NormalMap(tile_image, images::NormalMap::FromRed()),
			        Mat4f(1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0,
			              0.0, 1.0));

			Attrs attrs = {
			        {"target",     GL_TEXTURE_2D          },
			        {"iformat",    GL_RGB8                }, // need functional
			        {"width",      norm_image.width()     },
			        {"height",     norm_image.height()    },
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter", GL_LINEAR              },
			        {"wrap_s",     GL_REPEAT              },
			        {"wrap_t",     GL_REPEAT              },
			};
			m_normTexture.init(attrs);
			m_normTexture.send(norm_image.data(), GL_RGB32F);
			m_unifs.u_norm_texture = m_normTexture.id();
		}
		Attrs frame_attrs = {
		        {"bgcolor0", m_bgcolor}
                };
		SpuPage::set(frame_attrs);

		{
			auto w = int32_t(viewport(0).sx);
			auto h = int32_t(viewport(0).sy);
			auto refl_texture_side = w > h ? h : w;
			auto view_rect
			        = Rectf((w - refl_texture_side) / 2, (h - refl_texture_side) / 2,
			                refl_texture_side, refl_texture_side);

			m_frame.setup(refl_texture_side);

			Attrs frame_attrs = {
			        {"viewport0", view_rect},
			        {"bgcolor0",  m_bgcolor},
			};
			m_frame.set(frame_attrs);
			m_unifs.u_lightmap = m_frame.getBuffer("color0").id();
			m_unifs.u_aspect = float(w) / float(h);
			m_unifs.u_viewsceen = math::perspective(viewport(0), 60, 1, 60);
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto reflection
		        = Mat4f(Vec4f(1, 0, 0, 0), Vec4f(0, -1, 0, 0), Vec4f(0, 0, 1, 0), Vec4f(0, 0, 0, 1));
		auto worldview = Mat4f::orbiting(ezero(), esec, 7.0, 2.5, 12.0, 0, 10, 45, -35, 7);
		auto &renderstate = SpuPage::getRenderstate();

		// render into the off-screen framebuffer
		{
			m_frame.begin();
			m_frame.clear();

			renderstate.flags.cull_face = true;
			renderstate.cull_face = GL_FRONT;
			renderstate.use();

			m_unifs.u_worldview = worldview * reflection;

			m_unifs.u_nodeworld = math::unit().trans({0.0, 0.6, 0.0});
			m_shape.draw(nullptr);
			m_frame.end();
		}

		// shape
		{
			renderstate.cull_face = GL_BACK;
			renderstate.use();
			m_unifs.u_worldview = worldview;
			m_shape.draw(nullptr);
		}

		// plane
		{
			renderstate.flags.cull_face = false;
			renderstate.use();
			m_unifs.u_worldview = worldview;
			m_unifs.u_eye_position = worldview.unitary_inverse().c[3];
			m_unifs.u_nodeworld = math::unit().trans({0.0, -0.5, 0.0});
			m_plane.draw(nullptr);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("030_pool_tiles");
}  // namespace
}  // namespace spu::oglplus
