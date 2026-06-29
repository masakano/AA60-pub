//
// App :
//
#include "base_app.h"
namespace spu::wrapmodes {
/* clang-format off */
const char *vs_source = {
    "#version 410 core                                                                      \n"
    "uniform vec2 u_offset;                                                                   \n"
    "out vec2 tex_coord;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    const vec4 vertices[] = vec4[](vec4(-0.45, -0.45, 0.5, 1.0),                       \n"
    "                                   vec4( 0.45, -0.45, 0.5, 1.0),                       \n"
    "                                   vec4(-0.45,  0.45, 0.5, 1.0),                       \n"
    "                                   vec4( 0.45,  0.45, 0.5, 1.0));                      \n"
    "                                                                                       \n"
    "    gl_Position = vertices[gl_VertexID] + vec4(u_offset, 0.0, 0.0);                      \n"
    "    tex_coord = vertices[gl_VertexID].xy * 3.0 + vec2(0.45 * 3);                       \n"
    "}                                                                                      \n"
};

const char *fs_source = {
    "#version 410 core                                                                      \n"
    "uniform sampler2D u_texture;                                                                   \n"
    "out vec4 color;                                                                        \n"
    "in vec2 tex_coord;                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_texture, tex_coord);                                                     \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	void render() override;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	uint32_t u_texture;
	float u_offset[2];
	SpuShader m_shader;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	u_texture = sb6::ktx::load("rightarrows.ktx");
	Attrs shader_attrs = {
	        {"frag", fs_source},
	        {"vert", vs_source},
	};
	Attrs unif_attrs = {
	        {"u_offset",  &u_offset[0]},
	        {"u_texture", &u_texture  },
	};
	loadShader(m_shader, shader_attrs, unif_attrs);
	spu_frame_set(-1, "bgcolor0", c_green);
}

void App::render()
{
	const Vec4f yellow = {0.4, 0.4, 0.0, 1.0};
	const GLenum wrapmodes[] = {GL_CLAMP_TO_EDGE, GL_REPEAT, GL_CLAMP_TO_BORDER, GL_MIRRORED_REPEAT};
	const float offsets[] = {-0.5, -0.5, +0.5, -0.5, -0.5, +0.5, +0.5, +0.5};
	for (auto i = 0; i < 4; i++) {
		Attrs tex_attrs = {
		        {"border", yellow      },
		        {"wrap_s", wrapmodes[i]},
		        {"wrap_t", wrapmodes[i]},
		};
		spu_texture_set(u_texture, tex_attrs);
		u_offset[0] = offsets[i * 2 + 0];
		u_offset[1] = offsets[i * 2 + 1];
		m_shader.use();
		drawFullscreenQuad();
	}
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("wrapmodes");
}  // namespace spu::wrapmodes
