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
    "#version 330                                                                             \n"
    "uniform mat4 u_worldscreen;                                                              \n"
    "uniform mat4 u_nodeworld;                                                                \n"
    "uniform mat4 u_light_proj_matrix;                                                        \n"
    "uniform mat4 u_texture_matrix;                                                           \n"
    "uniform vec3 u_eye_position;                                                             \n"
    "uniform vec3 u_light_position;                                                           \n"
    "in vec4 a_position;                                                                      \n"
    "in vec3 a_normal;                                                                        \n"
    "in vec2 a_texcoord;                                                                      \n"
    "out gl_PerVertex {                                                                       \n"
    "       vec4 gl_Position;                                                                 \n"
    "};                                                                                       \n"
    "out float g_noise;                                                                       \n"
    "out vec3 g_normal;                                                                       \n"
    "out vec3 g_light_dir, g_view_dir;                                                        \n"
    "out vec2 g_texcoord;                                                                     \n"
    "out vec4 g_light_texcoord;                                                               \n"
    "void main()                                                                              \n"
    "{                                                                                        \n"
    "       gl_Position =                                                                     \n"
    "               u_nodeworld *                                                             \n"
    "               a_position;                                                               \n"
    "       g_noise = noise1(a_position.x+a_position.y+a_position.z);                         \n"
    "       g_light_dir = u_light_position - gl_Position.xyz;                                 \n"
    "       g_view_dir = u_eye_position - gl_Position.xyz;                                    \n"
    "       g_normal =  (u_nodeworld * vec4(a_normal, 0.0)).xyz;                              \n"
    "       g_texcoord = mat2(u_texture_matrix) * a_texcoord;                                 \n"
    "       g_light_texcoord = u_light_proj_matrix* gl_Position;                              \n"
    "       gl_Position = u_worldscreen * gl_Position;                                        \n"
    "}                                                                                        \n"
};

const char *c_shadow_frag =  {
    "#version 330                                                                           \n"
    "in float g_noise;                                                                      \n"
    "in vec3 g_normal;                                                                      \n"
    "in vec3 g_light_dir, g_view_dir;                                                       \n"
    "in vec2 g_texcoord;                                                                    \n"
    "in vec4 g_light_texcoord;                                                              \n"
    "void main(){ }                                                                         \n"
};

const char *c_line_geom =  {
    "#version 330                                                                           \n"
    "layout(lines) in;                                                                      \n"
    "layout(line_strip, max_vertices=4) out;                                                \n"
    "in gl_PerVertex {                                                                      \n"
    "       vec4 gl_Position;                                                               \n"
    "} gl_in[];                                                                             \n"
    "in float g_noise[];                                                                    \n"
    "in vec3 g_normal[];                                                                    \n"
    "in vec3 g_light_dir[];                                                                 \n"
    "in vec3 g_view_dir[];                                                                  \n"
    "in vec2 g_texcoord[];                                                                  \n"
    "in vec4 g_light_texcoord[];                                                            \n"
    "out gl_PerVertex {                                                                     \n"
    "       vec4 gl_Position;                                                               \n"
    "};                                                                                     \n"
    "out float f_opacity;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec4 p0 = gl_in[0].gl_Position;                                                 \n"
    "       vec4 p1 = gl_in[1].gl_Position;                                                 \n"
    "       vec4 n0 = vec4(g_normal[0], 0.0);                                               \n"
    "       vec4 n1 = vec4(g_normal[1], 0.0);                                               \n"
    "       float dp = pow(length(p1 - p0), 0.25);                                          \n"
    "       float l0 = cos(g_noise[0]*1.618)*dp;                                            \n"
    "       float l1 = sin(g_noise[1]*1.618)*dp;                                            \n"
    "       vec4 v0 = p0 + n0*l0*0.01;                                                      \n"
    "       vec4 v1 = p1 + n1*l1*0.01;                                                      \n"
    "       vec4 v = v1 - v0;                                                               \n"
    "       gl_Position = v0 - v*abs(l1*0.41);                                              \n"
    "       f_opacity = 0.0;                                                                \n"
    "       EmitVertex();                                                                   \n"
    "       gl_Position = p0;                                                               \n"
    "       f_opacity = 1.0;                                                                \n"
    "       EmitVertex();                                                                   \n"
    "       gl_Position = p1;                                                               \n"
    "       f_opacity = 1.0;                                                                \n"
    "       EmitVertex();                                                                   \n"
    "       gl_Position = v1 + v*abs(l0*0.44);                                              \n"
    "       f_opacity = 0.0;                                                                \n"
    "       EmitVertex();                                                                   \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
};

