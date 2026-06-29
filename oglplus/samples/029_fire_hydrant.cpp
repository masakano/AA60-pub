//
// MeshArray :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/blender_mesh.hpp>
#include <math/curve.hpp>
#include "replace_text.h"

namespace spu::oglplus {

namespace {

/* clang-format off */
const char *c_shadow_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_light_proj_matrix;                                                      \n"
    "uniform mat4 u_light_matrix;                                                           \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "mat4 u_matrix =u_light_proj_matrix*u_light_matrix*u_nodeworld;                         \n"
    "in vec4 a_position;\n" // location must be 0! why??
    "in vec3 a_normal;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_matrix * a_position;                                            \n"
    "}                                                                                      \n"
};

const char *c_shadow_frag =  {
    "#version 330                                                                           \n"
    "void main() {}                                                                         \n"
};

const char *c_shape_vert =  {
    "#version 330                                                                           \n"
    "uniform vec3 u_light_position;                                                         \n"
    "uniform vec3 u_eye_position;                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "mat3 model_rot_matrix = mat3(u_nodeworld);                                             \n"
    "uniform mat4 u_light_proj_matrix;                                                      \n"
    "uniform mat4 u_light_matrix;                                                           \n"
    "mat4 lt_mat = u_light_proj_matrix*u_light_matrix*u_nodeworld;                          \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "in vec3 a_tangent;                                                                     \n"
    "in vec3 a_bitangent;                                                                   \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec3 f_normal, f_tangent, f_bitangent;                                             \n"
    "out vec3 f_light_dir, f_view_dir;                                                      \n"
    "out vec2 f_texcoord;                                                                   \n"
    "out vec4 f_position_shadow;                                                            \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_light_dir = u_light_position - gl_Position.xyz;                               \n"
    "       f_view_dir = u_eye_position - gl_Position.xyz;                                  \n"
    "       gl_Position = u_viewsceen*u_worldview*gl_Position;                              \n"
    "       f_normal = model_rot_matrix* a_normal;                                          \n"
    "       f_tangent = model_rot_matrix*a_tangent;                                         \n"
    "       f_bitangent = model_rot_matrix*a_bitangent;                                     \n"
    "       f_texcoord = a_texcoord;                                                        \n"
    "       f_position_shadow = lt_mat * a_position;                                        \n"
    "}                                                                                      \n"
};

const char *c_shape_frag =  {
    "#version 330                                                                                      \n"
    //"const int c_shadow_samples = 32;                                                                    \n"
    "const float inv_c_shadow_samples = 1.0 / c_shadow_samples;                                            \n"
    "const vec3 light_color = vec3(1.0, 1.0, 1.0);                                                     \n"
    "uniform sampler2D u_normal_map;                                                                   \n"
    "uniform sampler2D u_lighting_map;                                                                 \n"
    "uniform sampler2D u_color_map;                                                                    \n"
    "uniform sampler2DShadow u_shadow_map;                                                             \n"
    "uniform vec3 u_shadow_offs[c_shadow_samples];                                                       \n"
    "in vec3 f_normal, f_tangent, f_bitangent;                                                         \n"
    "in vec3 f_light_dir, f_view_dir;                                                                  \n"
    "in vec2 f_texcoord;                                                                               \n"
    "in vec4 f_position_shadow;                                                                        \n"
    "out vec3 final_color;                                                                             \n"
    "void main()                                                                                       \n"
    "{                                                                                                 \n"
    "       vec3 n_map = normalize(texture(u_normal_map, f_texcoord).rgb);                             \n"
    "       vec3 nadj = vec3(n_map.x*2.0-1.0, n_map.y*2.0-1.0, n_map.z);                               \n"
    "       vec3 lmap = texture(u_lighting_map, f_texcoord).rgb;                                       \n"
    "       vec3 u_color = texture(u_color_map, f_texcoord).rgb;                                       \n"
    "       float inv_w = 1.0/f_position_shadow.w;                                                     \n"
    "       float f = f_position_shadow.w / 128.0;                                                     \n"
    "       float shadow = 0.0;                                                                        \n"
    "       for (int s=0; s!=c_shadow_samples; ++s)                                                    \n"
    "       {                                                                                          \n"
    "               vec3 sample_coord = ((u_shadow_offs[s]*f+f_position_shadow.xyz)*inv_w) * 0.5 + 0.5;\n"
    "               if (                                                                               \n"
    "                       sample_coord.x >= 0.0 &&                                                   \n"
    "                       sample_coord.x <= 1.0 &&                                                   \n"
    "                       sample_coord.y >= 0.0 &&                                                   \n"
    "                       sample_coord.y <= 1.0 &&                                                   \n"
    "                       sample_coord.z <= 1.0                                                      \n"
    "               ) shadow += texture(u_shadow_map, sample_coord)*inv_c_shadow_samples;              \n"
    "               else shadow += inv_c_shadow_samples;                                               \n"
    "       }                                                                                          \n"
    "       vec3 light_dir = normalize(f_light_dir);                                                   \n"
    "       vec3 view_dir = normalize(f_view_dir);                                                     \n"
    "       vec3 normal = normalize(f_normal);                                                         \n"
    "       vec3 tangent = normalize(f_tangent);                                                       \n"
    "       vec3 bitangent = normalize(f_bitangent);                                                   \n"
    "       vec3 frag_normal = f_tangent*nadj.x + bitangent*nadj.y + normal*nadj.z;                    \n"
    "       vec3 light_refl = reflect(-light_dir, frag_normal);                                        \n"
    "       float ambi = sqrt(max(dot(frag_normal, vec3(0.0, 1.0, 0.0))+0.2, 0.0))*0.6 + 0.4;          \n"
    "       float diff = sqrt(max(dot(frag_normal, light_dir), 0.0))*shadow;                           \n"
    "       float spec = pow(max(dot(light_refl, view_dir)+0.15, 0.0), 0.5+1.5*lmap.b)*shadow;         \n"
    "       final_color =                                                                              \n"
    "                       (lmap.r*0.5)*ambi*u_color +                                                \n"
    "                       (lmap.g*0.8)*diff*u_color +                                                \n"
    "                       (lmap.b*0.4)*spec*light_color;                                             \n"
    //"       final_color = vec3(shadow);                                                     \n"
    "}                                                                                      \n"
};

/* clang-format on */
class MeshArray : public shapes::Array {
public:
	enum {
		e_shape = 0,
		e_shadow,
	};
	Vec3f u_light_position;
	Vec3f u_eye_position;
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;

