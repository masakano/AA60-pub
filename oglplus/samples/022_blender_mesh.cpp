//
// MeshArray :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/blender_mesh.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_depth_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_viewsceen * u_worldview * u_nodeworld * a_position;             \n"
    "}                                                                                      \n"
};

const char *c_depth_frag =  {
    "#version 330                                                                           \n"
    "void main() { }                                                                        \n"
};

const char *c_shape_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "mat3 rot_matrix = mat3(u_nodeworld);                                                   \n"
    "const vec3 u_light_position = vec3(12.0, 11.0, 17.0);                                  \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "in float a_material;                                                                   \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out vec3 f_normal;                                                                     \n"
    "flat out int f_material;                                                               \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_light_dir = u_light_position - gl_Position.xyz;                               \n"
    "       f_normal = rot_matrix * a_normal;                                               \n"
    "       f_material = int(a_material);                                                   \n"
    "       gl_Position = u_viewsceen * u_worldview *                                       \n"
    "gl_Position;                                                                           \n"
    "}                                                                                      \n"
};

const char *c_shape_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2DRect u_depth_tex;                                                     \n"
    "uniform vec4 u_colors[3];                                                              \n"
    "in vec4 gl_FragCoord;                                                                  \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_normal;                                                                      \n"
    "flat in int f_material;                                                                \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float w = gl_FragCoord.w;                                                       \n"
    "       float sum_bz = 0.0;                                                             \n"
    "       const int nd = 2;                                                               \n"
    "       float ns = 0.0;                                                                 \n"
    "       for (int dy=-nd; dy!=(nd+1); ++dy)                                              \n"
    "       for (int dx=-nd; dx!=(nd+1); ++dx)                                              \n"
    "       {                                                                               \n"
    "               ivec2 texel_coord = ivec2(gl_FragCoord.xy)+ivec2(dx, dy);               \n"
    "               float bz = texelFetch(u_depth_tex, texel_coord).x;                      \n"
    "               if (bz < 1.0)                                                           \n"
    "               {                                                                       \n"
    "                       sum_bz += bz;                                                   \n"
    "                       ns += 1.0;                                                      \n"
    "               }                                                                       \n"
    "       }                                                                               \n"
    "       float depth_diff = (sum_bz/ns)/w - gl_FragCoord.z/w;                            \n"
    "                                                                                       \n"
    "       depth_diff =  clamp(pow(mix(1.0, 0.0, depth_diff*2.0), 3.0), 0.0, 1.0);         \n"
    "       vec3 normal = normalize(f_normal);                                              \n"
    "       vec3 light_dir = normalize(f_light_dir);                                        \n"
    "       float ambi = 0.2;                                                               \n"
    "       float diff  = 0.4*max(dot(light_dir, normal)+0.1, 0.0);                         \n"
    "       final_color = (ambi + diff + depth_diff) * u_colors[f_material].rgb;            \n"
    "}                                                                                      \n"
};

/* clang-format on */

class MeshArray : public shapes::Array {
public:
	enum {
		e_shape = 0,
		e_depth = 1,
	};

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_colors[3];
	uint32_t u_depth_tex = 0u;

	MeshArray()
	{
		Attrs shape_shader_attrs = {
		        {"vert", c_shape_vert},
		        {"frag", c_shape_frag},
		};

		Attrs depth_shader_attrs = {
		        {"frag", c_depth_frag},
		        {"vert", c_depth_vert},
		};

		Attrs unif_attrs = {
		        {"u_viewsceen", &u_viewsceen},
                        {"u_worldview", &u_worldview},
		        {"u_nodeworld", &u_nodeworld},
                        {"u_colors",    &u_colors[0]},
		        {"u_depth_tex", &u_depth_tex},
		};

		setMaxShaderType(2);
		initShader(shape_shader_attrs, unif_attrs, 0);
		initShader(depth_shader_attrs, unif_attrs, 1);
	}
};

class App : public SpuPage {
public:
	MeshArray m_meshArray;
	;
	SpuFrame m_frame;

	shapes::BlenderMesh loadMonkeys()
	{
		File input_file("assets/models/monkeys.blend", "rb");

		printf("input_file: %ld bytes\n", input_file.size());

		imports::BlendFile blend_file(input_file);
		std::array<const char *, 3> model_list = {"Monkey1", "Monkey2", "Monkey3"};

		return {blend_file, model_list,
		        shapes::BlenderMesh::LoadingOptions().nothing().normals().materials()};
	}

	App(const char *name) : SpuPage(name, true, {0.8, 0.8, 0.7, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		auto monkey_shape = loadMonkeys();
		m_meshArray.initArray(monkey_shape, {"position", "normal", "material"});

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;

		{
			auto bgdepth = 1.0f;
			Attrs attrs = {
			        {"viewport0",     viewport(0)          },
			        {"color0.target", GL_RENDERBUFFER      },
			        {"depth.target",  GL_TEXTURE_RECTANGLE },
			        {"depth.iformat", GL_DEPTH_COMPONENT32F},
			        {"bgdepth",       bgdepth              }, // not pointer!
			};
			m_frame.init(attrs);
			m_meshArray.u_depth_tex = m_frame.getBuffer("depth").id();
			m_meshArray.u_viewsceen = math::perspective(viewport(0), 60, 1, 20);
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();
		m_meshArray.u_worldview = Mat4f::orbiting(ezero(), esec, 7.0, 0, 0, 19, 0, 0, 90, 17);

		const Vec3f colors[3][3] = {
		        {{1.0, 0.3, 0.3}, {1.0, 0.5, 0.5}, {1.0, 0.7, 0.7}},
		        {{0.3, 1.0, 0.3}, {0.5, 1.0, 0.5}, {0.7, 1.0, 0.7}},
		        {{0.3, 0.3, 1.0}, {0.5, 0.5, 1.0}, {0.7, 0.7, 1.0}}
                };

		const Mat4f rot_matrices[3] = {
		        Mat4f(Quatf(radians(19 * esec), Vec3f(1, 1, 1))),
		        Mat4f(Quatf(radians(31 * esec), Vec3f(1, 1, 1))),
		        Mat4f(Quatf(radians(47 * esec), Vec3f(1, 1, 1))),
		};

		const Mat4f pos_matrices[3] = {
		        math::unit().rot("Y", 120.0) * math::unit().trans({0.0, 0.0, 2.0}),
		        math::unit().rot("Y", 240.0) * math::unit().trans({0.0, 0.0, 2.0}),
		        math::unit().trans({0.0, 0.0, 2.0}),
		};

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.cull_face = GL_FRONT;
		renderstate.use();
		m_frame.begin();
		m_frame.clear();

		m_meshArray.setShaderType(MeshArray::e_depth);
		for (auto i = 0; i < 3; i++) {
			memcpy(m_meshArray.u_colors, colors[i], sizeof(colors[i]));
			m_meshArray.u_nodeworld = pos_matrices[i] * rot_matrices[i];
			m_meshArray.draw(nullptr);
		}
		m_frame.end();

		renderstate.cull_face = GL_BACK;
		renderstate.use();
		m_meshArray.setShaderType(MeshArray::e_shape);
		for (auto i = 0; i < 3; i++) {
			memcpy(m_meshArray.u_colors, colors[i], sizeof(colors[i]));
			m_meshArray.u_nodeworld = pos_matrices[i] * rot_matrices[i];
			m_meshArray.draw(nullptr);
		}
	}
};
static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("022_blender_mesh");
}  // namespace
}  // namespace spu::oglplus
