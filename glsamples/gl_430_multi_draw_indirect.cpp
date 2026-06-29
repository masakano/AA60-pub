//
// App :
//
#include "base_app.h"
namespace spu {
namespace {
/* clang-format off */
const char *c_vert = {
    "#version 420 core                                                                      \n"
    "#define MAX_DRAW        3                                                              \n"

    "uniform UB_TRANSFORM                                                                   \n"
    "{                                                                                      \n"
    "    int transform[MAX_DRAW];                                                           \n"
    "    mat4 worldscreen[MAX_DRAW];                                                         \n"
    "} ub_transform;                                                                        \n"
    
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "in int drawID;                                                                         \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "out vec2 f_texcoord;                                                                   \n"
    "flat out int f_drawID;                                                                 \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_drawID = drawID;                                                                 \n"
    "    f_texcoord = a_texcoord.st;                                                        \n"
    "    gl_Position =                                                                      \n"
    "           ub_transform.worldscreen[ub_transform.transform[drawID]] *                   \n"
    "           vec4(a_position, 0.0, 1.0);                                                 \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "in vec2 f_texcoord;                                                                    \n"
    "flat in int f_drawID;                                                                  \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "uniform sampler2D u_albedos[3];                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_albedos[f_drawID], f_texcoord.st);                               \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	std::vector<Rectf> m_viewports;
	SpuShader m_shader;
	uint32_t u_albedos[3];
	uint32_t m_arrayId;

	struct {
		int32_t transform[4];
		Mat4f worldscreen[4];
	} ub_transform;

	struct DrawElementsIndirectCommand {
		uint32_t m_primitiveCount;
		uint32_t m_instanceCount;
		uint32_t m_firstIndex;
		int32_t m_baseVertex;
		uint32_t m_baseInstance;
	};

	App(const char *name) : BaseApp(name, true, c_white) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"ub_transform", &ub_transform},
			        {"u_albedos",    &u_albedos[0]},
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}

		// c_vertices
		{
			const std::vector<v2fv2f_t> c_vertices = {
			        {-1.0, -1.0, +0.0, +1.0},
                                {+1.0, -1.0, +1.0, +1.0},
                                {+1.0, +1.0, +1.0, +0.0},
			        {-1.0, +1.0, +0.0, +0.0},
                                {-0.5, -1.0, +0.0, +1.0},
                                {+1.5, -1.0, +1.0, +1.0},
			        {+0.5, +1.0, +1.0, +0.0},
                                {-0.5, -1.0, +0.0, +1.0},
                                {+0.5, -1.0, +1.0, +1.0},
			        {+1.5, +1.0, +1.0, +0.0},
                                {-1.5, +1.0, +0.0, +0.0},
			};
			Attrs attrs0 = {
			        {"shader_id",    m_shader.id()    },
                                {"a.a_position", 2                },
                                {"a.a_texcoord", 2                },
			        {"data",         c_vertices.data()},
                                {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs0);
		}

		// c_indices
		uint32_t nelem;
		{
			const std::vector<uint32_t> c_indices = {0, 1, 2, 0, 2, 3, 0, 1, 2, 0, 1, 2, 0, 2, 3};
			spu_array_send(m_arrayId, c_indices.data(), c_indices.size(), -1, sizeof(c_indices[0]));
			nelem = c_indices.size();
		}

		// drawid
		{
			const std::vector<uint32_t> c_draw_id_data = {0, 1, 2};
			Attrs attrs1 = {
			        {"shader_id", m_shader.id()        },
			        {"format",    GL_INT               },
			        {"oformat",   GL_INT               },
			        {"divisor",   1                    },
			        {"a.drawID",  1                    },
			        {"data",      c_draw_id_data.data()},
			        {"nelem",     c_draw_id_data.size()},
			};
			spu_array_aux(m_arrayId, attrs1, 1);
		}

		// command
		{
			const std::vector<DrawElementsIndirectCommand> c_commands = {
			        {nelem,     1, 0, 0, 0},
                                {nelem / 2, 1, 6, 4, 1},
                                {nelem,     1, 9, 7, 2},
			        {nelem,     1, 0, 0, 0},
                                {nelem / 2, 1, 6, 4, 1},
                                {nelem,     1, 9, 7, 2},
			};
			spu_array_send(m_arrayId, c_commands.data(), c_commands.size(), -2);
		}

		// texture
		{
			Attrs attrs = {
			        {"swizzle_r",  GL_RED                 },
                                {"swizzle_g",  GL_GREEN               },
			        {"swizzle_b",  GL_BLUE                },
                                {"swizzle_a",  GL_ALPHA               },
			        {"mag_filter", GL_LINEAR              },
                                {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			};

			u_albedos[0] = loadDDS("kueken7_bgra8_srgb.dds", attrs);
			u_albedos[1] = loadDDS("kueken7_bgra8_srgb.dds", attrs);
			u_albedos[2] = loadDDS("kueken7_bgra8_srgb.dds", attrs);

			Attrs texture_attrs0 = {
			        {"swizzle_g", GL_NONE},
			        {"swizzle_b", GL_NONE},
			};
			Attrs texture_attrs1 = {
			        {"swizzle_b", GL_NONE},
			        {"swizzle_r", GL_NONE},
			};
			Attrs texture_attrs2 = {
			        {"swizzle_r", GL_NONE},
			        {"swizzle_g", GL_NONE},
			};

			spu_texture_set(u_albedos[0], texture_attrs0);
			spu_texture_set(u_albedos[1], texture_attrs1);
			spu_texture_set(u_albedos[2], texture_attrs2);
		}

		m_viewports = makeViewports(3, 1);
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = true /*1*/;
			// renderstate.use();
		}
	}

	void render() override
	{
		uint32_t draw_count[3] = {3, 2, 1};
		uint32_t draw_offsets[3] = {0, 1, 2};

		for (auto i = 0; i < 3; i++) {
			spu_frame_set(-1, "viewport0", m_viewports[i]);

			getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
			auto worldview = getCamera().worldview();

			ub_transform.worldscreen[0]
			        = getCamera().viewscreen() * worldview * Mat4f().trans({0.0, 0.0, +0.5});
			ub_transform.worldscreen[1]
			        = getCamera().viewscreen() * worldview * Mat4f().trans({0.0, 0.0, +0.0});
			ub_transform.worldscreen[2]
			        = getCamera().viewscreen() * worldview * Mat4f().trans({0.0, 0.0, -0.5});

			ub_transform.transform[0] = 0;
			ub_transform.transform[1] = 1;
			ub_transform.transform[2] = 2;

			m_shader.use();

			spu_array_draw(m_arrayId, GL_TRIANGLES, draw_offsets[i], draw_count[i]);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_430_multi_draw_indirect");
}  // namespace
}  // namespace spu
