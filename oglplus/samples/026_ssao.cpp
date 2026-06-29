//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/obj_mesh.hpp>
#include <shapes/screen.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_data_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform vec3 u_light_position;                                                         \n"
    "uniform vec3 u_eye_position;                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "out vec3 g_light_dir;                                                                  \n"
    "out vec3 g_view_dir;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       g_light_dir = u_light_position - gl_Position.xyz;                               \n"
    "       g_view_dir = u_eye_position - gl_Position.xyz;                                  \n"
    "}                                                                                      \n"
};

const char *c_data_geom =  {
    "#version 330                                                                           \n"
    "layout (triangles) in;                                                                 \n"
    "layout (triangle_strip, max_vertices=3) out;                                           \n"
    "uniform mat4 u_worldscreen;                                                            \n"
    "in vec3 g_light_dir[3];                                                                \n"
    "in vec3 g_view_dir[3];                                                                 \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out vec3 f_view_dir;                                                                   \n"
    "out vec3 f_normal;                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_normal = normalize(                                                           \n"
    "               cross(                                                                  \n"
    "                       gl_in[1].gl_Position.xyz-                                       \n"
    "                       gl_in[0].gl_Position.xyz,                                       \n"
    "                       gl_in[2].gl_Position.xyz-                                       \n"
    "                       gl_in[0].gl_Position.xyz                                        \n"
    "               )                                                                       \n"
    "       );                                                                              \n"
    "       for (int v=0; v!=3; ++v)                                                        \n"
    "       {                                                                               \n"
    "               gl_Position = u_worldscreen * gl_in[v].gl_Position;                     \n"
    "               f_light_dir = g_light_dir[v];                                           \n"
    "               f_view_dir = g_view_dir[v];                                             \n"
    "               EmitVertex();                                                           \n"
    "       }                                                                               \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
};

const char *c_data_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_view_dir;                                                                    \n"
    "in vec3 f_normal;                                                                      \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 normal = normalize(f_normal);                                              \n"
    "       vec3 light_dir = normalize(f_light_dir);                                        \n"
    "       vec3 light_refl  = reflect(-light_dir, normal);                                 \n"
    "       vec3 view_dir = normalize(f_view_dir);                                          \n"
    "       final_color = vec4(                                                             \n"
    "               clamp(dot(normal, view_dir), 0, 1),                                     \n"
    "               clamp(dot(normal, light_dir),0, 1),                                     \n"
    "               clamp(dot(view_dir, light_refl), 0, 1),                                 \n"
    "               gl_FragCoord.z                                                          \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

const char *c_shape_vert =  {
    "#version 330                                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position;                                                       \n"
    "}                                                                                      \n"
};

const char *c_shape_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2DRect u_data_map;                                                      \n"
    "uniform float u_slider;                                                                \n"
    "uniform vec2 u_sample_offs[32];                                                        \n"
    "const int nsamples = 32;                                                               \n"
    "const float inv_nsam = 1.0 / nsamples;                                                 \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec4 fdata = texture(u_data_map, gl_FragCoord.xy);                              \n"
    "       float mask = ceil(fdata.w);                                                     \n"
    "       float ambi = 1;                                                                 \n"
    "       float view = fdata.x;                                                           \n"
    "       float diff = fdata.y;                                                           \n"
    "       float spec = pow(fdata.z, 8.0);                                                 \n"
    "       float sample_spread = mix(16, 8, fdata.w);                                      \n"
    "       float ssao = 0.0;                                                               \n"
    "       for (int s=0; s!=nsamples; ++s)                                                 \n"
    "       {                                                                               \n"
    "               vec2 sample_coord = gl_FragCoord.xy+sample_spread*u_sample_offs[s];     \n"
    "               vec4 sdata = texture(u_data_map, sample_coord);                         \n"
    "               float x = (fdata.w - sdata.w)*16.0;                                     \n"
    "               float y = view*x*exp(1-abs(x))*min(exp(x), 1.0);                        \n"
    "               ssao += max(1.0-y*2.0, 0.0);                                            \n"
    "       }                                                                               \n"
    "       ssao *= inv_nsam;                                                               \n"
    "       if (gl_FragCoord.x < u_slider) ssao = 1.0;                                      \n"
    "       ambi *= (0.00+0.30*ssao);                                                       \n"
    "       diff *= (0.05+0.40*ssao);                                                       \n"
    "       spec *= (0.05+0.60*ssao);                                                       \n"
    "       float sl = int(gl_FragCoord.x) == int(u_slider)?1:0;                            \n"
    "       vec3 u_color = vec3(0.9, 0.8, 0.3)*(ambi+diff)+vec3(spec);                      \n"
    "       vec3 bg_color = vec3(0.2);                                                      \n"
    "       vec3 sl_color = vec3(1.0);                                                      \n"
    "       final_color = mix(mix(bg_color, u_color, mask), sl_color, sl);                  \n"
    "}                                                                                      \n"
};

