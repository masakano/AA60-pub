//
// MeshArray :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/obj_mesh.hpp>

namespace spu::oglplus {
namespace {

/* clang-format on */
const char *c_depth_vert
        = {"#version 330                                                                           \n"
           "uniform mat4 u_viewsceen;                                                              \n"
           "uniform mat4 u_worldview;                                                              \n"
           "uniform mat4 u_nodeworld;                                                              \n"
           "in vec4 a_position;                                                                    \n"
           "void main()                                                                            \n"
           "{                                                                                      \n"
           "       gl_Position = u_viewsceen * u_worldview * u_nodeworld * a_position;             \n"
           "}                                                                                      \n"};

const char *c_depth_frag
        = {"#version 330                                                                           \n"
           "void main() { }                                                                        \n"};

const char *c_shape_vert
        = {"#version 330                                                                           \n"
           "uniform mat4 u_viewsceen;                                                              \n"
           "uniform mat4 u_worldview;                                                              \n"
           "uniform mat4 u_nodeworld;                                                              \n"
           "mat3 rot_matrix = mat3(u_nodeworld);                                                   \n"
           "const vec3 u_light_position = vec3(12.0, 11.0, 17.0);                                  \n"
           "in vec4 a_position;                                                                    \n"
           "in vec3 a_normal;                                                                      \n"
           "out vec3 f_light_dir;                                                                  \n"
           "out vec3 f_normal;                                                                     \n"
           "out vec3 f_color;                                                                      \n"
           "void main()                                                                            \n"
           "{                                                                                      \n"
           "       gl_Position = u_nodeworld * a_position;                                         \n"
           "       f_light_dir = u_light_position - gl_Position.xyz;                               \n"
           "       f_normal = rot_matrix * a_normal;                                               \n"
           "       f_color = normalize(                                                            \n"
           "               vec3(1, 1, 1) -                                                         \n"
           "               mix(f_normal, a_position.xyz, 0.5)                                      \n"
           "       );                                                                              \n"
           "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
           "}                                                                                      \n"};

const char *c_shape_frag
        = {"#version 330                                                                           \n"
           "uniform sampler2DRect u_depth_texture;                                                 \n"
           "in vec4 gl_FragCoord;                                                                  \n"
           "in vec3 f_light_dir;                                                                   \n"
           "in vec3 f_normal;                                                                      \n"
           "in vec3 f_color;                                                                       \n"
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
           "               float bz = texelFetch(u_depth_texture, texel_coord).x;                  \n"
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
           "       final_color = (ambi + diff + depth_diff) * f_color;                             \n"
           "}                                                                                      \n"};

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
	uint32_t u_depth_texture = 0;

	MeshArray()
	{
		Attrs shape_shader_attrs = {
		        {"frag", c_shape_frag},
		        {"vert", c_shape_vert},
		};

		Attrs depth_shader_attrs = {
		        {"frag", c_depth_frag},
		        {"vert", c_depth_vert},
		};

		Attrs unif_attrs = {
		        {"u_viewsceen",     &u_viewsceen    },
		        {"u_worldview",     &u_worldview    },
		        {"u_nodeworld",     &u_nodeworld    },
		        {"u_depth_texture", &u_depth_texture},
		};

		setMaxShaderType(2);
		initShader(shape_shader_attrs, unif_attrs, 0);
		initShader(depth_shader_attrs, unif_attrs, 1);
	}
};

class App : public SpuPage {
public:
	MeshArray m_meshArray;
	SpuTexture m_depthTexture;

	App(const char *name) : SpuPage(name, true) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		File input_file("assets/models/arrow_z.obj", "rb");

		shapes::ObjMesh mesh(input_file, shapes::ObjMesh::LoadingOptions(false).normals().materials());

		m_meshArray.initArray(mesh, {"position", "normal", "material"});

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;

		{
			if (m_meshArray.u_depth_texture != 0u) {
				spu_texture_delete(m_meshArray.u_depth_texture);
			}
			m_meshArray.u_depth_texture = 0;

			std::vector<float> buf;
			buf.resize(size_t(viewport(0).sx * viewport(0).sy),
			           0.75);  // debug for CopyTexImage

			Attrs tex_attrs = {
			        {"target",  GL_TEXTURE_RECTANGLE   },
			        {"iformat", GL_DEPTH_COMPONENT32F  },
			        {"width",   int32_t(viewport(0).sx)},
			        {"height",  int32_t(viewport(0).sy)},
			        {"data",    &buf[0]                },
			};
			m_depthTexture.init(tex_attrs);
			m_meshArray.u_depth_texture = m_depthTexture.id();
			m_meshArray.u_viewsceen = math::perspective(viewport(0), 60, 1, 10000);
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto worldview = Mat4f::orbiting(ezero(), esec, 2.8, 0, 0, 0, 19, 0, 90, 17);

		// pass 1
		{
			float bgdepth = 0.0;
			Attrs attrs = {
			        {"bgcolor0", Vec4f(-1)},
			        {"bgdepth",  bgdepth  },
			};

			SpuPage::set(attrs);
			SpuPage::clear();

			auto &renderstate = SpuPage::getRenderstate();
			renderstate.depth_func = GL_GREATER;
			renderstate.cull_face = GL_FRONT;
			renderstate.use();

			m_meshArray.u_worldview = worldview;
			m_meshArray.u_nodeworld = math::unit();
			m_meshArray.setShaderType(MeshArray::e_depth);
			m_meshArray.draw(nullptr);

			// printf("spu_texture_copy: %08x\n", m_meshArray.u_depth_texture);
			spu_texture_copy(m_meshArray.u_depth_texture, 0);
		}

		// pass 2
		{
			Vec4f color = {0.8, 0.8, 0.7, 0.0};
			float bgdepth = 1.0;
			Attrs attrs = {
			        {"bgcolor0", color  },
			        {"bgdepth",  bgdepth},
			};

			SpuPage::set(attrs);
			SpuPage::clear();

			auto &renderstate = SpuPage::getRenderstate();
			renderstate.depth_func = GL_LESS;
			renderstate.cull_face = GL_BACK;
			renderstate.use();

			m_meshArray.u_worldview = worldview;
			m_meshArray.u_nodeworld = math::unit();
			m_meshArray.setShaderType(MeshArray::e_shape);
			m_meshArray.draw(nullptr);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("021_translucent_arrow");
}  // namespace
}  // namespace spu::oglplus