	Mat4f u_light_proj_matrix;
	Mat4f u_light_matrix;

	uint32_t u_normal_map;
	uint32_t u_lighting_map;
	uint32_t u_color_map;
	uint32_t u_shadow_map;

	static constexpr auto c_shadow_samples = 32;
	const Attrs replace_attrs = {
		{"c_shadow_samples", c_shadow_samples},
	};
	Vec3f u_shadow_offs[c_shadow_samples];

	MeshArray()
	{
		RandomGenerator<float> frand = {0.0, 1.0};

		Attrs shape_shader_attrs = {
		        {"vert", c_shape_vert},
		        {"frag", replaceText(c_shape_frag, replace_attrs)}
		};

		Attrs shadow_shader_attrs = {
		        {"vert", c_shadow_vert},
		        {"frag", c_shadow_frag},
		};

		Attrs unif_attrs = {
		        {"u_light_position",    &u_light_position   },
		        {"u_eye_position",      &u_eye_position     },
		        {"u_viewsceen",         &u_viewsceen        },
		        {"u_worldview",         &u_worldview        },
		        {"u_light_matrix",      &u_light_matrix     },
		        {"u_nodeworld",         &u_nodeworld        },
		        {"u_light_proj_matrix", &u_light_proj_matrix},
		        {"u_normal_map",        &u_normal_map       },
		        {"u_lighting_map",      &u_lighting_map     },
		        {"u_color_map",         &u_color_map        },
		        {"u_shadow_map",        &u_shadow_map       },
		        {"u_shadow_offs",       u_shadow_offs       },
		};
		setMaxShaderType(2);  // 0:shape, 1:shadow
		initShader(shape_shader_attrs, unif_attrs, 0);
		initShader(shadow_shader_attrs, unif_attrs, 1);

		for (auto &u_shadow_off: u_shadow_offs) {
			auto u = frand();
			auto v = frand();

			u_shadow_off
			        = {sqrtf(v) * cosf(2.0f * pi() * u), sqrtf(v) * sinf(2.0f * pi() * u), 0.0f};
		}
	}
};

class App : public SpuPage {
public:
	static constexpr auto c_ntex = 3u;
	static constexpr auto c_smap_side = 1024;

	CubicBezierLoop<Vec3f, double> m_lightPath;
	CubicBezierLoop<Vec3f, double> m_cameraPath;
	CubicBezierLoop<Vec3f, double> m_targetPath;

	SpuFrame m_frame;
	MeshArray m_meshArray;

