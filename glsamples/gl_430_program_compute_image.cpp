//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

const std::vector<v4fv4fv4f_t> c_vertices = {
        {-1.0, -1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0},
        {+1.0, -1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0},
        {+1.0, +1.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0},
        {-1.0, +1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0},
        {-1.0, -1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.5, 0.5, 1.0},
        {+1.0, -1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.5, 1.0},
        {+1.0, +1.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.5, 1.0, 0.0, 1.0},
        {-1.0, +1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.5, 0.5, 1.0, 1.0},
};

const std::vector<Vec4f> c_positions = {
        {-1.0, -1.0, 0.0, 1.0},
        {+1.0, -1.0, 0.0, 1.0},
        {+1.0, +1.0, 0.0, 1.0},
        {-1.0, +1.0, 0.0, 1.0},
        {-1.0, -1.0, 0.0, 1.0},
        {+1.0, -1.0, 0.0, 1.0},
        {+1.0, +1.0, 0.0, 1.0},
        {-1.0, +1.0, 0.0, 1.0},
};

const std::vector<Vec4f> c_texcoords = {
        {0.0, 1.0, 0.0, 0.0},
        {1.0, 1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0},
        {0.0, 1.0, 0.0, 0.0},
        {1.0, 1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0},
};

const std::vector<Vec4f> c_colors = {
        {1.0, 0.0, 0.0, +1.0},
        {1.0, 1.0, 0.0, +1.0},
        {0.0, 1.0, 0.0, +1.0},
        {0.0, 0.0, 1.0, +1.0},
        {1.0, 0.5, 0.5, +1.0},
        {1.0, 1.0, 0.5, +1.0},
        {0.5, 1.0, 0.0, +1.0},
        {0.5, 0.5, 1.0, +1.0},
};

const auto c_position_size = c_positions.size() * sizeof(c_positions[0]);

/* clang-format off */
const char *c_vert = {
    "#version 420 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "uniform samplerBuffer u_position;                                                      \n"
    "uniform samplerBuffer u_texcoord;                                                      \n"
    "uniform samplerBuffer u_color;                                                         \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "out vec4 f_texcoord;                                                                   \n"
    "out vec4 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = texelFetch(u_texcoord, gl_VertexID);                                  \n"
    "    f_color = texelFetch(u_color, gl_VertexID);                                        \n"
    "    gl_Position = texelFetch(u_position, gl_VertexID);                                 \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec4 f_texcoord;                                                                    \n"
    "in vec4 f_color;                                                                       \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, f_texcoord.st) * f_color;                               \n"
    "}                                                                                      \n"
};

const char *c_comp = {
    "#version 420 core                                                                      \n"
    "#extension GL_ARB_compute_shader : require                                             \n"
    "layout (local_size_x = 8) in;                                                          \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "layout(binding = 0, rgba32f) readonly uniform imageBuffer u_position_input;            \n"
    "layout(binding = 1, rgba32f) readonly uniform imageBuffer u_texcoord_input;            \n"
    "layout(binding = 2, rgba32f) readonly uniform imageBuffer u_color_input;               \n"
    "layout(binding = 3, rgba32f) writeonly uniform imageBuffer u_position_output;          \n"
    "layout(binding = 4, rgba32f) writeonly uniform imageBuffer u_texcoord_output;          \n"
    "layout(binding = 5, rgba32f) writeonly uniform imageBuffer u_color_output;             \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    int vertexID = int(gl_LocalInvocationIndex);                                       \n"
    "    vec4 position = u_worldscreen * imageLoad(u_position_input, vertexID);              \n"
    "    vec4 texcoord = imageLoad(u_texcoord_input, vertexID);                             \n"
    "    vec4 color = imageLoad(u_color_input, vertexID);                                   \n"
    "    imageStore(u_position_output, vertexID, position);                                 \n"
    "    imageStore(u_texcoord_output, vertexID, texcoord);                                 \n"
    "    imageStore(u_color_output, vertexID, color * 2.0);                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	// 0:diffuse 1:position_in 2:texcoord_in 3:color_in 4:position_out 5:texcoord_out
	// 6:color_out
	uint32_t m_textureIds[7];

	SpuShader m_compShader;
	SpuShader m_shader;
	Mat4f u_worldscreen;

	uint32_t m_computeId;
	uint32_t m_arrayId;

	uint32_t u_diffuse;
	uint32_t u_position;
	uint32_t u_texcoord;
	uint32_t u_color;

	uint32_t u_position_input;
	uint32_t u_texcoord_input;
	uint32_t u_color_input;
	uint32_t u_position_output;
	uint32_t u_texcoord_output;
	uint32_t u_color_output;

	App(const char *name) : BaseApp(name, true, c_white) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program graphics
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"worldscreen", &u_worldscreen},
                                {"u_diffuse",   &u_diffuse    },
			        {"u_position",  &u_position   },
                                {"u_texcoord",  &u_texcoord   },
			        {"u_color",     &u_color      },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// program compute
		{
			Attrs shader_attrs = {
			        {"comp", c_comp},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen",     &u_worldscreen    },
			        {"u_position_input",  &u_position_input },
			        {"u_texcoord_input",  &u_texcoord_input },
			        {"u_color_input",     &u_color_input    },
			        {"u_position_output", &u_position_output},
			        {"u_texcoord_output", &u_texcoord_output},
			        {"u_color_output",    &u_color_output   },
			};
			loadShader(m_compShader, shader_attrs, unif_attrs);
		}

		// array
		{
			const std::vector<uint16_t> c_indices = {0, 1, 2, 2, 3, 0};

			m_computeId = spu_array_new(Attrs());

			Attrs model_attrs = {
			        {"nelem", 1},
			};

			m_arrayId = spu_array_new(model_attrs);
			spu_array_send(m_arrayId, c_indices.data(), c_indices.size(), -1, 2);
		}

		// texture
		{
			Attrs attrs = {
			        {"mag_filter", GL_LINEAR              },
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			};
			m_textureIds[0] = loadDDS("kueken7_bgra8_srgb.dds", attrs);

			for (auto i = 1; i < 7; i++) {
				Attrs attrs = {
				        {"target",  GL_TEXTURE_BUFFER},
				        {"iformat", GL_RGBA32F       },
				        {"size",    c_position_size  },
				};
				m_textureIds[i] = spu_texture_new(attrs);
			}

			spu_texture_send(m_textureIds[1], c_positions.data(), GL_RGBA32F);
			spu_texture_send(m_textureIds[2], c_texcoords.data(), GL_RGBA32F);
			spu_texture_send(m_textureIds[3], c_colors.data(), GL_RGBA32F);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

		u_worldscreen = getCamera().worldscreen();

		u_position_input = m_textureIds[1];
		u_texcoord_input = m_textureIds[2];
		u_color_input = m_textureIds[3];

		u_position_output = m_textureIds[4];
		u_texcoord_output = m_textureIds[5];
		u_color_output = m_textureIds[6];

		m_compShader.use();

		spu_array_draw(m_computeId, 0xffff, c_vertices.size(), 1, 1);

		u_diffuse = m_textureIds[0];
		u_position = m_textureIds[4];
		u_texcoord = m_textureIds[5];
		u_color = m_textureIds[6];

		m_shader.use();

		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_430_program_compute_image");
}  // namespace
}  // namespace spu
