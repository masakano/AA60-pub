//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/wicker_torus.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_shape_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out vec3 f_light_refl_dir;                                                             \n"
    "out vec3 f_view_dir;                                                                   \n"
    "out vec3 f_view_refl_dir;                                                              \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_normal = mat3(u_nodeworld)*a_normal;                                          \n"
    "       f_light_dir = u_light_pos - gl_Position.xyz;                                    \n"
    "       f_light_refl_dir = reflect(                                                     \n"
    "               -normalize(f_light_dir),                                                \n"
    "               normalize(f_normal)                                                     \n"
    "       );                                                                              \n"
    "       f_view_dir = (vec4(0.0, 0.0, 1.0, 1.0) * u_worldview).xyz;                      \n"
    "       f_view_refl_dir = reflect(                                                      \n"
    "               normalize(f_view_dir),                                                  \n"
    "               normalize(f_normal)                                                     \n"
    "       );                                                                              \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_shape_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_light_refl_dir;                                                              \n"
    "in vec3 f_view_dir;                                                                    \n"
    "in vec3 f_view_refl_dir;                                                               \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float l = length(f_light_dir);                                                  \n"
    "       float d = dot(                                                                  \n"
    "               normalize(f_normal),                                                    \n"
    "               normalize(f_light_dir)                                                  \n"
    "       ) / l;                                                                          \n"
    "       float s = dot(                                                                  \n"
    "               normalize(f_light_refl_dir),                                            \n"
    "               normalize(f_view_dir)                                                   \n"
    "       );                                                                              \n"
    "       vec3 ambi = vec3(0.6, 0.3, 0.5);                                                \n"
    "       vec3 diff = vec3(0.9, 0.7, 0.8);                                                \n"
    "       vec3 spec = vec3(1.0, 0.9, 0.95);                                               \n"
    "       final_color = vec4(                                                             \n"
    "               ambi * 0.3 +                                                            \n"
    "               diff * 0.7 * max(d, 0.0) +                                              \n"
    "               spec * pow(max(s, 0.0), 64),                                            \n"
    "               1.0                                                                     \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

const char *c_depth_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position =                                                                   \n"
    "               mat4(                                                                   \n"
    "                       1.0, 0.0, 0.0, -u_light_pos.x,                                  \n"
    "                       0.0, 1.0, 0.0, -u_light_pos.y,                                  \n"
    "                       0.0, 0.0, 1.0, -u_light_pos.z,                                  \n"
    "                       0.0, 0.0, 0.0,  1.0                                             \n"
    "               )*                                                                      \n"
    "               u_nodeworld *                                                           \n"
    "               mat4(                                                                   \n"
    "                       10.0,  0.0,  0.0,  0.0,                                         \n"
    "                        0.0, 10.0,  0.0,  0.0,                                         \n"
    "                        0.0,  0.0, 10.0,  0.0,                                         \n"
    "                        0.0,  0.0,  0.0,  1.0                                          \n"
    "               )*                                                                      \n"
    "               a_position;                                                             \n"
    "}                                                                                      \n"
};

