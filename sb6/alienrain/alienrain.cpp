//
// App :
//
#include <ssys/random_generator.h>
#include "base_app.h"
namespace spu::alienrain {

/* clang-format off */
const char *vert = {
    "#version 410 core                                                                      \n"
    "                                                                                       \n"
    "layout (location = 0) in int alien_index;                                              \n"
    "                                                                                       \n"
    "out VS_OUT                                                                             \n"
    "{                                                                                      \n"
    "    flat int alien;                                                                    \n"
    "    vec2 tc;                                                                           \n"
    "} vs_out;                                                                              \n"
    "                                                                                       \n"
    "struct droplet_t                                                                       \n"
    "{                                                                                      \n"
    "    float x_offset;                                                                    \n"
    "    float y_offset;                                                                    \n"
    "    float orientation;                                                                 \n"
    "    float unused;                                                                      \n"
    "};                                                                                     \n"
    "                                                                                       \n"
    "layout (std140) uniform droplets                                                       \n"
    "{                                                                                      \n"
    "    droplet_t droplet[256];                                                            \n"
    "};                                                                                     \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    const vec2[4] position = vec2[4](vec2(-0.5, -0.5),                                 \n"
    "                                     vec2( 0.5, -0.5),                                 \n"
    "                                     vec2(-0.5,  0.5),                                 \n"
    "                                     vec2( 0.5,  0.5));                                \n"
    "    vs_out.tc = position[gl_VertexID].xy + vec2(0.5);                                  \n"
    "    float co = cos(droplet[alien_index].orientation);                                  \n"
    "    float so = sin(droplet[alien_index].orientation);                                  \n"
    "    mat2 rot = mat2(vec2(co, so),                                                      \n"
    "                    vec2(-so, co));                                                    \n"
    "    vec2 pos = 0.25 * rot * position[gl_VertexID];                                     \n"
    "    gl_Position = vec4(pos.x + droplet[alien_index].x_offset,                          \n"
    "                       pos.y + droplet[alien_index].y_offset,                          \n"
    "                       0.5, 1.0);                                                      \n"
    "    vs_out.alien = alien_index % 64;                                                   \n"
    "}                                                                                      \n"
};

const char *frag = {
    "#version 410 core                                                                      \n"
    "                                                                                       \n"
    "layout (location = 0) out vec4 color;                                                  \n"
    "                                                                                       \n"
    "in VS_OUT                                                                              \n"
    "{                                                                                      \n"
    "    flat int alien;                                                                    \n"
    "    vec2 tc;                                                                           \n"
    "} fs_in;                                                                               \n"
    "                                                                                       \n"
    "uniform sampler2DArray tex_aliens;                                                     \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(tex_aliens, vec3(fs_in.tc, float(fs_in.alien)));                   \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;
	void send(int32_t val);
	uint32_t u_tex_aliens;
	float m_xOffset[256];
	float m_rotSpeed[256];
	float m_fallSpeed[256];
	SpuShader m_shader;
	SpuArray m_array;
	Vec4f u_droplet[256];
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	RandomGenerator<float> frand;
	// shader
	{
		Attrs shader_attrs = {
		        {"frag", frag},
		        {"vert", vert},
		};
		Attrs unif_attrs = {
		        {"tex_aliens", &u_tex_aliens},
		        {"droplets",   u_droplet    },
		};
		loadShader(m_shader, shader_attrs, unif_attrs);
	}
	// array && tex
	{
		Attrs attrs = {
		        {"shader_id",     m_shader.id()},
		        {"format",        GL_INT       },
		        {"oformat",       GL_INT       },
		        {"a.alien_index", 1            },
		};
		m_array.init(attrs);
		send(0);
		u_tex_aliens = sb6::ktx::load("aliens.ktx");
	}
	// position
	{
		for (auto i = 0; i < 256; i++) {
			m_xOffset[i] = frand() * 2.0 - 1.0;
			m_rotSpeed[i] = (frand() + 0.5) * ((i & 1) ? -3.0 : 3.0);
			m_fallSpeed[i] = frand() + 0.2;
		}
	}
	// renderstate
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.blend = true;
		renderstate.blend_func = {
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		};
	}
}

void App::render()
{
	auto t = getSeconds().current();
	for (auto i = 0; i < 256; i++) {
		u_droplet[i].x = m_xOffset[i];
		u_droplet[i].y = 2.0 - fmodf((t + i) * m_fallSpeed[i], 4.31);
		u_droplet[i].z = t * m_rotSpeed[i];
	}
	for (auto alien_index = 0; alien_index < 256; alien_index++) {
		send(alien_index);
		m_shader.use();
		m_array.draw(GL_TRIANGLE_STRIP, 0, 4);
	}
}

void App::send(int32_t val)
{
	int32_t vert[] = {val, val, val, val};
	m_array.send(vert, 4);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("alienrain");
}  // namespace spu::alienrain