const char *c_line_frag =  {
    "#version 330                                                                           \n"
    "in float f_opacity;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = vec4(0.0, 0.0, 0.0, 0.1+0.4*f_opacity);                           \n"
    "}                                                                                      \n"
};

const char *c_sketch_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler3D u_sketch_texture;                                                    \n"
    "uniform sampler2DShadow u_shadow_texture;                                              \n"
    "in float g_noise;                                                                      \n"
    "in vec3 g_normal;                                                                      \n"
    "in vec3 g_light_dir, g_view_dir;                                                       \n"
    "in vec2 g_texcoord;                                                                    \n"
    "in vec4 g_light_texcoord;                                                              \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float u_light_mult = 1.0;                                                       \n"
    "       vec3 light_proj_coord = (                                                       \n"
    "               g_light_texcoord.xyz /                                                  \n"
    "               g_light_texcoord.w                                                      \n"
    "       ) * 0.5 + 0.5;                                                                  \n"
    "       if (                                                                            \n"
    "               light_proj_coord.x >= 0.0 &&                                            \n"
    "               light_proj_coord.x <= 1.0 &&                                            \n"
    "               light_proj_coord.y >= 0.0 &&                                            \n"
    "               light_proj_coord.y <= 1.0 &&                                            \n"
    "               light_proj_coord.z <= 1.0                                               \n"
    "       )                                                                               \n"
    "       {                                                                               \n"
    "               float shadow = 0.0;                                                     \n"
    "               const int sn = 12;                                                      \n"
    "               const float o = 1.0/32.0;                                               \n"
    "               for (int s=0; s!=sn; ++s)                                               \n"
    "               {                                                                       \n"
    "                       float r = float(s)/sn;                                          \n"
    "                       float a = 4.0*3.14151*r;                                        \n"
    "                       shadow += texture(                                              \n"
    "                               u_shadow_texture,                                       \n"
    "                               light_proj_coord+                                       \n"
    "                               vec3(cos(a)*o*r, sin(a)*o*r, 0.0)                       \n"
    "                       );                                                              \n"
    "               }                                                                       \n"
    "               u_light_mult *= (shadow / sn);                                          \n"
    "       }                                                                               \n"
    "       vec3 light_refl = reflect(                                                      \n"
    "               -normalize(g_light_dir),                                                \n"
    "               normalize(g_normal)                                                     \n"
    "       );                                                                              \n"
    "       float specular = pow(max(dot(                                                   \n"
    "               normalize(light_refl),                                                  \n"
    "               normalize(g_view_dir)                                                   \n"
    "       )+0.02, 0.0)*1.1, 2.0);                                                         \n"
    "       float diffuse = max(dot(                                                        \n"
    "               normalize(g_normal),                                                    \n"
    "               normalize(g_light_dir)                                                  \n"
    "       ), 0.0)*3.5;                                                                    \n"
    "       float ambient = 0.1;                                                            \n"
    "       float light = ambient + (diffuse + specular)*u_light_mult;                      \n"
    "       float shadow = clamp(2.0 - light, 0.0, 1.0);                                    \n"
    "       vec3 sample = texture(u_sketch_texture, vec3(g_texcoord, shadow)).rgb;          \n"
    "       final_color = vec4(0.0, 0.0, 0.0, pow(sample.b*(0.5 + shadow*0.5), 0.5));       \n"
    "}                                                                                      \n"
};

/* clang-format on */
class Uniforms {
public:
	Mat4f u_worldscreen;
	Mat4f u_nodeworld;
	Mat4f u_light_proj_matrix;
	Mat4f u_texture_matrix;
	Vec3f u_eye_position;
	Vec3f u_light_position;
	uint32_t u_sketch_texture = 0u;
	uint32_t u_shadow_texture = 0u;

