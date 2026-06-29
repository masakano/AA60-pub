//
// App :
//
#include "base_app.h"
namespace spu::tunnel {
/* clang-format off */
const char *vs_source = {
    "#version 420 core                                                                      \n"
    "out VS_OUT                                                                             \n"
    "{                                                                                      \n"
    "    vec2 tc;                                                                           \n"
    "} vs_out;                                                                              \n"
    "uniform mat4 u_modelscreen;                                                            \n"
    "uniform float u_offset;                                                                \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    const vec2[4] position = vec2[4](vec2(-0.5, -0.5),                                 \n"
    "                                     vec2( 0.5, -0.5),                                 \n"
    "                                     vec2(-0.5,  0.5),                                 \n"
    "                                     vec2( 0.5,  0.5));                                \n"
    "    vs_out.tc = (position[gl_VertexID].xy + vec2(u_offset, 0.5)) * vec2(30.0, 1.0);    \n"
    "    gl_Position = u_modelscreen * vec4(position[gl_VertexID], 0.0, 1.0);               \n"
    "}                                                                                      \n"
};

const char *fs_source = {
    "#version 420 core                                                                      \n"
    "layout (location = 0) out vec4 color;                                                  \n"
    "in VS_OUT                                                                              \n"
    "{                                                                                      \n"
    "    vec2 tc;                                                                           \n"
    "} fs_in;                                                                               \n"
    "layout (binding = 0) uniform sampler2D u_texture;                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_texture, fs_in.tc);                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	void render() override;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	SpuShader m_shader;
	uint32_t m_wallTexture;
	uint32_t m_ceilingTexture;
	uint32_t m_floorTexture;

	Mat4f u_modelscreen;
	float u_offset;
	uint32_t u_texture;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	Attrs shader_attrs = {
	        {"frag", fs_source},
	        {"vert", vs_source},
	        //{"use_block_unif", false},
	};
	Attrs unif_attrs = {
	        {"u_modelscreen", &u_modelscreen},
	        {"u_offset",      &u_offset     },
	        {"u_texture",     &u_texture    },
	};
	loadShader(m_shader, shader_attrs, unif_attrs);

	spu::Attrs tex_attrs = {
	        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
	        {"mag_filter", GL_LINEAR              },
	};
	m_wallTexture = sb6::ktx::load("brick.ktx", tex_attrs);
	m_ceilingTexture = sb6::ktx::load("ceiling.ktx", tex_attrs);
	m_floorTexture = sb6::ktx::load("floor.ktx", tex_attrs);
}

void App::render()
{
	auto t = getSeconds().current();
	sb6::Composition composition;
	composition.perspective(viewport(0), 60.0, 0.1, 100.0);
	u_offset = t * 0.003;
	uint32_t textures[] = {m_wallTexture, m_floorTexture, m_wallTexture, m_ceilingTexture};
	for (auto i = 0; i < 4; i++) {
		auto u_modelview = c_unit.scale({30.0, 1.0, 1.0})
		                           .rot("Y", -90.0)
		                           .trans({-0.5, 0.0, -10.0})
		                           .rot("Z", -90.0 * i);
		u_modelscreen = composition.viewscreen() * u_modelview;
		u_texture = textures[i];
		m_shader.use();
		drawFullscreenQuad();
	}
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("tunnel");
}  // namespace spu::tunnel
