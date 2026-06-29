//
// App :
//
#include "base_app.h"
namespace spu::simpletexture {
/* clang-format off */
const char *vs_source = {
    "#version 420 core                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    const vec4 vertices[] = vec4[](vec4( 0.75, -0.75, 0.5, 1.0),                       \n"
    "                                   vec4(-0.75, -0.75, 0.5, 1.0),                       \n"
    "                                   vec4( 0.75,  0.75, 0.5, 1.0));                      \n"
    "                                                                                       \n"
    "    gl_Position = vertices[gl_VertexID];                                               \n"
    "}                                                                                      \n"
};

const char *fs_source = {
    "#version 430 core                                                                      \n"
    "uniform sampler2D s;                                                                   \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(s, gl_FragCoord.xy / textureSize(s, 0));                           \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	void render() override;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;

private:
	SpuShader m_shader;
	uint32_t u_s;
	SpuArray m_array;
	void generateTexture(Vec4f *data, int32_t width, int32_t height);
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shader & array
	{
		Attrs shader_attrs = {
		        {"frag", fs_source},
		        {"vert", vs_source},
		};
		Attrs unif_attrs = {
		        {"s", &u_s},
		};
		loadShader(m_shader, shader_attrs, unif_attrs);
		m_array.init({
		        {"nelem", 4}
                });
	}
	// texture
	{
		std::vector<Vec4f> data(256 * 256);
		generateTexture(data.data(), 256, 256);
		// const void *pixv[] = { data.data(), 0 };
		Attrs attrs = {
		        {"target",  GL_TEXTURE_2D},
                        {"iformat", GL_RGBA32F   },
                        {"width",   256          },
		        {"height",  256          },
                        {"data",    data.data()  },
		};
		u_s = spu_texture_new(attrs);
	}
	// bg
	{
		spu_frame_set(-1, "bgcolor0", c_green);
	}
}

void App::generateTexture(Vec4f *data, int32_t width, int32_t height)
{
	int32_t x, y;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++, data++) {
			data->r = ((x & y) & 0xFF) / 255.0;
			data->g = ((x | y) & 0xFF) / 255.0;
			data->b = ((x ^ y) & 0xFF) / 255.0;
			data->a = 1.0;
		}
	}
}

void App::render()
{
	m_shader.use();
	m_array.draw(GL_TRIANGLES, 0, 3);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("simpletexture");
}  // namespace spu::simpletexture
