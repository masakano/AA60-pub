//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "in vec2 a_position;                                                                    \n"
    "out vec2 f_position;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = vec4(a_position, 0.0, 1.0);                                       \n"
    "       f_position = gl_Position.xy;                                                    \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "uniform float u_time;                                                                  \n"
    "uniform vec2 u_sun_position;                                                           \n"
    "uniform vec3 u_sun1;                                                                   \n"
    "uniform vec3 u_sun2;                                                                   \n"
    "uniform vec3 u_sky1;                                                                   \n"
    "uniform vec3 u_sky2;                                                                   \n"
    "in vec2 f_position;                                                                    \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec2 v = f_position - u_sun_position;                                           \n"
    "       float l = length(v);                                                            \n"
    "       float a = atan(v.y, v.x)/3.1415;                                                \n"
    "       if (l < 0.1)                                                                    \n"
    "               final_color = u_sun1;                                                   \n"
    "       else if (int(18*(u_time*0.1 + 1.0 + a)) % 2 == 0)                               \n"
    "               final_color = mix(u_sun1, u_sun2, l);                                   \n"
    "       else                                                                            \n"
    "               final_color = mix(u_sky1, u_sky2, l);                                   \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	SpuShader m_shader;
	SpuArray m_array;

	Vec3f u_sun1 = Vec3f(0.95, 0.85, 0.60);
	Vec3f u_sun2 = Vec3f(0.90, 0.80, 0.20);
	Vec3f u_sky1 = Vec3f(0.90, 0.80, 0.50);
	Vec3f u_sky2 = Vec3f(0.80, 0.60, 0.40);
	Vec2f u_sun_position;
	float u_time;

	App(const char *name) : SpuPage(name, true) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};

		Attrs unif_attrs = {
		        {"u_sun1",         &u_sun1        },
		        {"u_sun2",         &u_sun2        },
		        {"u_sky1",         &u_sky1        },
		        {"u_sky2",         &u_sky2        },
		        {"u_sun_position", &u_sun_position},
		        {"u_time",         &u_time        },
		};
		shapes::loadShader(m_shader, shader_attrs, unif_attrs);

		std::vector<vec2f_t> vertices = {
		        {-1.0, -1.0},
		        {-1.0, +1.0},
		        {+1.0, -1.0},
		        {+1.0, +1.0},
		};

		Attrs vert_attrs = {
		        {"shader_id",    m_shader.id()  },
		        {"a.a_position", 2              },
		        {"data",         vertices.data()},
		        {"nelem",        vertices.size()},
		};

		m_array.init(vert_attrs);
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto angle = esec * 0.05 * math::two_pi();
		u_time = esec;
		u_sun_position = Vec2f(-cos(angle), sin(angle));
		m_shader.use();
		m_array.draw(GL_TRIANGLE_STRIP, 0, 4);
	}
};
static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("006_cartoon_sun");
}  // namespace
}  // namespace spu::oglplus
