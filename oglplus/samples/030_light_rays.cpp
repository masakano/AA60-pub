//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/obj_mesh.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_common_vert =  {
    "#version 330                                                                           \n"
    "uniform vec3 u_light_position;                                                         \n"
    "uniform mat4 u_viewsceen, u_worldview, u_nodeworld,                                    \n"
    "u_light_matrix;                                                                        \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out vec4 f_position_shadow;                                                            \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_light_dir = normalize(u_light_position - gl_Position.xyz);                    \n"
    "       f_normal = normalize(mat3(u_nodeworld) * a_normal);                             \n"
    "       f_position_shadow = u_light_matrix * u_nodeworld * a_position;                  \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_shadow_frag =  {
    "#version 330                                                                           \n"
    "void main() { }                                                                        \n"
};

const char *c_mask_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2DShadow u_shadow_map;                                                  \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec4 f_position_shadow;                                                             \n"
    "out float frag_intensity;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 shadow_coord = (f_position_shadow.xyz/f_position_shadow.w)*0.5 + 0.5;      \n"
    "       float s = 0.0;                                                                  \n"
    "       if (                                                                            \n"
    "               shadow_coord.x >= 0.0 &&                                                \n"
    "               shadow_coord.x <= 1.0 &&                                                \n"
    "               shadow_coord.y >= 0.0 &&                                                \n"
    "               shadow_coord.y <= 1.0 &&                                                \n"
    "               shadow_coord.z <= 1.0                                                   \n"
    "       ) s = max(texture(u_shadow_map, shadow_coord), 0.05);                           \n"
    "       float l = max(dot(f_normal, f_light_dir)+0.1, 0.0);                             \n"
    "       frag_intensity = l * s;                                                         \n"
    "}                                                                                      \n"
};

