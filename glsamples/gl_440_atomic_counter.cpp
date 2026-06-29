//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 440 core                                                                      \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = vec4(4*(gl_VertexID%2)-1, 4*(gl_VertexID/2)-1, 0, 1);                \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 440 core                                                                      \n"
    "layout(binding = 0) buffer a_atomic { uint ui; } b_atomic;                             \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    uint counter = atomicAdd(b_atomic.ui, 1);                                          \n"
    "    color = vec4(                                                                      \n"
    "	    float((counter >> 0) & 255) / 255.0,                                              \n"
    "	    float((counter >> 8) & 255) / 255.0,                                              \n"
    "	    float((counter >>16) & 255) / 5.0,                                                \n"
    "	    0.5);                                                                             \n"
    "}                                                                                      \n"
    
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_arrayId;

	SpuShader m_shader;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			{
				Attrs attrs = {
				        {"a.0",   1}, // array buffer
				        {"nelem", 3},
				};
				m_arrayId = spu_array_new(attrs);
			}
			{
				Attrs attrs = {
				        {"a.+0",  0}, // shader storage
				        {"nelem", 4},
				};
				spu_array_aux(m_arrayId, attrs, 1);
			}
		}
	}

	void render() override
	{
		auto *ptr = static_cast<uint32_t *>(
		        spu_array_map(m_arrayId, GL_MAP_WRITE_BIT | GL_MAP_READ_BIT, 1));

		spu_printf(0, "atomic counter = %d\n", *ptr);
		*ptr = 0;
		spu_array_unmap(m_arrayId, 1);

		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_440_atomic_counter");
}  // namespace
}  // namespace spu
