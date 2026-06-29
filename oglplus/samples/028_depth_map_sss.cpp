//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <cmath>
#include <shapes/obj_mesh.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_depth_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldscreen;                                                            \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_worldscreen * u_nodeworld * a_position;                         \n"
    "}                                                                                      \n"
};

const char *c_depth_frag =  {
    "#version 330                                                                           \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "}                                                                                      \n"
};

const char *c_shape_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_light_matrix;                                                           \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform vec3 u_light_position;                                                         \n"
    "uniform vec3 u_eye_position;                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "out vec4 g_position_depth;                                                             \n"
    "out vec3 g_light_dir;                                                                  \n"
    "out vec3 g_view_dir;                                                                   \n"

    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       g_position_depth = u_light_matrix * gl_Position;                                \n"
    "       g_light_dir = u_light_position - gl_Position.xyz;                               \n"
    "       g_view_dir = u_eye_position - gl_Position.xyz;                                  \n"
    "}                                                                                      \n"
};

const char *c_shape_geom =  {
    "#version 330                                                                             \n"
    "layout (triangles) in;                                                                   \n"
    "layout (triangle_strip, max_vertices=3) out;                                             \n"
    "uniform mat4 u_worldscreen;                                                              \n"
    "in vec4 g_position_depth[3];                                                             \n"
    "in vec3 g_light_dir[3];                                                                  \n"
    "in vec3 g_view_dir[3];                                                                   \n"
    "out vec4 f_position_depth;                                                               \n"
    "out vec3 f_light_dir;                                                                    \n"
    "out vec3 f_view_dir;                                                                     \n"
    "out vec3 f_normal;                                                                       \n"
    "void main()                                                                              \n"
    "{                                                                                        \n"
    "       f_normal = normalize(                                                             \n"
    "               cross(                                                                    \n"
    "                       gl_in[1].gl_Position.xyz-                                         \n"
    "                       gl_in[0].gl_Position.xyz,                                         \n"
    "                       gl_in[2].gl_Position.xyz-                                         \n"
    "                       gl_in[0].gl_Position.xyz                                          \n"
    "               )                                                                         \n"
    "       );                                                                                \n"
    "       for (int v=0; v!=3; ++v)                                                          \n"
    "       {                                                                                 \n"
    "               gl_Position = u_worldscreen * gl_in[v].gl_Position;                       \n"
    "               f_position_depth = g_position_depth[v];                                   \n"
    "               f_light_dir = g_light_dir[v];                                             \n"
    "               f_view_dir = g_view_dir[v];                                               \n"
    "               EmitVertex();                                                             \n"
    "       }                                                                                 \n"
    "       EndPrimitive();                                                                   \n"
    "}                                                                                        \n"
};

const char *c_shape_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2D u_depth_map;                                                         \n"
    "uniform vec2 u_depth_offs[32];                                                         \n"
    "const int depth_samples = 32;                                                          \n"
    "const float inv_depth_samples = 1.0 / depth_samples;                                   \n"
    "in vec4 gl_FragCoord;                                                                  \n"
    "in vec4 f_position_depth;                                                              \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_view_dir;                                                                    \n"
    "in vec3 f_normal;                                                                      \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float light_dist = f_position_depth.z/f_position_depth.w;                       \n"
    "       vec3 normal = normalize(f_normal);                                              \n"
    "       vec3 light_dir = normalize(f_light_dir);                                        \n"
    "       vec3 light_refl  = reflect(-light_dir, normal);                                 \n"
    "       vec3 view_dir = normalize(f_view_dir);                                          \n"
    "       float inv_w = 1.0/f_position_depth.w;                                           \n"
    "       float u_depth = 0.0;                                                            \n"
    "       for (int s=0; s!=depth_samples; ++s)                                            \n"
    "       {                                                                               \n"
    "               vec2 sample_coord = u_depth_offs[s]+f_position_depth.xy;                \n"
    "               sample_coord *= inv_w;                                                  \n"
    "               sample_coord *= 0.5;                                                    \n"
    "               sample_coord += 0.5;                                                    \n"
    "               float sample = texture(u_depth_map, sample_coord).r;                    \n"
    "               if (sample < 0.95)                                                      \n"
    "                       u_depth += sample;                                              \n"
    "               else u_depth += 0.5;                                                    \n"
    "       }                                                                               \n"
    "       u_depth *= inv_depth_samples;                                                   \n"
    "       float ambi = 0.15;                                                              \n"
    "       float bk_lt = (dot(-light_dir, view_dir)+3.0)*0.25;                             \n"
    "       float su_ss = pow(abs(u_depth-light_dist), 2.0)*bk_lt*1.2;                      \n"
    "       float shdw = min(pow(abs(u_depth-light_dist)*2.0, 8.0), 1.0);                   \n"
    "       float diff  = sqrt(max(dot(light_dir, normal)+0.1, 0.0))*0.4;                   \n"
    "       float spec  = pow(max(dot(light_refl, view_dir), 0.0), 64.0);                   \n"
    "       vec3 u_color = vec3(0.2, 0.9, 0.7);                                             \n"
    "       final_color = (ambi + shdw*diff + su_ss) * u_color;                             \n"
    "       final_color += shdw*spec * vec3(1.0, 1.0, 1.0);                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */
class Uniforms {
public:
	Mat4f u_worldscreen;
	Mat4f u_light_matrix;
	Mat4f u_nodeworld;
	Vec3f u_eye_position;
	Vec3f u_light_position;
	vec2f_t u_depth_offs[32];
	uint32_t u_depth_map;