	App(const char *name) : SpuPage(name, true, {0.6, 0.6, 0.5, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_lightPath.init({
		        {-15.0, 10.0, -12.5},
		        {-15.0, 14.0, 0.5  },
		        {-12.0, 20.0, 17.0 },
		        {0.0,   14.0, 12.0 },
		        {11.0,  11.0, 11.5 },
		        {7.2,   11.0, -10.0}
                });

		m_cameraPath.init({
		        {6.2,  12.0, -7.0},
		        {4.0,  15.0, 0.0 },
		        {5.0,  12.0, 6.5 },
		        {-5.0, 13.0, 7.0 },
		        {-6.0, 11.0, 0.0 },
		        {-4.0, 14.0, -6.5}
                });

		m_targetPath.init({
		        {3.2,  2.0, -0.1},
                        {-3.2, 3.0, -0.3},
                        {0.3,  1.0, 1.3 }
                });

		File input_file("assets/models/hydrant.blend", "rb");
		imports::BlendFile blend_file(input_file);

		std::array<const char *, 1> model_list = {"hydrant"};
		auto shape = shapes::BlenderMesh(blend_file, model_list);

		std::vector<const char *> attrib_names
		        = {"position", "normal", "tangent", "bitangent", "texcoord"};
		m_meshArray.initArray(shape, attrib_names);

		const char *tex_filenames[3] = {"hydrant_normal", "hydrant_light", "hydrant_color"};

		uint32_t tex_id[3];

		for (auto t = 0u; t < c_ntex; t++) {
			std::string path = std::string("assets/models/") + tex_filenames[t] + ".png";
			tex_id[t] = spu_inventory_new("texture", path.c_str(), Attrs());
		}

		{
			auto viewport = Rectf(0, 0, c_smap_side, c_smap_side);
			Attrs attrs = {
			        {"viewport0",          viewport                 },
			        {"color0.target",      GL_RENDERBUFFER          },
			        {"depth.target",       GL_TEXTURE_2D            },
			        {"depth.min_filter",   GL_LINEAR                },
			        {"depth.mag_filter",   GL_LINEAR                },
			        {"depth.wrap_s",       GL_CLAMP_TO_EDGE         },
			        {"depth.wrap_t",       GL_CLAMP_TO_EDGE         },
			        {"depth.compare_mode", GL_COMPARE_REF_TO_TEXTURE},
			        {"depth.iformat",      GL_DEPTH_COMPONENT32F    },
			        {"bgcolor0",           Vec4f(-1)                }, // no clear
			};
			m_frame = SpuFrame(attrs);
		}

		m_meshArray.u_normal_map = tex_id[0];
		m_meshArray.u_lighting_map = tex_id[1];
		m_meshArray.u_color_map = tex_id[2];
		m_meshArray.u_shadow_map = m_frame.getBuffer("depth").id();

		auto light_viewport = Rectf(0, 0, 1, 1);
		auto light_proj_mat = math::perspective(light_viewport, 40, 1.0, 100.0);
		m_meshArray.u_light_proj_matrix = light_proj_mat;

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.poly_offset = {1.0, 1.0};
	}

	void render() override
	{
		m_meshArray.u_viewsceen = math::perspective(viewport(0), 75, 1, 40);

		auto esec = getSeconds().current();
		auto light_pos = m_lightPath.position(esec / 9.0);
		auto light_matrix = math::lookat(light_pos, Vec3f(0, 2, 0), ey());
		auto worldview = math::lookat(
		        m_cameraPath.position(esec / 19.0), m_targetPath.position(esec / 11.0), ey());

		auto model1 = math::unit().trans({+3.0, 0.0, 0.0}) * math::unit().rot("Y", 45.0);
		auto model2 = math::unit().trans({-3.0, 0.0, 0.0}) * math::unit().rot("Y", -175.0);

		// render the shadow map
		m_frame.begin();
		m_frame.clear();  // clear depth only

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.fill_offset = true;
		renderstate.use();

		m_meshArray.setShaderType(MeshArray::e_shadow);
		m_meshArray.u_light_matrix = light_matrix;
		m_meshArray.u_nodeworld = model1;
		m_meshArray.draw(nullptr);
		m_meshArray.u_nodeworld = model2;
		m_meshArray.draw(nullptr);
		m_frame.end();

		// render the frame
		renderstate.use();

		renderstate.flags.fill_offset = false;
		renderstate.use();

		m_meshArray.setShaderType(MeshArray::e_shape);
		m_meshArray.u_light_position = light_pos;
		m_meshArray.u_eye_position = worldview.unitary_inverse().c[3];
		m_meshArray.u_worldview = worldview;
		m_meshArray.u_light_matrix = light_matrix;

		m_meshArray.u_nodeworld = model1;
		m_meshArray.draw(nullptr);

		m_meshArray.u_nodeworld = model2;
		m_meshArray.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("029_fire_hydrant");
}  // namespace
}  // namespace spu::oglplus
