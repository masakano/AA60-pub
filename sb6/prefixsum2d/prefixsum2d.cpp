//
// App :
//
#include "base_app.h"
namespace spu::prefixsum2d {
class App : public BaseApp {
public:
	enum { e_num_elements = 2048 };
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;

	SpuShader m_sumShader;
	SpuShader m_showShader;
	SpuArray m_array;
	uint32_t u_images[3];
	uint32_t u_input_image;
	uint32_t u_output_image;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	u_images[0] = sb6::ktx::load("salad-gray.ktx");

	for (auto i = 1; i < 3; i++) {
		Attrs attrs = {
		        {"target",      GL_TEXTURE_2D },
                        {"iformat",     GL_R32F       },
                        {"width",       e_num_elements},
		        {"height",      e_num_elements},
                        {"max_level",   0             },
                        {"auto_mipmap", 0             },
		};
		float color[4] = {1, 1, 1, 1};
		u_images[i] = spu_texture_new(attrs);
		spu_texture_send(u_images[i], color, GL_R32F, 0, 0, 1);
	}
	// common dummy array
	{
		m_array.init({
		        {"nelem", 4}
                });
	}
	// shader
	{
		Attrs shader_attrs = {
		        {"def_local_size", 1024},
		};
		Attrs unif_attrs = {
		        {"input_image",  &u_input_image },
		        {"output_image", &u_output_image},
		};
		loadShader(m_sumShader, "prefixsum2d/prefixsum2d.us", shader_attrs, unif_attrs);
	}
	{
		Attrs unif_attrs = {
		        {"input_image", &u_images[2]},
		};
		loadShader(m_showShader, "prefixsum2d/showimage.us", Attrs(), unif_attrs);
	}
}

void App::render()
{
	u_input_image = u_images[0];
	u_output_image = u_images[1];
	m_sumShader.use();
	m_array.draw(0xffff, e_num_elements, 1, 1);
	spu_graphics_memory_barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	u_input_image = u_images[1];
	u_output_image = u_images[2];
	m_sumShader.use();
	m_array.draw(0xffff, e_num_elements, 1, 1);
	spu_graphics_memory_barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	m_showShader.use();
	m_array.draw(GL_TRIANGLE_STRIP, 0, 4);
	// spu_texture_save(u_images[2], "temp.bmp", GL_RGBA8);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("prefixsum2d");
}  // namespace spu::prefixsum2d