/* clang-format on */
class Uniforms {
public:
	Mat4f u_worldscreen;
	Mat4f u_nodeworld;
	Vec3f u_eye_position;
	Vec3f u_light_position;
	float u_slider;
	vec2f_t u_sample_offs[32];
	uint32_t u_data_map;

	Uniforms()
	{
		RandomGenerator<float> frand = {0.0, 1.0};

		for (auto &u_sample_off: u_sample_offs) {
			auto u = frand();
			auto v = frand();
			auto x = sqrtf(v) * cosf(2.0f * pi() * u);
			auto y = sqrtf(v) * sinf(2.0f * pi() * u);

			u_sample_off = {x, y};
		}

		m_attrs = {
		        {"u_worldscreen",    &u_worldscreen   },
		        {"u_nodeworld",      &u_nodeworld     },
		        {"u_eye_position",   &u_eye_position  },
		        {"u_light_position", &u_light_position},
		        {"u_slider",         &u_slider        },
		        {"u_sample_offs",    &u_sample_offs   },
		        {"u_data_map",       &u_data_map      },
		};
	}

	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class SSAOFrame : public SpuFrame {
public:
	void resize(Uniforms &unifs, float width, float height)
	{
		Rectf viewport{0, 0, width, height};
		Attrs attrs = {
		        {"viewport0",          viewport             },
                        {"color0.target",      GL_TEXTURE_RECTANGLE },
		        {"color0.iformat",     GL_RGBA32F           },
                        {"color0.auto_mipmap", 0                    },
		        {"depth.target",       GL_RENDERBUFFER      },
                        {"depth.iformat",      GL_DEPTH_COMPONENT32F},
		        {"depth.auto_mipmap",  0                    },
		};
		SpuFrame::init(attrs);
		unifs.u_data_map = getBuffer("color0").id();
	}
};

class App : public SpuPage {
public:
	Uniforms m_unifs;
	SSAOFrame m_frame;
	shapes::Array m_shapeArray;
	shapes::Array m_screenArray;

	App(const char *name) : SpuPage(name, true) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs data_shader_attrs = {
		        {"frag", c_data_frag},
                        {"geom", c_data_geom},
                        {"vert", c_data_vert}
                };

		Attrs shape_shader_attrs = {
		        {"vert", c_shape_vert},
		        {"frag", c_shape_frag},
		};

		m_shapeArray.initShader(data_shader_attrs, Attrs(m_unifs));
		m_screenArray.initShader(shape_shader_attrs, Attrs(m_unifs));

		File input_file("assets/models/stanford_dragon.obj", "rb");

		shapes::ObjMesh mesh(input_file, shapes::ObjMesh::LoadingOptions(false));

		m_shapeArray.initArray(mesh, {"position"});

		m_screenArray.initArray(shapes::Screen(), {"position"});

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.cull_face = GL_BACK;
		renderstate.flags.cull_face = true;

		Attrs frame_attrs = {
		        {"bgdepth", -1.0}
                };
		SpuPage::set(frame_attrs);  // no depth clear

		{
			m_frame.resize(m_unifs, int32_t(viewport(0).sx), int32_t(viewport(0).sy));
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto radius = 9.180799f;
		auto center = Vec3f(0.000000, 4.969950, 0.000000);
		auto bs_rad = radius * 1.2f;
		auto light = Mat4f::orbiting(center, esec, radius * 10.0, 0, 0, 0, 23, 0, -80, 31);
		auto worldview
		        = Mat4f::orbiting(center, esec, radius * 2.56, radius * 0.8, 23, 0, 19, 0, 80, 21);
		auto eye_target_dist = length(center - Vec3f(worldview.unitary_inverse().c[3]));
		auto viewscreen
		        = math::perspective(viewport(0), 45, eye_target_dist - bs_rad, eye_target_dist + bs_rad);
		auto nodeworld = math::unit().rot("Z", sin(esec / 21.0 * math::two_pi()) * 25)
		               * math::unit().trans({
		                       {0.0f, -bs_rad * 0.25f, 0.0f}
                });

		m_unifs.u_worldscreen = viewscreen * worldview;
		m_unifs.u_nodeworld = nodeworld;
		m_unifs.u_eye_position = worldview.unitary_inverse().c[3];
		m_unifs.u_light_position = light.unitary_inverse().c[3];

		// pass 1
		{
			auto &renderstate = SpuPage::getRenderstate();
			m_frame.begin();
			m_frame.clear();
			renderstate.flags.depth_test = true;
			renderstate.use();
			m_shapeArray.draw(nullptr);
			m_unifs.u_slider = (cos(esec / 11.0 * math::two_pi()) + 1.0) / 2.0 * viewport(0).sx;
			m_frame.end();
		}

		// pass 2
		{
			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.depth_test = false;
			renderstate.flags.cull_face = false;  // need check
			renderstate.use();
			m_screenArray.draw(nullptr);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("026_ssao");
}  // namespace
}  // namespace spu::oglplus