	Uniforms()
	{
		m_attrs = {
		        {"u_sketch_texture",    &u_sketch_texture   },
		        {"u_shadow_texture",    &u_shadow_texture   },
		        {"u_worldscreen",       &u_worldscreen      },
		        {"u_nodeworld",         &u_nodeworld        },
		        {"u_light_proj_matrix", &u_light_proj_matrix},
		        {"u_texture_matrix",    &u_texture_matrix   },
		        {"u_eye_position",      &u_eye_position     },
		        {"u_light_position",    &u_light_position   },
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
		e_line,
		e_shadow,
	};

	static constexpr auto c_sketch_texture_layers = size_t(8);
	static constexpr auto c_shadow_texture_side = size_t(1024);

	SpuFrame m_shadowFrame;

	Uniforms m_unifs;

	shapes::EdgeArray m_plane;
	shapes::EdgeArray m_torus;
	SpuTexture m_sketchTexture;

	App(const char *name) : SpuPage(name, true, {1.0, 0.9, 0.8, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shadow_shader_attrs = {
		        {"vert", c_vert       },
		        {"frag", c_shadow_frag},
		};

		Attrs sketch_shader_attrs = {
		        {"vert", c_vert       },
		        {"frag", c_sketch_frag},
		};

		Attrs line_shader_attrs = {
		        {"vert", c_vert     },
		        {"geom", c_line_geom},
		        {"frag", c_line_frag},
		};

		m_plane.setMaxShaderType(3);  // 0:shape 1:line 2:shadow
		m_plane.initShader(sketch_shader_attrs, (Attrs)m_unifs, 0);
		m_plane.initShader(line_shader_attrs, (Attrs)m_unifs, 1);
		m_plane.initShader(shadow_shader_attrs, (Attrs)m_unifs, 2);

		m_torus.setMaxShaderType(3);  // 0:shape 1:line 2:shadow
		m_torus.initShader(sketch_shader_attrs, (Attrs)m_unifs, 0);
		m_torus.initShader(line_shader_attrs, (Attrs)m_unifs, 1);
		m_torus.initShader(shadow_shader_attrs, (Attrs)m_unifs, 2);

		// model
		auto plane_shape = shapes::Plane(ezero(), Vec3f(9, 0, 0), Vec3f(0, 0, -9), 9, 9);
		m_plane.initArray(plane_shape, {"position", "normal", "tangent", "texcoord"});

		auto torus_shape = shapes::WickerTorus();
		m_torus.initArray(torus_shape, {"position", "normal", "tangent", "texcoord"});

		// sketch tex
		{
			std::vector<uint8_t> pix(512 * 512 * 3 * c_sketch_texture_layers);
			for (auto i = 0u; i != c_sketch_texture_layers; ++i) {
				auto size = size_t(512 * 512);
				auto image = images::BrushedMetalUByte(
				        512, 512, 64 + i * 128, -(2 + i * 4), +(2 + i * 4), 64, 256 - i * 4);
				memcpy(&pix[size * 3 * i], image.data(), size * 3);
			}

			Attrs attrs = {
			        {"target",      GL_TEXTURE_3D           },
			        {"iformat",     GL_RGB8                 },
			        {"width",       512                     },
			        {"height",      512                     },
			        {"depth",       c_sketch_texture_layers },
			        {"data",        (const void *)pix.data()},
			        {"min_filter",  GL_LINEAR_MIPMAP_LINEAR },
			        {"mag_filter",  GL_LINEAR               },
			        {"wrap_s",      GL_REPEAT               },
			        {"wrap_t",      GL_REPEAT               },
			        {"wrap_r",      GL_CLAMP_TO_EDGE        },
			        {"auto_mipmap", 1                       },
			};
			m_sketchTexture.init(attrs);
			m_unifs.u_sketch_texture = m_sketchTexture.id();
		}

		// shadow tex
		{
			auto viewport = Rectf(0, 0, c_shadow_texture_side, c_shadow_texture_side);
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
			m_shadowFrame.init(attrs);
			m_unifs.u_shadow_texture = m_shadowFrame.getBuffer("depth").id();
		}

		// renderstate
		{
			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.flags.cull_face = true;
			renderstate.depth_func = GL_LEQUAL;
			renderstate.blend_func = {
			        GL_SRC_ALPHA,
			        GL_ONE_MINUS_SRC_ALPHA,
			        GL_SRC_ALPHA,
			        GL_ONE_MINUS_SRC_ALPHA,
			};
			renderstate.poly_offset = {1.0, 1.0};
			// renderstate.use();
		}
	}

	void renderShadow(
	        const Vec3f &u_light_position, const Mat4f &torus_matrix, const Mat4f &u_light_proj_matrix)
	{
		m_shadowFrame.begin();
		m_shadowFrame.clear();

		SpuScopedRenderstate renderstate(true);
		renderstate.cull_face = GL_BACK;
		renderstate.flags.fill_offset = true;
		renderstate.use();

		m_unifs.u_worldscreen = u_light_proj_matrix;
		m_unifs.u_eye_position = u_light_position;

		m_unifs.u_nodeworld = torus_matrix;

		m_torus.setShaderType(e_shadow);
		m_torus.draw(nullptr);
		m_shadowFrame.end();
	}

	void renderModel(const Mat4f &torus_matrix, const Mat4f &u_light_proj_matrix)
	{
		auto &renderstate = SpuPage::getRenderstate();
		renderstate.cull_face = GL_BACK;
		renderstate.use();

		auto esec = getSeconds().current();
		auto viewscreen = math::perspective(viewport(0), 60, 1, 60);
		auto worldview = Mat4f::orbiting(ezero(), esec, 7, 3, 14, 0, 26, 45, 40, 17);

		m_unifs.u_light_proj_matrix = u_light_proj_matrix;
		m_unifs.u_worldscreen = viewscreen * worldview;
		m_unifs.u_eye_position = worldview.unitary_inverse().c[3];

		// render into the depth buffer
		{
			renderstate.flags.fill_offset = true;
			renderstate.write_mask = {0, 0, 0, 0, 1};
			renderstate.use();

			m_unifs.u_nodeworld = math::unit();

			m_plane.setShaderType(e_shadow);
			m_plane.draw(nullptr);

			m_unifs.u_nodeworld = torus_matrix;

			m_torus.setShaderType(e_shadow);
			m_torus.draw(nullptr);
		}

		// render solid
		{
			renderstate.write_mask = {1, 1, 1, 1, 1};
			renderstate.flags.fill_offset = false;
			renderstate.flags.blend = true;
			renderstate.use();

			m_unifs.u_nodeworld = math::unit();

			m_unifs.u_texture_matrix = Mat4f(
			        Vec4f(3, 0, 0, 0), Vec4f(0, 3, 0, 0), Vec4f(0, 0, 1, 0), Vec4f(0, 0, 0, 1));

			m_plane.setShaderType(e_shape);
			m_plane.draw(nullptr);

			m_unifs.u_nodeworld = torus_matrix;

			m_unifs.u_texture_matrix
			        = {Vec4f(8, 0, 0, 0), Vec4f(0, 2, 0, 0), Vec4f(0, 0, 1, 0), Vec4f(0, 0, 0, 1)};

			m_torus.setShaderType(e_shape);
			m_torus.setDriver([](uint32_t phase) noexcept { return phase < 4; });
			m_torus.draw(nullptr);

			m_unifs.u_texture_matrix = {
			        0, 2, 0, 0, 8, 0, 0, 0, 8, 0, 1, 0, 8, 0, 0, 1,
			};

			m_torus.setDriver([](uint32_t phase) noexcept { return phase >= 4; });
			m_torus.draw(nullptr);

			m_torus.setDriver([](uint32_t) noexcept { return 1; });
		}

		// render the edges
		{
			m_unifs.u_nodeworld = math::unit();
			m_plane.setShaderType(e_line);
			m_plane.useEdge(true);
			m_plane.draw(nullptr);
			m_plane.useEdge(false);

			m_unifs.u_nodeworld = torus_matrix;
			m_torus.setShaderType(e_line);
			m_torus.useEdge(true);
			m_torus.draw(nullptr);
			m_torus.useEdge(false);
		}
		renderstate.flags.blend = false;
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto light_position = Vec3f(16.0, 10.0, 9.0);
		auto torus_center = Vec3f(0.0, 1.5, 0.0);
		auto torus_matrix = math::unit().trans(torus_center)
		                  * Mat4f(Quatf(esec / 17.0 * math::two_pi(), Vec3f(1, 2, 1)));

		auto light_viewport = Rectf(0, 0, 1, 1);
		auto light_proj_matrix = math::perspective(light_viewport, 10, 1, 100)
		                       * math::lookat(light_position, torus_center, ey());
		m_unifs.u_light_position = light_position;

		renderShadow(light_position, torus_matrix, light_proj_matrix);
		renderModel(torus_matrix, light_proj_matrix);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("031_sketch");
}  // namespace
}  // namespace spu::oglplus
