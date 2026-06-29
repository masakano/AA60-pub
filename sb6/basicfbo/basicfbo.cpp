//
// FboPainter :
//
#include "base_app.h"
namespace spu::basicfbo {
/* clang-format off */
const char *vert = {
    "#version 410 core                                                                      \n"
    "layout (location = 0) in vec4 position;                                                \n"
    "layout (location = 1) in vec2 texcoord;                                                \n"
    "out VS_OUT                                                                             \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "    vec2 texcoord;                                                                     \n"
    "} vs_out;                                                                              \n"
    "uniform mat4 u_modelview;                                                              \n"
    "uniform mat4 u_viewscreen;                                                             \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_viewscreen * u_modelview * position;                               \n"
    "    vs_out.color = position * 2.0 + vec4(0.5, 0.5, 0.5, 0.0);                          \n"
    "    vs_out.texcoord = texcoord;                                                        \n"
    "}                                                                                      \n"
};

const char *frag1 = {
    "#version 410 core                                                                      \n"
    "in VS_OUT                                                                              \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "    vec2 texcoord;                                                                     \n"
    "} fs_in;                                                                               \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = sin(fs_in.color * vec4(40.0, 20.0, 30.0, 1.0)) * 0.5 + vec4(0.5);          \n"
    "}                                                                                      \n"
};

const char *frag2 = {
    "#version 420 core                                                                      \n"
    "uniform sampler2D u_tex;                                                               \n"
    "out vec4 color;                                                                        \n"
    "in VS_OUT                                                                              \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "    vec2 texcoord;                                                                     \n"
    "} fs_in;                                                                               \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = mix(fs_in.color, texture(u_tex, fs_in.texcoord), 0.7);                     \n"
    "}                                                                                      \n"
};

/* clang-format on */
class FboPainter {
public:
	SpuShader m_shader;
	SpuArray m_array;
	Mat4f u_modelview;
	Mat4f u_viewscreen;
	uint32_t u_tex;
	FboPainter(const char *vert, const char *frag)
	{
		// shader
		{
			Attrs shader_attrs = {
			        {"vert", vert},
			        {"frag", frag},
			};
			Attrs unif_attrs = {
			        {"u_modelview",  &u_modelview },
			        {"u_viewscreen", &u_viewscreen},
			        {"u_tex",        &u_tex       },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			struct Vertex {
				float x, y, z, u, v;
			};
			const float u = 0.25;
			const std::vector<Vertex> vertices = {
			        {-u, -u, +u, 0, 1},
                                {-u, -u, -u, 0, 0},
                                {+u, -u, -u, 1, 0},
                                {+u, -u, -u, 1, 0},
			        {+u, -u, +u, 1, 1},
                                {-u, -u, +u, 0, 1},
                                {+u, -u, -u, 0, 0},
                                {+u, +u, -u, 1, 0},
			        {+u, -u, +u, 0, 1},
                                {+u, +u, -u, 1, 0},
                                {+u, +u, +u, 1, 1},
                                {+u, -u, +u, 0, 1},
			        {+u, +u, -u, 1, 0},
                                {-u, +u, -u, 0, 0},
                                {+u, +u, +u, 1, 1},
                                {-u, +u, -u, 0, 0},
			        {-u, +u, +u, 0, 1},
                                {+u, +u, +u, 1, 1},
                                {-u, +u, -u, 1, 0},
                                {-u, -u, -u, 0, 0},
			        {-u, +u, +u, 1, 1},
                                {-u, -u, -u, 0, 0},
                                {-u, -u, +u, 0, 1},
                                {-u, +u, +u, 1, 1},
			        {-u, +u, -u, 0, 1},
                                {+u, +u, -u, 1, 1},
                                {+u, -u, -u, 1, 0},
                                {+u, -u, -u, 1, 0},
			        {-u, -u, -u, 0, 0},
                                {-u, +u, -u, 0, 1},
                                {-u, -u, +u, 0, 0},
                                {+u, -u, +u, 1, 0},
			        {+u, +u, +u, 1, 1},
                                {+u, +u, +u, 1, 1},
                                {-u, +u, +u, 0, 1},
                                {-u, -u, +u, 0, 0},
			};
			Attrs array_attrs = {
			        {"shader_id",  m_shader.id()},
			        {"a.position", 3            },
			        {"a.texcoord", 2            },
			};
			m_array.init(array_attrs);
			m_array.send(vertices.data(), vertices.size());
		}
	}
	void draw()
	{
		m_shader.use();
		m_array.draw(GL_TRIANGLES, 0, 36);
	}
};

class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name), m_painterA(vert, frag1), m_painterB(vert, frag2) {}

	void init(const Attrs &attrs) override;
	void render() override;
	FboPainter m_painterA;
	FboPainter m_painterB;
	SpuTexture m_color;
	SpuTexture m_depth;
	SpuFrame m_frame;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	{
		Attrs attrs = {
		        {"target",     GL_TEXTURE_2D},
                        {"min_filter", GL_LINEAR    },
		        {"mag_filter", GL_LINEAR    },
                        {"width",      512          },
		        {"height",     512          },
                        {"iformat",    GL_RGBA8     },
		};
		m_color.init(attrs);
	}
	{
		Attrs attrs = {
		        {"target",  GL_TEXTURE_2D        },
		        {"width",   512                  },
		        {"height",  512                  },
		        {"iformat", GL_DEPTH_COMPONENT32F},
		};
		m_depth.init(attrs);
	}
	{
		Rectf viewport = {0, 0, 512, 512};
		Attrs attrs = {
		        {"color0",    m_color.id()},
		        {"depth",     m_depth.id()},
		        {"viewport0", viewport    },
		};
		m_frame.init(attrs);
	}
	{
		Attrs attrs0 = {
		        {"bgcolor0", c_green},
		        {"bgdepth",  1.0    },
		};
		Attrs attrs1 = {
		        {"bgcolor0", c_blue},
		        {"bgdepth",  1.0   },
		};
		m_frame.set(attrs0);
		BaseApp::set(attrs1);
	}
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.cull_face = true;
		renderstate.flags.depth_test = true;
		renderstate.depth_func = GL_LEQUAL;
		// renderstate.use();
	}
}

void App::render()
{
	auto t = getSeconds().current() * 0.5f;
	auto u_modelview = c_unit.rot("XY", t * 81.0, t * 45.0)
	                           .trans({sinf(2.1 * t) * 0.5f, cosf(1.7 * t) * 0.5f,
	                                   sinf(1.3 * t) * cosf(1.5 * t) * 2.0f})
	                           .trans({0.0, 0.0, -2.0});
	sb6::Composition composition_a;
	composition_a.perspective(Rectf(0, 0, 512, 512), 50.0, 0.1, 1000.0);

	sb6::Composition composition_b;
	composition_b.perspective(viewport(0), 50.0, 0.1, 1000.0);

	m_frame.begin();
	m_frame.clear();
	{
		m_painterA.u_modelview = u_modelview;
		m_painterA.u_viewscreen = composition_a.viewscreen();
		m_painterA.draw();
	}
	m_frame.end();

	{
		m_painterB.u_modelview = u_modelview;
		m_painterB.u_viewscreen = composition_b.viewscreen();
		m_painterB.u_tex = m_color.id();
		m_painterB.draw();
	}
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("basicfbo");
}  // namespace spu::basicfbo
