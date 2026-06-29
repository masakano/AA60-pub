//
// PointShader :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <cstdlib>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform vec3 u_color1;                                                                 \n"
    "uniform vec3 u_color2;                                                                 \n"
    "uniform float u_status;                                                                \n"
    "in vec4 a_position1, a_position2;                                                      \n"
    "in float a_radiance1, a_radiance2;                                                     \n"
    "out vec3 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen *                                                           \n"
    "               u_worldview *                                                           \n"
    "               u_nodeworld *                                                           \n"
    "               mix(a_position1, a_position2, u_status);                                \n"
    "       gl_PointSize = (2.0 + 3.0 * mix(                                                \n"
    "               a_radiance1,                                                            \n"
    "               a_radiance2,                                                            \n"
    "               u_status                                                                \n"
    "       ));                                                                             \n"
    "       f_color = mix(                                                                  \n"
    "               (0.2 + a_radiance1) * u_color1,                                         \n"
    "               (0.2 + a_radiance2) * u_color2,                                         \n"
    "               u_status                                                                \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_color;                                                                       \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = vec4(f_color, 1);                                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */

class PointShader : public SpuShader {
public:
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_color1, u_color2;
	float u_status;

	PointShader()
	{
		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};

		Attrs unif_attrs = {
		        {"u_viewsceen", &u_viewsceen},
                        {"u_worldview", &u_worldview},
		        {"u_nodeworld", &u_nodeworld},
                        {"u_color1",    &u_color1   },
		        {"u_color2",    &u_color2   },
                        {"u_status",    &u_status   },
		};
		shapes::loadShader(*this, shader_attrs, unif_attrs);
	}
};

class PointArray : public SpuArray {
public:
	explicit PointArray(const PointShader &shader)
	{
		makeShape1(shader, 0, "a.a_position1");
		makeShape2(shader, 1, "a.a_position2");
		makeRadiance(shader, 2, "a.a_radiance1");
		makeRadiance(shader, 3, "a.a_radiance2");
	}
	void draw() { SpuArray::draw(GL_POINTS); }

private:
	static constexpr auto c_point_count = 4096u;

	template<class T>
	void sendVertex(const PointShader &, int32_t slot, const char *name, const std::vector<T> &data)
	{
		int32_t size = sizeof(T) / 4;  // 32bit unit
		Attrs attrs = {
		        {"buffer_target", GL_ARRAY_BUFFER},
		        {"data",          data.data()    },
		        {"nelem",         data.size()    },
		        {name,            size           },
		};
		aux(attrs, slot);
	}

	void makeShape1(const PointShader &shader, int32_t vbo, const char *name)
	{
		std::vector<vec3f_t> data(c_point_count);
		for (auto &i: data) {
			auto phi = ((rand() % 1001) * 0.001) * math::pi() * 2.0;
			auto rho = ((rand() % 1001) * 0.002 - 1.0) * math::pi() / 2.0;

			i.x = cos(phi) * cos(rho);
			i.y = sin(rho);
			i.z = sin(phi) * cos(rho);
		}
		sendVertex(shader, vbo, name, data);
	}

	void makeShape2(const PointShader &shader, int32_t vbo, const char *name)
	{
		std::vector<vec3f_t> data(c_point_count);

		for (auto &i: data) {
			auto phi = ((rand() % 1001) * 0.001) * math::pi() * 2.0;
			auto rho = ((rand() % 1001) * 0.001) * math::pi() * 2.0;

			i.x = cos(phi) * (0.5 + 0.5 * (1.0 + cos(rho)));
			i.y = sin(rho) * 0.5;
			i.z = sin(phi) * (0.5 + 0.5 * (1.0 + cos(rho)));
		}

		sendVertex(shader, vbo, name, data);
	}

	void makeRadiance(const PointShader &shader, int32_t vbo, const char *name)
	{
		std::vector<float> data(c_point_count);
		for (float &i: data) {
			i = (rand() % 101) * 0.01;
		}
		sendVertex(shader, vbo, name, data);
	}
};

class App : public SpuPage {
public:
	PointShader m_shader;
	PointArray m_array;
	double m_status = 0;

	App(const char *name) : SpuPage(name, true, {0.2, 0.2, 0.2, 0.0}), m_array(m_shader) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_shader.u_color1 = {1.0, 0.5, 0.4};
		m_shader.u_color2 = {1.0, 0.8, 0.7};

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.program_point_size = true;
		renderstate.flags.blend = true;
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto dsec = getSeconds().delta();

		m_shader.u_viewsceen = math::perspective(viewport(0), 48, 1, 20);

		if (long(esec) % 4 == 0) {
			m_status += dsec;
			m_status += dsec;
		}
		else if (m_status != floor(m_status)) {
			if (m_status - floor(m_status) < 0.5) {
				m_status = floor(m_status);
			}
			else {
				m_status = 1.0 + floor(m_status);
			}
		}

		m_shader.u_status = 0.5 - 0.5 * cos(m_status / 2 * math::two_pi());
		m_shader.u_worldview = Mat4f::orbiting(ezero(), esec, 5.5, 0, 0, 0, 19, 0, 40, 15);
		m_shader.u_nodeworld = Mat4f().rot("x", m_status * math::two_pi() / 4);

		m_shader.use();
		m_array.draw();
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("021_morphing");
}  // namespace
}  // namespace spu::oglplus