const char *c_depth_geom =  {
    "#version 330                                                                           \n"
    "layout(triangles) in;                                                                  \n"
    "layout(triangle_strip, max_vertices = 18) out;                                         \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "const mat4 cube_face_matrix[6] = mat4[6](                                              \n"
    "       mat4(                                                                           \n"
    "                0.0,  0.0, -1.0,  0.0,                                                 \n"
    "                0.0, -1.0,  0.0,  0.0,                                                 \n"
    "               -1.0,  0.0,  0.0,  0.0,                                                 \n"
    "                0.0,  0.0,  0.0,  1.0                                                  \n"
    "       ), mat4(                                                                        \n"
    "                0.0,  0.0,  1.0,  0.0,                                                 \n"
    "                0.0, -1.0,  0.0,  0.0,                                                 \n"
    "                1.0,  0.0,  0.0,  0.0,                                                 \n"
    "                0.0,  0.0,  0.0,  1.0                                                  \n"
    "       ), mat4(                                                                        \n"
    "                1.0,  0.0,  0.0,  0.0,                                                 \n"
    "                0.0,  0.0, -1.0,  0.0,                                                 \n"
    "                0.0,  1.0,  0.0,  0.0,                                                 \n"
    "                0.0,  0.0,  0.0,  1.0                                                  \n"
    "       ), mat4(                                                                        \n"
    "                1.0,  0.0,  0.0,  0.0,                                                 \n"
    "                0.0,  0.0,  1.0,  0.0,                                                 \n"
    "                0.0, -1.0,  0.0,  0.0,                                                 \n"
    "                0.0,  0.0,  0.0,  1.0                                                  \n"
    "       ), mat4(                                                                        \n"
    "                1.0,  0.0,  0.0,  0.0,                                                 \n"
    "                0.0, -1.0,  0.0,  0.0,                                                 \n"
    "                0.0,  0.0, -1.0,  0.0,                                                 \n"
    "                0.0,  0.0,  0.0,  1.0                                                  \n"
    "       ), mat4(                                                                        \n"
    "               -1.0,  0.0,  0.0,  0.0,                                                 \n"
    "                0.0, -1.0,  0.0,  0.0,                                                 \n"
    "                0.0,  0.0,  1.0,  0.0,                                                 \n"
    "                0.0,  0.0,  0.0,  1.0                                                  \n"
    "       )                                                                               \n"
    ");                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       for (gl_Layer=0; gl_Layer!=6; ++gl_Layer)                                       \n"
    "       {                                                                               \n"
    "               for (int i=0; i!=3; ++i)                                                \n"
    "               {                                                                       \n"
    "                       gl_Position =                                                   \n"
    "                               u_viewsceen *                                           \n"
    "                               cube_face_matrix[gl_Layer]*                             \n"
    "                               gl_in[i].gl_Position;                                   \n"
    "                       EmitVertex();                                                   \n"
    "               }                                                                       \n"
    "               EndPrimitive();                                                         \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
};

const char *c_depth_frag =  {
    "#version 330                                                                           \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_FragDepth = gl_FragCoord.z;                                                  \n"
    "}                                                                                      \n"
};

const char *c_light_vert =  {
    "#version 330                                                                           \n"
    "in vec3 a_position;                                                                    \n"
    "out float g_z_offset;                                                                  \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "uniform int u_sample_count;                                                            \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float hp = (u_sample_count-1) * 0.5;                                            \n"
    "       g_z_offset = (gl_InstanceID - hp)/hp;                                           \n"
    "       gl_Position = vec4(a_position + u_light_pos, 1.0);                              \n"
    "}                                                                                      \n"
};

const char *c_light_geom =  {
    "#version 330                                                                           \n"
    "layout(points) in;                                                                     \n"
    "layout(triangle_strip, max_vertices = 4) out;                                          \n"
    "in float g_z_offset[];                                                                 \n"
    "out vec4 f_position;                                                                   \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform vec3 u_view_x;                                                                 \n"
    "uniform vec3 u_view_y;                                                                 \n"
    "uniform vec3 u_view_z;                                                                 \n"
    "uniform float u_light_vol_size;                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float zo = g_z_offset[0];                                                       \n"
    "       float yo[2] = float[2](-1.0, 1.0);                                              \n"
    "       float xo[2] = float[2](-1.0, 1.0);                                              \n"
    "       for (int j=0;j!=2;++j)                                                          \n"
    "       for (int i=0;i!=2;++i)                                                          \n"
    "       {                                                                               \n"
    "               f_position = vec4(                                                      \n"
    "                       gl_in[0].gl_Position.xyz+                                       \n"
    "                       u_view_x * xo[i] * u_light_vol_size+                            \n"
    "                       u_view_y * yo[j] * u_light_vol_size+                            \n"
    "                       u_view_z * zo    * u_light_vol_size,                            \n"
    "                       1.0                                                             \n"
    "               );                                                                      \n"
    "               gl_Position =                                                           \n"
    "                       u_viewsceen * u_worldview * f_position;                         \n"
    "               EmitVertex();                                                           \n"
    "       }                                                                               \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
};

const char *c_light_frag =  {
    "#version 330                                                                           \n"
    "in vec4 f_position;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "uniform samplerCubeShadow u_shadow_map;                                                \n"
    "uniform int u_sample_count;                                                            \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 light_dir = f_position.xyz - u_light_pos;                                  \n"
    "       vec4 shadow_coord = vec4(                                                       \n"
    "               normalize(light_dir),                                                   \n"
    "               length(light_dir)                                                       \n"
    "       );                                                                              \n"
    "       float s = texture(u_shadow_map, shadow_coord);                                  \n"
    "       float alpha = s / (u_sample_count * pow(length(light_dir), 2));                 \n"
    "       final_color = vec4(1.0, 1.0, 1.0, alpha);                                       \n"
    "}                                                                                      \n"
};

/* clang-format on */
class Uniforms {
public:
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	int32_t u_sample_count;
	float u_light_vol_size;
	uint32_t u_shadow_map;
	Vec3f u_light_pos;
	Vec3f u_view_x;
	Vec3f u_view_y;
	Vec3f u_view_z;

	Uniforms()
	{
		m_attrs = {
		        {"u_viewsceen",      &u_viewsceen     },
		        {"u_worldview",      &u_worldview     },
		        {"u_nodeworld",      &u_nodeworld     },
		        {"u_sample_count",   &u_sample_count  },
		        {"u_light_vol_size", &u_light_vol_size},
		        {"u_shadow_map",     &u_shadow_map    },
		        {"u_light_pos",      &u_light_pos     },
		        {"u_view_x",         &u_view_x        },
		        {"u_view_y",         &u_view_y        },
		        {"u_view_z",         &u_view_z        },
		};
	}

	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class App : public SpuPage {
public:
	enum {
		e_shape = 0,
		e_depth,
	};
	static constexpr auto c_tex_side = 128;
	static constexpr auto c_sample_count = 128;

	SpuFrame m_depthFrame;
	Uniforms m_unifs;
	shapes::Array m_shapeArray;
	SpuShader m_lightShader;
	SpuArray m_lightArray;

	App(const char *name) : SpuPage(name, true, {0.2, 0.05, 0.1, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		// programs
		{
			Attrs shape_shader_attrs = {
			        {"frag", c_shape_frag},
			        {"vert", c_shape_vert},
			};

			Attrs depth_shader_attrs = {
			        {"frag", c_depth_frag},
			        {"vert", c_depth_vert},
			        {"geom", c_depth_geom},
			};

			Attrs light_shader_attrs = {
			        {"frag", c_light_frag},
			        {"vert", c_light_vert},
			        {"geom", c_light_geom},
			};

			m_shapeArray.setMaxShaderType(2);  // 0:shape 1:depth
			m_shapeArray.initShader(shape_shader_attrs, Attrs(m_unifs), 0);
			m_shapeArray.initShader(depth_shader_attrs, Attrs(m_unifs), 1);

			shapes::loadShader(m_lightShader, light_shader_attrs, Attrs(m_unifs));
		}

		// model
		{
			m_shapeArray.initArray(shapes::WickerTorus(), {"position", "normal"});

			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.flags.cull_face = true;

			renderstate.cull_face = GL_BACK;
			renderstate.blend_func = {
			        GL_SRC_ALPHA,
			        GL_ONE,
			        GL_SRC_ALPHA,
			        GL_ONE,
			};
		}

		// light points
		{
			float position[3] = {0.0, 0.0, 0.0};
			Attrs attrs = {
			        {"shader_id",    m_lightShader.id()},
			        {"a.a_position", 3                 },
			        {"data",         position          },
			        {"nelem",        1                 },
			};
			m_lightArray.init(attrs);
		}

		// frame
		{
			auto viewport = Rectf(0, 0, c_tex_side, c_tex_side);
			Attrs attrs = {
			        {"viewport0",          viewport                 },
			        {"depth.target",       GL_TEXTURE_CUBE_MAP      },
			        {"depth.iformat",      GL_DEPTH_COMPONENT16     },
			        {"depth.min_filter",   GL_LINEAR                },
			        {"depth.mag_filter",   GL_LINEAR                },
			        {"depth.wrap_s",       GL_CLAMP_TO_EDGE         },
			        {"depth.wrap_t",       GL_CLAMP_TO_EDGE         },
			        {"depth.wrap_r",       GL_CLAMP_TO_EDGE         },
			        {"depth.compare_mode", GL_COMPARE_REF_TO_TEXTURE},
			        {"depth.compare_func", GL_LEQUAL                },
			        {"depth.max_level",    0                        },
			        {"depth.auto_mipmap",  0                        },
			};
			m_depthFrame.init(attrs);
			m_unifs.u_shadow_map = m_depthFrame.getBuffer("depth").id();
		}
		m_unifs.u_sample_count = c_sample_count;
		m_unifs.u_light_vol_size = 4;
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto worldview = Mat4f::orbiting(ezero(), esec, 6.5, 1.5, 16.0, 0, 12, 0, 90, 30);
		auto model = Mat4f(Quatf(esec / 10.0 * math::two_pi(), Vec3f(1, 1, 1)));
		auto light_pos = Vec3f(0.0, sin(esec / 7.0 * math::two_pi()) * 0.5, 0.0);

		m_unifs.u_viewsceen = math::perspective(viewport(0), 60, 1, 60);
		m_unifs.u_light_pos = light_pos;
		m_unifs.u_worldview = worldview;
		m_unifs.u_nodeworld = model;

		{
			float depth[6] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
			m_depthFrame.getBuffer("depth").send(
			        &depth, GL_DEPTH_COMPONENT32F, nullptr, nullptr, true);
		}

		{
			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.blend = false;
			renderstate.flags.fill = false;
			renderstate.flags.cull_face = false;
			renderstate.line_width = 2.0;
			renderstate.use();

			m_depthFrame.begin();
			m_shapeArray.setShaderType(e_depth);
			m_shapeArray.draw(nullptr);
			m_depthFrame.end();
		}

		// frame (line)
		{
			m_shapeArray.setShaderType(e_shape);
			m_shapeArray.draw(nullptr);
		}

		// light (point)
		{
			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.fill = true;
			renderstate.flags.blend = true;
			renderstate.use();

			m_unifs.u_view_x = worldview.unitary_inverse().c[0];
			m_unifs.u_view_y = worldview.unitary_inverse().c[1];
			m_unifs.u_view_z = worldview.unitary_inverse().c[2];

			m_lightShader.use();
			m_lightArray.draw(GL_POINTS, 0, 1, c_sample_count);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("030_shadow_volume");
}  // namespace
}  // namespace spu::oglplus
