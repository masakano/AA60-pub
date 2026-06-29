//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 330											\n"
    "void main()											\n"
    "{													\n"
    "    gl_Position = vec4(4.0 * (gl_VertexID % 2) - 1.0, 4.0 * (gl_VertexID / 2) - 1.0, 0.0, 1.0);	\n"
    "}													\n"
};

const char *c_frag = {
    "#version 330								\n"
    "uniform sampler2D u_diffuse;						\n"
    "uniform vec2 u_winsize;							\n"
    "out vec4 color;								\n"
    "void main()								\n"
    "{										\n"
    "    color = texture(							\n"
    "                u_diffuse,							\n"
    "                vec2(gl_FragCoord.x, 1.0 - gl_FragCoord.y) / u_winsize);	\n"
    "}										\n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	uint32_t m_arrayId;
	uint32_t u_diffuse;
	Vec2f u_winsize;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// shader
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_diffuse", &u_diffuse},
			        {"u_winsize", &u_winsize},
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// texture
		{
			Attrs attrs = {
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter", GL_LINEAR              },
			};
			u_diffuse = loadDDS("kueken7_bgra8_srgb.dds", attrs);
		}
		// array
		{
			Attrs attrs = {
			        {"nelem", 3},
			};
			m_arrayId = spu_array_new(attrs);
		}
	}

	void render() override
	{
		u_winsize.x = viewport(0).sx;
		u_winsize.y = viewport(0).sy;
		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_draw_image_space");
}  // namespace
}  // namespace spu