const char *c_shape_frag =  {
    "#version 330                                                                           \n"
    "const vec3 light_color = vec3(0.6, 0.6, 1.0);                                          \n"
    "const vec3 up = normalize(vec3(0.1, 1.0, 0.1));                                        \n"
    "uniform vec3 u_light_screen_pos;                                                       \n"
    "uniform vec2 u_screen_size;                                                            \n"
    "uniform sampler2DRect u_light_map;                                                     \n"
    "uniform sampler2DShadow u_shadow_map;                                                  \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec4 f_position_shadow;                                                             \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 shadow_coord = (f_position_shadow.xyz/f_position_shadow.w)*0.5 + 0.5;      \n"
    "       float s = 0.0;                                                                  \n"
    "       if (                                                                            \n"
    "               shadow_coord.x >= 0.0 &&                                                \n"
    "               shadow_coord.x <= 1.0 &&                                                \n"
    "               shadow_coord.y >= 0.0 &&                                                \n"
    "               shadow_coord.y <= 1.0 &&                                                \n"
    "               shadow_coord.z <= 1.0                                                   \n"
    "       ) s = texture(u_shadow_map, shadow_coord);                                      \n"
    "       float a = 0.1*(max(dot(f_normal, up)+0.1, 0.0)+0.1);                            \n"
    "       float d = max(dot(f_normal, f_light_dir)+0.1, 0.0)+a;                           \n"

    "       vec2 lmcoord = gl_FragCoord.xy;                                                 \n"
    "       vec2 lpos = (u_light_screen_pos.xy*0.5+0.5)*u_screen_size;                      \n"
    "       vec2 ray = lmcoord - lpos;                                                      \n"
    "       float len = length(ray);                                                        \n"
    "       int nsampl = int(max(abs(ray.x), abs(ray.y)))+1;                                \n"
    "       vec2 ray_step = ray / nsampl;                                                   \n"
    "       float r = texture(u_light_map, lmcoord).r;                                      \n"
    "       nsampl = min(nsampl, int(min(u_screen_size.x, u_screen_size.y)*0.25));          \n"
    "       for (int s=0; s!=nsampl;++s)                                                    \n"
    "       {                                                                               \n"
    "               r += texture(u_light_map, lpos+ray_step*s).r;                           \n"
    "       }                                                                               \n"
    "       r /= nsampl;                                                                    \n"
    "       r = min(r, 1.0);                                                                \n"
    "       final_color = light_color * (mix(a, d, s) + r);                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */
class Uniforms {
public:
	Mat4f u_rotation;  // set in driver
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Mat4f u_light_matrix;
	Vec3f u_light_position = Vec3f(0, 0, -100);
	Vec3f u_light_screen_pos;
	Vec2f u_screen_size = Vec2f(0.0);
	uint32_t u_shadow_map = 0;
	uint32_t u_light_map = 0;

	Uniforms()
	{
		m_attrs = {
		        {"u_viewsceen",        &u_viewsceen       },
                        {"u_worldview",        &u_worldview       },
		        {"u_nodeworld",        &u_nodeworld       },
                        {"u_light_matrix",     &u_light_matrix    },
		        {"u_light_position",   &u_light_position  },
                        {"u_light_screen_pos", &u_light_screen_pos},
		        {"u_screen_size",      &u_screen_size     },
                        {"u_shadow_map",       &u_shadow_map      },
		        {"u_light_map",        &u_light_map       },
		};
	}

	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class MeshArray : public shapes::Array {
public:
	Uniforms &m_unifs;
	uint32_t m_fanIndex;

	explicit MeshArray(Uniforms &unifs) : m_unifs(unifs) {}

	void initArray()
	{
		File file("assets/models/large_fan.obj", "rb");
		shapes::ObjMesh mesh_shape(file, shapes::ObjMesh::LoadingOptions(false).normals());

		m_fanIndex = mesh_shape.getMeshIndex("Fan");
		Array::initArray(mesh_shape, {"position", "normal"});
		setDriver();
	}

private:
	void setDriver()
	{
		m_driver = [&](uint32_t phase) {
			if (phase == m_fanIndex) {
				m_unifs.u_nodeworld = m_unifs.u_rotation;
				useShader();
				return 1;
			}
			m_unifs.u_nodeworld = Mat4f();
			useShader();
			return 2;
		};
	}
};

class App : public SpuPage {
public:
	enum {
		e_shape = 0,
		e_mask,
		e_shadow,
	};

	Uniforms m_unifs;
	SpuFrame m_lightFrame;

	void renderShadowMap(int32_t size)
	{
		auto lt_viewport = Rectf(0, 0, 1, 1);
		auto lt_proj = math::perspective(lt_viewport, 12, 85.0, 110.0);
		auto light = math::lookat(m_unifs.u_light_position, ezero(), ey());

		m_unifs.u_light_matrix = lt_proj * light;

		SpuFrame frame;
		{
			auto viewport = Rectf(0, 0, size, size);
			Attrs attrs = {
			        {"viewport0",          viewport                 },
			        {"color0.target",      GL_RENDERBUFFER          },
			        {"depth.target",       GL_TEXTURE_2D            },
			        {"depth.iformat",      GL_DEPTH_COMPONENT32F    },
			        {"depth.min_filter",   GL_LINEAR                },
			        {"depth.mag_filter",   GL_LINEAR                },
			        {"depth.wrap_s",       GL_CLAMP_TO_BORDER       },
			        {"depth.wrap_t",       GL_CLAMP_TO_BORDER       },
			        {"depth.compare_mode", GL_COMPARE_REF_TO_TEXTURE},
			        {"depth.auto_mipmap",  0                        },
			        {"bgcolor0",           Vec4f(-1)                },
			};
			frame.init(attrs);
			m_unifs.u_shadow_map = frame.getBuffer("depth").id();
		}
		frame.begin();
		{
			SpuScopedRenderstate renderstate(true);

			// setup the matrices
			m_unifs.u_viewsceen = lt_proj;
			m_unifs.u_worldview = light;

			// draw the meshes
			renderstate.poly_offset = {1.0, 1.0};
			renderstate.flags.fill_offset = true;
			renderstate.flags.depth_test = true;
			renderstate.use();
			frame.clear();

			m_meshArray.setShaderType(e_shadow);
			m_meshArray.draw(nullptr);
		}
		frame.end();
	}

	void resizeLightMask(float width, float height)
	{
		auto viewport = Rectf(0, 0, width, height);
		Attrs attrs = {
		        {"viewport0",          viewport             },
                        {"color0.target",      GL_TEXTURE_RECTANGLE },
		        {"color0.iformat",     GL_R8                },
                        {"color0.auto_mipmap", 0                    },
		        {"depth.target",       GL_RENDERBUFFER      },
                        {"depth.iformat",      GL_DEPTH_COMPONENT32F},
		};
		m_lightFrame.init(attrs);
		m_unifs.u_light_map = m_lightFrame.getBuffer("color0").id();
	}

	Mat4f m_viewscreen;
	MeshArray m_meshArray;

	App(const char *name) : SpuPage(name, true), m_meshArray(m_unifs) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shape_shader_attrs = {
		        {"frag", c_shape_frag },
		        {"vert", c_common_vert},
		};
		Attrs mask_shader_attrs = {
		        {"frag", c_mask_frag  },
		        {"vert", c_common_vert},
		};
		Attrs shadow_shader_attrs = {
		        {"frag", c_shadow_frag},
		        {"vert", c_common_vert},
		};

		m_meshArray.setMaxShaderType(3);  // 0:shape 1:mask 2:shadow

		m_meshArray.initShader(shape_shader_attrs, Attrs(m_unifs), 0);
		m_meshArray.initShader(mask_shader_attrs, Attrs(m_unifs), 1);
		m_meshArray.initShader(shadow_shader_attrs, Attrs(m_unifs), 2);
		m_meshArray.initArray();

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderShadowMap(512);

		{
			m_viewscreen = math::perspective(viewport(0), 70, 1, 200);
			m_unifs.u_screen_size = Vec2f(viewport(0).sx, viewport(0).sy);
			resizeLightMask(int32_t(viewport(0).sx), int32_t(viewport(0).sy));
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto bgcolor = Vec4f(1.0, 1.0, 1.0, 0.0);

		auto worldview0 = Mat4f().rot(
		        "z",
		        radians(sin(esec / 11.0 * math::two_pi()) * 7 + sin(esec / 13.0 * math::two_pi()) * 5));
		auto worldview1 = Mat4f::orbiting(
		        ezero(), esec, 40, 0, 0,
		        radians(sin(esec / 11.0 * math::two_pi()) * 10 + cos(esec / 19.0 * math::two_pi()) * 10
		                - 90),
		        0, sin(esec / 17.0 * math::two_pi()) * 10 + sin(esec / 13.0 * math::two_pi()) * 10, 0,
		        0);
		auto worldview = worldview0 * worldview1;

		m_unifs.u_rotation = math::unit().rot("z", -esec / 7.0 * math::two_pi());

		// render the light mask
		m_lightFrame.begin();
		m_lightFrame.set("bgcolor0", bgcolor);
		m_lightFrame.clear();

		m_unifs.u_worldview = worldview;
		m_unifs.u_viewsceen = m_viewscreen;
		m_unifs.u_nodeworld = math::unit();

		m_meshArray.setShaderType(e_mask);
		m_meshArray.draw(nullptr);

		m_lightFrame.end();

		// render the final image
		Attrs frame_attrs = {
		        {"bgcolor0", bgcolor}
                };
		SpuPage::set(frame_attrs);

		auto lsp = m_viewscreen * worldview * Vec4f(m_unifs.u_light_position, 1.0);

		m_unifs.u_light_screen_pos = lsp / lsp.w;
		m_unifs.u_worldview = worldview;
		m_unifs.u_nodeworld = math::unit();
		m_unifs.u_viewsceen = m_viewscreen;

		m_meshArray.setShaderType(e_shape);
		m_meshArray.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("030_light_rays");
}  // namespace
}  // namespace spu::oglplus
