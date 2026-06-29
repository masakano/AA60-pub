//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/plane.hpp>
#include <shapes/spiral_sphere.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_plane_vert =  {
    "#version 330                                                                           \n"
    "uniform vec3 u_light_position;                                                         \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_light_dir = normalize(                                                        \n"
    "               u_light_position - gl_Position.xyz                                      \n"
    "       );                                                                              \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen * u_worldview * gl_Position;                                \n"
    "       f_texcoord = a_texcoord;                                                        \n"
    "}                                                                                      \n"
};

const char *c_plane_frag =  {
    "#version 330                                                                           \n"
    "uniform vec3 u_normal;                                                                 \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float checker = (                                                               \n"
    "               int(f_texcoord.x*18) % 2+                                               \n"
    "               int(f_texcoord.y*18) % 2                                                \n"
    "       ) % 2;                                                                          \n"
    "       vec3 color = mix(                                                               \n"
    "               vec3(0.2, 0.4, 0.9),                                                    \n"
    "               vec3(0.2, 0.2, 0.7),                                                    \n"
    "               checker                                                                 \n"
    "       );                                                                              \n"
    "       float d = dot(                                                                  \n"
    "               u_normal,                                                               \n"
    "               f_light_dir                                                             \n"
    "       );                                                                              \n"
    "       float intensity = 0.5 + pow(1.4*d, 2.0);                                        \n"
    "       final_color = vec4(color*intensity, 1.0);                                       \n"
    "}                                                                                      \n"
};

const char *c_shape_vert =  {
    "#version 330                                                                           \n"
    "uniform vec3 u_light_position;                                                         \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform vec4 u_clip_plane;                                                             \n"
    "uniform float u_clip_direction;                                                        \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out vec3 f_light_refl;                                                                 \n"
    "out vec3 f_view_dir;                                                                   \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position =                                                                   \n"
    "               u_nodeworld * a_position;                                               \n"
    "       gl_ClipDistance[0] =                                                            \n"
    "               u_clip_direction* dot(u_clip_plane, gl_Position);                       \n"
    "       f_light_dir = u_light_position - gl_Position.xyz;                               \n"
    "       f_normal = mat3(u_nodeworld)* a_normal;                                         \n"
    "       f_light_refl = reflect(                                                         \n"
    "               -normalize(f_light_dir),                                                \n"
    "               normalize(f_normal)                                                     \n"
    "       );                                                                              \n"
    "       f_view_dir = (vec4(0.0, 0.0, 1.0, 1.0)*u_worldview).xyz;                        \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "       vec3 tex_offs = mat3(u_worldview)*f_normal*0.05;                                \n"
    "       f_texcoord =                                                                    \n"
    "               vec2(0.5, 0.5) +                                                        \n"
    "               (gl_Position.xy/gl_Position.w)*0.5 +                                    \n"
    "               (tex_offs.z<0.0 ? tex_offs.xy : -tex_offs.xy);                          \n"
    "}                                                                                      \n"
};

