//
// App :
//
#include "base_app.h"
namespace spu::blendmatrix {
/* clang-format off */
const char *vert = {
    "#version 410 core                                                                      \n"
    "in vec4 position;                                                                      \n"
    "out VS_OUT                                                                             \n"
    "{                                                                                      \n"
    "    vec4 color0;                                                                       \n"
    "    vec4 color1;                                                                       \n"
    "} vs_out;                                                                              \n"
    "                                                                                       \n"
    "uniform mat4 u_modelview;                                                              \n"
    "uniform mat4 u_viewscreen;                                                             \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_viewscreen * u_modelview * position;                               \n"
    "    vs_out.color0 = position * 2.0 + vec4(0.5, 0.5, 0.5, 0.0);                         \n"
    "    vs_out.color1 = vec4(0.5, 0.5, 0.5, 0.0) - position * 2.0;                         \n"
    "}                                                                                      \n"
};

const char *frag = {
    "#version 410 core                                                                      \n"
    "layout (location = 0, index = 0) out vec4 color0;                                      \n"
    "layout (location = 0, index = 1) out vec4 color1;                                      \n"
    "in VS_OUT                                                                              \n"
    "{                                                                                      \n"
    "    vec4 color0;                                                                       \n"
    "    vec4 color1;                                                                       \n"
    "} fs_in;                                                                               \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color0 = vec4(fs_in.color0.xyz, 1.0);                                              \n"
    "    color1 = vec4(fs_in.color0.xyz, 1.0);                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;
	SpuShader m_shader;
	uint32_t m_shaderId;
	SpuArray m_array;
	Mat4f u_modelview;
	Mat4f u_viewscreen;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shader
	{
		Attrs shader_attrs = {
		        {"frag", frag},
		        {"vert", vert},
		};
		Attrs unif_attrs = {
		        {"u_modelview",  &u_modelview },
		        {"u_viewscreen", &u_viewscreen},
		};
		loadShader(m_shader, shader_attrs, unif_attrs);
	}
	// array
	{
		const uint32_t indices[] = {0, 1, 2, 2, 1, 3, 2, 3, 4, 4, 3, 5, 4, 5, 6, 6, 5, 7,
		                            6, 7, 0, 0, 7, 1, 6, 0, 2, 2, 4, 6, 7, 5, 3, 7, 3, 1};
		const float positions[] = {
		        -0.25, -0.25, -0.25, -0.25, 0.25,  -0.25, 0.25,  -0.25, -0.25, +0.25, +0.25, -0.25,
		        0.25,  -0.25, 0.25,  +0.25, +0.25, +0.25, -0.25, -0.25, 0.25,  -0.25, 0.25,  +0.25,
		};
		Attrs array_attrs = {
		        {"shader_id",  m_shader.id()},
		        {"a.position", 3            },
		};
		m_array.init(array_attrs);
		m_array.send(positions, 8);
		m_array.send(indices, 36, -1, 4);
	}
	// environment
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.cull_face = true;
		const Vec4f orange = {0.6, 0.4, 0.1, 1.0};
		// spu_frame_set(-1, "bgcolor0", orange);
		spu_frame_set(-1, "bgcolor0", orange);
	}
}

void App::render()
{
	const std::vector<uint32_t> blend_funcs
	        = {GL_ZERO,
	           GL_ONE,
	           GL_SRC_COLOR,
	           GL_ONE_MINUS_SRC_COLOR,
	           GL_DST_COLOR,
	           GL_ONE_MINUS_DST_COLOR,
	           GL_SRC_ALPHA,
	           GL_ONE_MINUS_SRC_ALPHA,
	           GL_DST_ALPHA,
	           GL_ONE_MINUS_DST_ALPHA,
	           GL_CONSTANT_COLOR,
	           GL_ONE_MINUS_CONSTANT_COLOR,
	           GL_CONSTANT_ALPHA,
	           GL_ONE_MINUS_CONSTANT_ALPHA,
	           GL_SRC_ALPHA_SATURATE,
	           GL_SRC1_COLOR,
	           GL_ONE_MINUS_SRC1_COLOR,
	           GL_SRC1_ALPHA,
	           GL_ONE_MINUS_SRC1_ALPHA};

	const auto x_scale = 20.0f / blend_funcs.size();
	const auto y_scale = 16.0f / blend_funcs.size();
	const auto t = getSeconds().current();

	sb6::Composition composition;
	composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
	u_viewscreen = composition.viewscreen();
	auto &renderstate = getRenderstate();
	renderstate.flags.blend = true;
	renderstate.blend_color = {0.2, 0.5, 0.7, 0.5};

	for (auto j = 0u; j < blend_funcs.size(); j++) {
		for (auto i = 0u; i < blend_funcs.size(); i++) {
			u_modelview = c_unit.rot("XY", -t * -21.0, -t * -45.0)
			                      .trans({9.5f - x_scale * i, 7.5f - y_scale * j, -18.0f});

			renderstate.blend_func = {
			        blend_funcs[i],
			        blend_funcs[j],
			        blend_funcs[i],
			        blend_funcs[j],
			};

			renderstate.use();
			m_shader.use();
			m_array.draw(GL_TRIANGLES, 0, 36);
		}
	}
}
static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("blendmatrix");
}  // namespace spu::blendmatrix
