//
// App :
//
#include "base_app.h"
namespace spu::noperspective {
/* clang-format off */
const char *vs_source = {
    "#version 410 core                                                                      \n"
    "out VS_OUT                                                                             \n"
    "{                                                                                      \n"
    "    vec2 tc;                                                                           \n"
    "    noperspective vec2 tc_np;                                                          \n"
    "} vs_out;                                                                              \n"
    "uniform mat4 u_modelscreen;                                                            \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    const vec4 vertices[] = vec4[](vec4(-0.5, -0.5, 0.0,       1.0),                   \n"
    "                                   vec4( 0.5, -0.5, 0.0,       1.0),                   \n"
    "                                   vec4(-0.5,  0.5, 0.0,       1.0),                   \n"
    "                                   vec4( 0.5,  0.5, 0.0,       1.0));                  \n"
    "                                                                                       \n"
    "    vec2 tc = (vertices[gl_VertexID].xy + vec2(0.5));                                  \n"
    "    vs_out.tc = tc;                                                                    \n"
    "    vs_out.tc_np = tc;                                                                 \n"
    "    gl_Position = u_modelscreen * vertices[gl_VertexID];                               \n"
    "}                                                                                      \n"
};

const char *fs_source = {
    "#version 410 core                                                                      \n"
    "out vec4 color;                                                                        \n"
    "uniform sampler2D u_texture;                                                           \n"
    "uniform bool u_use_perspective;                                                        \n"
    "in VS_OUT                                                                              \n"
    "{                                                                                      \n"
    "    vec2 tc;                                                                           \n"
    "    noperspective vec2 tc_np;                                                          \n"
    "} fs_in;                                                                               \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec2 tc = mix(fs_in.tc_np, fs_in.tc, bvec2(u_use_perspective));                    \n"
    "    color = texture(u_texture, tc).rrrr;                                               \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	void render() override;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	SpuShader m_shader;
	Mat4f u_modelscreen;
	uint32_t u_texture;
	int32_t u_use_perspective = false;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shader
	{
		Attrs shader_attrs = {
		        {"frag", fs_source},
		        {"vert", vs_source},
		};
		Attrs unif_attrs = {
		        {"u_modelscreen",     &u_modelscreen    },
		        {"u_use_perspective", &u_use_perspective},
		        {"u_texture",         &u_texture        },
		};
		loadShader(m_shader, shader_attrs, unif_attrs);
	}

	// texture
	{
		const std::vector<uint8_t> checker_datas = {
		        0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
		        0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00,
		        0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
		        0xFF, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0xFF,
		        0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
		};
		// const void *pixv[] = { checker_data, 0 };
		Attrs attr = {
		        {"target",     GL_TEXTURE_2D       },
		        {"iformat",    GL_R8               },
		        {"width",      8                   },
		        {"height",     8                   },
		        {"mag_filter", GL_NEAREST          },
		        {"min_filter", GL_NEAREST          },
		        {"wrap_s",     GL_CLAMP_TO_EDGE    },
		        {"wrap_t",     GL_CLAMP_TO_EDGE    },
		        {"data",       checker_datas.data()},
		};
		u_texture = spu_texture_new(attr);
	}
}

void App::render()
{
	auto t = getSeconds().current() * 14.3f;
	Mat4f viewscreen;
	sb6::Composition composition;
	composition.perspective(viewport(0), 60.0, 0.1, 1000.0);
	viewscreen = composition.viewscreen();
	Mat4f modelview = c_unit.rot("Y", -t).trans({0.0, 0.0, -1.5});
	u_modelscreen = viewscreen * modelview;
	m_shader.use();
	drawFullscreenQuad();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("noperspective");
}  // namespace spu::noperspective
