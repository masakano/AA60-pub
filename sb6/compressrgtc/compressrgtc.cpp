//
// App :
//
#include <unistd.h>
#include "base_app.h"
namespace spu::compressrgtc {
class App : public BaseApp {
public:
	static constexpr auto c_side = 512;

	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;

private:
	SpuShader m_compressShader;
	SpuShader m_renderShader;
	SpuArray m_compute;
	uint32_t m_outputTexture;
	uint32_t m_outputBuffer;

	uint32_t u_input_image;
	uint32_t u_output_buffer;
	uint32_t u_tex;
	void loadShaders();
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// input tex
	u_input_image = spu_inventory_new("texture", "mandril.jpg", Attrs());

	// output texbuffer
	{
		Attrs attrs = {
		        {"target",  GL_TEXTURE_BUFFER  },
		        {"iformat", GL_RG32UI          },
		        {"size",    c_side * c_side / 2},
		};
		u_output_buffer = spu_texture_new(attrs);
		spu_texture_get(u_output_buffer, "buffer_id", &m_outputBuffer);
	}
	// draw texture
	{
		Attrs attrs = {
		        {"target",     GL_TEXTURE_2D          },
		        {"iformat",    GL_COMPRESSED_RED_RGTC1},
		        {"width",      c_side                 },
		        {"height",     c_side                 },
		        {"min_filter", GL_NEAREST             },
		        {"mag_filter", GL_NEAREST             },
		        {"buffer_id",  m_outputBuffer         },
		};
		m_outputTexture = spu_texture_new(attrs);
	}
	loadShaders();
	m_compute.init(Attrs());
}

void App::render()
{
	// auto &viewport = getViewports().at(0);
	// auto &viewport = viewport(0);
	auto org_viewport = viewport(0);
	auto ox = (viewport(0).sx / 2 - c_side) / 2;
	auto oy = (viewport(0).sy - c_side) / 2;

	// compress
	{
		m_compressShader.use();
		m_compute.draw(0xffff, c_side / 4, c_side / 4, 1);
		spu_graphics_memory_barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		spu_texture_copy(m_outputTexture, u_output_buffer);
	}

	{
		auto new_viewport = Rectf(ox, oy, c_side, c_side);
		spu_frame_set(-1, "viewport0", new_viewport);
		// BaseApp::sync(0);

		u_tex = u_input_image;
		m_renderShader.use();
		drawFullscreenQuad();
	}
	{
		auto new_viewport = Rectf(ox * 2 + c_side, oy, c_side, c_side);
		// BaseApp::sync(0);
		spu_frame_set(-1, "viewport0", new_viewport);

		u_tex = m_outputTexture;
		m_renderShader.use();
		drawFullscreenQuad();
	}
	spu_frame_set(-1, "viewport0", org_viewport);
	// viewport = viewport_save;
	// BaseApp::sync(0);
}

void App::loadShaders()
{
	{
		Attrs shader_attrs = {
		        {"def_local_size", 1},
		};
		Attrs unif_attrs = {
		        {"input_image",   &u_input_image  },
		        {"output_buffer", &u_output_buffer},
		};
		loadShader(m_compressShader, "compressrgtc/rgtccompress.us", shader_attrs, unif_attrs);
	}
	{
		Attrs unif_attrs = {
		        {"tex", &u_tex},
		};
		loadShader(m_renderShader, "compressrgtc/drawquad.us", Attrs(), unif_attrs);
	}
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("compressrgtc");
}  // namespace spu::compressrgtc