const char *c_shape_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2D u_texture;                                                           \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_light_refl;                                                                  \n"
    "in vec3 f_view_dir;                                                                    \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 final_color;                                                                  \n"

    "float adj_lt(float i)                                                                  \n"
    "{                                                                                      \n"
    "       return i > 0.0 ? i : -0.7*i;                                                    \n"
    "}                                                                                      \n"

    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float l = length(f_light_dir);                                                  \n"
    "       float d = dot(                                                                  \n"
    "               normalize(f_normal),                                                    \n"
    "               normalize(f_light_dir)                                                  \n"
    "       ) / l;                                                                          \n"
    "       float s = dot(                                                                  \n"
    "               normalize(f_light_refl),                                                \n"
    "               normalize(f_view_dir)                                                   \n"
    "       );                                                                              \n"
    "       vec3 lt = vec3(1.0, 1.0, 1.0);                                                  \n"
    "       vec3 tex = texture(u_texture, f_texcoord).rgb;                                  \n"
    "       final_color = vec4(                                                             \n"
    "               tex * 0.5 +                                                             \n"
    "               (lt + tex) * 1.5 * adj_lt(d) +                                          \n"
    "               lt * pow(adj_lt(s), 64),                                                \n"
    "               1.0                                                                     \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	static constexpr auto c_tex_side = 512;

	shapes::Array m_planeArray;
	shapes::Array m_shapeArray;
	SpuTexture m_texture;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec4f u_clip_plane = ezero<Vec4f>();
	Vec3f u_light_position = ezero();
	Vec3f u_normal = ezero();
	float u_clip_direction = 0;
	uint32_t u_texture = 0u;

	void buildPlaneArray()
	{
		Attrs shader_attrs = {
		        {"vert", c_plane_vert},
		        {"frag", c_plane_frag},
		};

		Attrs unif_attrs = {
		        {"u_light_position", &u_light_position},
		        {"u_viewsceen",      &u_viewsceen     },
		        {"u_worldview",      &u_worldview     },
		        {"u_nodeworld",      &u_nodeworld     },
		        {"u_normal",         &u_normal        },
		};
		m_planeArray.initShader(shader_attrs, unif_attrs);

		shapes::Plane plane_shape(Vec3f(2, 0, 0), Vec3f(0, 0, -2));
		m_planeArray.initArray(plane_shape, {"position", "texcoord"});
		u_normal = plane_shape.normal();
	}

	void buildShapeArray()
	{
		Attrs shader_attrs = {
		        {"vert", c_shape_vert},
		        {"frag", c_shape_frag},
		};

		Attrs unif_attrs = {
		        {"u_light_position", &u_light_position},
		        {"u_viewsceen",      &u_viewsceen     },
		        {"u_worldview",      &u_worldview     },
		        {"u_nodeworld",      &u_nodeworld     },
		        {"u_clip_plane",     &u_clip_plane    },
		        {"u_clip_direction", &u_clip_direction},
		        {"u_texture",        &u_texture       },
		};
		m_shapeArray.initShader(shader_attrs, unif_attrs);
		m_shapeArray.initArray(shapes::SpiralSphere(), {"position", "normal"});
	}

	App(const char *name) : SpuPage(name, true, {0.8, 0.8, 0.7, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		buildPlaneArray();
		buildShapeArray();

		u_light_position = {3.0, 3.0, 3.0};

		{
			Attrs tex_attrs = {
			        {"target",     GL_TEXTURE_2D},
                                {"iformat",    GL_RGB8      },
                                {"width",      c_tex_side   },
			        {"height",     c_tex_side   },
                                {"min_filter", GL_LINEAR    },
			};
			m_texture.init(tex_attrs);
			u_texture = m_texture.id();
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();

		u_viewsceen = math::perspective(viewport(0), 48, 1, 15);
		u_worldview = Mat4f::orbiting(ezero(), esec, 5.5, 0, 0, 0, 10, 45, 30, 7);
		u_nodeworld = math::unit().trans({0.0, -1.1, 0.0});

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.cull_face = false;
		renderstate.flags.clip_distance0 = false;
		renderstate.use();

		m_planeArray.draw(nullptr);

		u_clip_plane = Vec4f(u_worldview.c[0].z, u_worldview.c[1].z, u_worldview.c[2].z, 0);
		u_nodeworld = math::unit().rot("x", -esec / 12.0 * math::two_pi());

		renderstate.flags.cull_face = true;
		renderstate.flags.clip_distance0 = true;
		renderstate.use();

		float clip_dirs[2] = {-1.0, 1.0};
		uint32_t facing_dirs[2] = {GL_FRONT, GL_BACK};

		for (float clip_dir: clip_dirs) {
			u_clip_direction = clip_dir;

			for (unsigned int facing_dir: facing_dirs) {
				auto viewport_w = int32_t(viewport(0).sx);
				auto viewport_h = int32_t(viewport(0).sy);
				int32_t src_loc[4] = {
				        int(c_tex_side == viewport_w ? 0 :
				                                       (viewport_w - c_tex_side) / 2),  // oax
				        int(c_tex_side == viewport_h ? 0 :
				                                       (viewport_h - c_tex_side) / 2),  // oy
				        0,                                                              // oy
				        0,                                                              // ol
				};
				spu_texture_copy(u_texture, 0, nullptr, src_loc, nullptr);
				renderstate.cull_face = facing_dir;
				renderstate.use();

				m_shapeArray.draw(nullptr);
			}
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("028_glass_shape");
}  // namespace
}  // namespace spu::oglplus
