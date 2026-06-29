//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/obj_mesh.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 f_normal;                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_normal = a_normal;                                                            \n"
    "       gl_Position = u_viewsceen * u_worldview * a_position;                           \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_normal;                                                                      \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 c = normalize(vec3(1, 1, 1) - f_normal);                                   \n"
    "       final_color = c;                                                                \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	Vec4f m_meshBs;
	shapes::Array m_array;
	Mat4f u_viewsceen;
	Mat4f u_worldview;

	App(const char *name) : SpuPage(name, true, {0.8, 0.8, 0.7, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};

		Attrs unif_attrs = {
		        {"u_viewsceen", &u_viewsceen},
		        {"u_worldview", &u_worldview},
		};
		m_array.initShader(shader_attrs, unif_attrs);

		File input_file("assets/models/suzanne.obj", "rb");
		shapes::ObjMesh load_mesh(input_file);
		Vec4f oglplus_mesh_bs;
		load_mesh.boundingSphere(oglplus_mesh_bs);
		m_meshBs = oglplus_mesh_bs;
		m_array.initArray(load_mesh, {"position", "normal"});
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto radius = m_meshBs.w * 1.4 + 1.0;
		u_viewsceen = math::perspective(viewport(0), 70, 1, 20);
		u_worldview = Mat4f::orbiting(m_meshBs, esec, radius, 0, 0, 0, 13.0, 0, 90, 17);
		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("015_obj_mesh");
}  // namespace
}  // namespace spu::oglplus
