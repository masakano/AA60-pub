//
// App :
//
#include <cmath>
#include <ssys/random_generator.h>
#include "base_app.h"
namespace spu::starfield {
enum { e_num_stars = 2000 };
/* clang-format off */
const char *fs_source = {
    "#version 410 core                                                                      \n"
    "layout (location = 0) out vec4 color;                                                  \n"
    "uniform sampler2D u_star_texture;                                                      \n"
    "flat in vec4 starColor;                                                                \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = starColor * texture(u_star_texture, gl_PointCoord);                        \n"
    "}                                                                                      \n"
};

const char *vs_source = {
    "#version 410 core                                                                      \n"
    "layout (location = 0) in vec4 position;                                                \n"
    "layout (location = 1) in vec4 color;                                                   \n"
    "uniform float u_time;                                                                  \n"
    "uniform mat4 u_viewscreen;                                                             \n"
    "flat out vec4 starColor;                                                               \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec4 newVertex = position;                                                         \n"
    "    newVertex.z += u_time;                                                             \n"
    "    newVertex.z = fract(newVertex.z);                                                  \n"
    "    float size = (20.0 * newVertex.z * newVertex.z);                                   \n"
    "    starColor = smoothstep(1.0, 7.0, size) * color;                                    \n"
    "    newVertex.z = (999.9 * newVertex.z) - 1000.0;                                      \n"
    "    gl_Position = u_viewscreen * newVertex;                                            \n"
    "    gl_PointSize = size;                                                               \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	void render() override;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	SpuShader m_shader;
	SpuArray m_array;

	uint32_t u_star_texture;
	float u_time;
	Mat4f u_viewscreen;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	Attrs shader_attrs = {
	        {"frag", fs_source},
	        {"vert", vs_source},
	};
	Attrs unif_attrs = {
	        {"u_time",         &u_time        },
	        {"u_star_texture", &u_star_texture},
	        {"u_viewscreen",   &u_viewscreen  },
	};
	loadShader(m_shader, shader_attrs, unif_attrs);
	u_star_texture = sb6::ktx::load("star.ktx");
	struct Star {
		vec3f_t position;
		vec3f_t color;
	};
	Attrs array_attrs = {
	        {"a.0", 3},
	        {"a.1", 3},
	};
	m_array.init(array_attrs);
	Star star[e_num_stars];
	RandomGenerator<float> frand;
	for (auto i = 0; i < e_num_stars; i++) {
		star[i].position.f[0] = (frand() * 2.0 - 1.0) * 100.0;
		star[i].position.f[1] = (frand() * 2.0 - 1.0) * 100.0;
		star[i].position.f[2] = frand();
		star[i].color.f[0] = 0.8 + frand() * 0.2;
		star[i].color.f[1] = 0.8 + frand() * 0.2;
		star[i].color.f[2] = 0.8 + frand() * 0.2;
	}
	m_array.send(star, e_num_stars);
}

void App::render()
{
	u_time = getSeconds().current();
	sb6::Composition composition;
	composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
	u_viewscreen = composition.viewscreen();
	u_time *= 0.1f;
	u_time -= floor(u_time);
	auto &renderstate = getRenderstate();
	renderstate.flags.blend = true;
	renderstate.blend_func = {
	        GL_ONE,
	        GL_ONE,
	        GL_ONE,
	        GL_ONE,
	};
	renderstate.flags.point_sprite = true;
	renderstate.flags.program_point_size = true;
	renderstate.use();
	m_shader.use();
	m_array.draw(GL_POINTS, 0, e_num_stars);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("starfield");
}  // namespace spu::starfield