	Uniforms()
	{
		RandomGenerator<float> frand = {0.0, 1.0};

		for (auto &u_depth_off: u_depth_offs) {
			auto u = frand();
			auto v = frand();

			auto x = sqrtf(v) * cosf(pi() * 2.0f * u);
			auto y = sqrtf(v) * sinf(pi() * 2.0f * u);

			u_depth_off = {x, y};
		}

		m_attrs = {
		        {"u_worldscreen",    &u_worldscreen   },
                        {"u_light_matrix",   &u_light_matrix  },
		        {"u_nodeworld",      &u_nodeworld     },
                        {"u_eye_position",   &u_eye_position  },
		        {"u_light_position", &u_light_position},
                        {"u_depth_offs",     &u_depth_offs    },
		        {"u_depth_map",      &u_depth_map     },
		};
	}
	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class DepthFrame : public SpuFrame {
public:
	DepthFrame(Uniforms &unifs, int32_t side)
	{
		auto viewport = Rectf(0, 0, side, side);
		Attrs attrs = {
		        {"viewport0",         viewport             },
		        {"color0.target",     GL_RENDERBUFFER      },
		        {"depth.target",      GL_TEXTURE_2D        },
		        {"depth.iformat",     GL_DEPTH_COMPONENT32F},
		        {"depth.min_filter",  GL_NEAREST           },
		        {"depth.mag_filter",  GL_NEAREST           },
		        {"depth.wrap_s",      GL_CLAMP_TO_EDGE     },
		        {"depth.wrap_t",      GL_CLAMP_TO_EDGE     },
		        {"depth.auto_mipmap", 0                    },
		};
		SpuFrame::init(attrs);
		unifs.u_depth_map = getBuffer("depth").id();
	}
};

class App : public SpuPage {
public:
	enum {
		e_shape = 0,
		e_depth,
	};

	Uniforms m_unifs;
	DepthFrame m_depthFrame;
	shapes::Array m_shapeArray;

	App(const char *name) : SpuPage(name, true, {0.2, 0.2, 0.2, 0.0}), m_depthFrame(m_unifs, 1024) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shape_shader_attrs = {
		        {"frag", c_shape_frag},
                        {"geom", c_shape_geom},
                        {"vert", c_shape_vert}
                };
		Attrs depth_shader_attrs = {
		        {"frag", c_depth_frag},
                        {"vert", c_depth_vert}
                };

		m_shapeArray.setMaxShaderType(2);  // 0:shape, 1:depth
		m_shapeArray.initShader(shape_shader_attrs, Attrs(m_unifs), 0);
		m_shapeArray.initShader(depth_shader_attrs, Attrs(m_unifs), 1);

		File input_file("assets/models/stanford_dragon.obj", "rb");
		shapes::ObjMesh mesh(input_file, shapes::ObjMesh::LoadingOptions(false));

		m_shapeArray.initArray(mesh, {"position"});

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;
		renderstate.cull_face = GL_BACK;
		renderstate.poly_offset = {1.0, 1.0};
	}

	void render() override
	{
		// ad-hock
		auto radius = 9.180799f;
		auto center = Vec3f(0.000000, 4.969950, 0.000000);
		auto bs_rad = radius * 1.2f;

		auto esec = getSeconds().current();
		auto light = Mat4f::orbiting(center, esec, radius * 10, 0, 0, 0, 23, 35, 50, 31);
		auto light_tgt_dist = length(center - Vec3f(light.unitary_inverse().c[3]));
		auto light_viewport = Rectf(0, 0, 1, 1);

		auto light_proj = math::perspective(
		        light_viewport, asin(bs_rad / light_tgt_dist) * 2 * 180.0 / pi(),
		        light_tgt_dist - bs_rad, light_tgt_dist + bs_rad);

		auto worldview = Mat4f::orbiting(ezero(), esec, radius * 2.0, 0, 0, 0, 17, 0, 80, 19);
		auto cam_tgt_dist = length(center - Vec3f(worldview.unitary_inverse().c[3]));
		auto cam_proj = math::perspective(
		        viewport(0), (asin(bs_rad / cam_tgt_dist) * 2) * 180 / pi(), cam_tgt_dist - bs_rad,
		        cam_tgt_dist + bs_rad);

		auto nodeworld = math::unit().rot("Z", sin(esec / 21.0 * math::two_pi()) * 25)
		               * math::unit().trans({0.0f, -bs_rad * 0.25f, 0.0f});

		m_unifs.u_nodeworld = nodeworld;
		m_unifs.u_light_matrix = light_proj * light;
		m_unifs.u_eye_position = worldview.unitary_inverse().c[3];
		m_unifs.u_light_position = light.unitary_inverse().c[3];

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.fill_offset = true;
		renderstate.use();
		m_depthFrame.begin();
		m_depthFrame.clear();

		m_unifs.u_worldscreen = light_proj * light;
		m_shapeArray.setShaderType(e_depth);
		m_shapeArray.draw(nullptr);

		m_depthFrame.end();

		renderstate.flags.fill_offset = false;
		renderstate.use();
		m_unifs.u_worldscreen = cam_proj * worldview;
		m_shapeArray.setShaderType(e_shape);
		m_shapeArray.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("028_depth_map_sss");
}  // namespace
}  // namespace spu::oglplus
