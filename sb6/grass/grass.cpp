//
// App :
//
#include "base_app.h"
namespace spu::grass {

/* clang-format off */
const char *grass_vs_source = {
    "#version 420 core                                                                      \n"
    "                                                                                       \n"
    "// Incoming per vertex positio                                                         \n"
    "in vec4 vVertex;                                                                       \n"
    "                                                                                       \n"
    "// Output varyings                                                                     \n"
    "out vec4 color;                                                                        \n"
    "                                                                                       \n"
    "uniform mat4 u_modelscreen;                                                            \n"
    "                                                                                       \n"
    "layout (binding = 0) uniform sampler1D grasspallete_texture;                           \n"
    "layout (binding = 1) uniform sampler2D u_length_texture;                               \n"
    "layout (binding = 2) uniform sampler2D u_orientation_texture;                          \n"
    "layout (binding = 3) uniform sampler2D u_grasscolor_texture;                           \n"
    "layout (binding = 4) uniform sampler2D u_bend_texture;                                 \n"
    "                                                                                       \n"
    "int random(int seed, int iterations)                                                   \n"
    "{                                                                                      \n"
    "    int value = seed;                                                                  \n"
    "    int n;                                                                             \n"
    "                                                                                       \n"
    "    for (n = 0; n < iterations; n++) {                                                 \n"
    "        value = ((value >> 7) ^ (value << 9)) * 15485863;                              \n"
    "    }                                                                                  \n"
    "                                                                                       \n"
    "    return value;                                                                      \n"
    "}                                                                                      \n"
    "                                                                                       \n"
    "vec4 random_vector(int seed)                                                           \n"
    "{                                                                                      \n"
    "    int r = random(gl_InstanceID, 4);                                                  \n"
    "    int g = random(r, 2);                                                              \n"
    "    int b = random(g, 2);                                                              \n"
    "    int a = random(b, 2);                                                              \n"
    "                                                                                       \n"
    "    return vec4(float(r & 0x3FF) / 1024.0,                                             \n"
    "                float(g & 0x3FF) / 1024.0,                                             \n"
    "                float(b & 0x3FF) / 1024.0,                                             \n"
    "                float(a & 0x3FF) / 1024.0);                                            \n"
    "}                                                                                      \n"
    "                                                                                       \n"
    "mat4 construct_rotation_matrix(float angle)                                            \n"
    "{                                                                                      \n"
    "    float st = sin(angle);                                                             \n"
    "    float ct = cos(angle);                                                             \n"
    "                                                                                       \n"
    "    return mat4(vec4(ct, 0.0, st, 0.0),                                                \n"
    "                vec4(0.0, 1.0, 0.0, 0.0),                                              \n"
    "                vec4(-st, 0.0, ct, 0.0),                                               \n"
    "                vec4(0.0, 0.0, 0.0, 1.0));                                             \n"
    "}                                                                                      \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec4 offset = vec4(float(gl_InstanceID >> 10) - 512.0,                             \n"
    "                       0.0,                                                            \n"
    "                       float(gl_InstanceID & 0x3FF) - 512.0,                           \n"
    "                       0.0);                                                           \n"
    "    int number1 = random(gl_InstanceID, 3);                                            \n"
    "    int number2 = random(number1, 2);                                                  \n"
    "    offset += vec4(float(number1 & 0xFF) / 256.0,                                      \n"
    "                   0.0,                                                                \n"
    "                   float(number2 & 0xFF) / 256.0,                                      \n"
    "                   0.0);                                                               \n"
    "    // float angle = float(random(number2, 2) & 0x3FF) / 1024.0;                       \n"
    "                                                                                       \n"
    "    vec2 texcoord = offset.xz / 1024.0 + vec2(0.5);                                    \n"
    "                                                                                       \n"
    "    // float bend_factor = float(random(number2, 7) & 0x3FF) / 1024.0;                 \n"
    "    float bend_factor = texture(u_bend_texture, texcoord).r * 2.0;                     \n"
    "    float bend_amount = cos(vVertex.y);                                                \n"
    "                                                                                       \n"
    "    float angle = texture(u_orientation_texture, texcoord).r * 2.0 * 141592;           \n"
    "    mat4 rot = construct_rotation_matrix(angle);                                       \n"
    "    vec4 position = (rot * (vVertex + vec4(0.0, 0.0, bend_amount * bend_factor, 0.0))) + offset;\n"
    "                                                                                       \n"
    "    //position *= vec4(1.0, texture(u_length_texture, texcoord).r * 0.9 + 3, 1.0, 1.0);\n"
    "    position *= vec4(1.0, 10*texture(u_length_texture, texcoord).r * 0.9 + 3, 1.0, 1.0);\n"
    "                                                                                       \n"
    "    gl_Position = u_modelscreen * position; // (rot * position);                       \n"
    "    color = vec4(random_vector(gl_InstanceID).xyz * vec3(0.1, 0.5, 0.1) + vec3(0.1, 0.4, 0.1), 1.0);\n"
    "    //color = texture(u_orientation_texture, texcoord);                                \n"
/* (does not work) */
/*
    "    color = texture(grasspallete_texture, texture(u_grasscolor_texture, texcoord).r) + \n"
    "            vec4(random_vector(gl_InstanceID).xyz * vec3(0.1, 0.5, 0.1), 1.0);         \n"
*/
    "}                                                                                      \n"
};

const char *grass_fs_source = {
    "#version 420 core                                                                      \n"
    "in vec4 color;                                                                         \n"
    "out vec4 output_color;                                                                 \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    output_color = color;                                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;
	SpuShader m_shader;
	SpuArray m_grass;

	uint32_t u_grasscolor_texture;
	uint32_t u_length_texture;
	uint32_t u_orientation_texture;
	uint32_t u_bend_texture;
	uint32_t u_grasspallet_texture;
	Mat4f u_modelscreen;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	const float grass_blade[] = {-0.30, 0.0, 0.30, 0.0, -0.20, 1.0, 0.10, 1.3, -0.05, 2.3, 0.00, 3.3};
	Attrs array_attrs = {
	        {"a.0", 2},
	};
	m_grass.init(array_attrs);
	m_grass.send(grass_blade, 6);
	// shader
	{
		Attrs shader_attrs = {
		        {"frag", grass_fs_source},
		        {"vert", grass_vs_source},
		};
		Attrs unif_attrs = {
		        {"u_modelscreen",         &u_modelscreen        },
		        {"u_grasspallet_texture", &u_grasspallet_texture},
		        {"u_length_texture",      &u_length_texture     },
		        {"u_orientation_texture", &u_orientation_texture},
		        {"u_grasscolor_texture",  &u_grasscolor_texture },
		        {"u_bend_texture",        &u_bend_texture       },
		};
		loadShader(m_shader, shader_attrs, unif_attrs);
	}
	u_grasspallet_texture = 0;  // invalid
	u_length_texture = sb6::ktx::load("grass_length.ktx");
	u_orientation_texture = sb6::ktx::load("grass_orientation.ktx");
	u_grasscolor_texture = sb6::ktx::load("grass_color.ktx");
	u_bend_texture = sb6::ktx::load("grass_bend.ktx");
	auto &renderstate = getRenderstate();
	renderstate.flags.depth_test = true;
	renderstate.depth_func = GL_LEQUAL;
	// renderstate.use();
}

void App::render()
{
	auto t = getSeconds().current() * 0.02;  // 0.02sec
	auto r = 550.0f;
	sb6::Composition composition;
	composition.lookat(Vec3f(sinf(t) * r, 25.0, cosf(t) * r), Vec3f(0.0, -50.0, 0.0), ey());
	composition.perspective(viewport(0), 45.0, 0.1, 1000.0);
	u_modelscreen = composition.worldscreen();
	m_shader.use();
	m_grass.draw(GL_TRIANGLE_STRIP, 0, 6, 1024 * 1024);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("grass");
}  // namespace spu::grass
