//
// App :
//
#include "base_app.h"
#include <omp.h>
namespace spu::pmbfractal {
class App : public BaseApp {
public:
	enum {
		e_fractal_width = 256,
		e_fractal_height = 256,
		e_buffer_size = (e_fractal_width * e_fractal_height)
	};
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;
	void updateOverlay();
	void updateFractal();
	SpuShader m_shader;
	SpuTexture m_texture;
	uint32_t u_texture = 0;

	std::vector<uint8_t> m_buffer;
	struct {
		Vec2f c;
		Vec2f offset;
		float zoom;
	} m_fractparams;
	float m_cputime = 0;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	m_buffer.resize(e_fractal_width * e_fractal_height, 0);
	m_shader.init("pmbfractal/fsq.us");

	m_shader.addUniform("tex", &u_texture);
	Attrs tex_attrs = {
	        {"target",      GL_TEXTURE_2D   },
                {"iformat",     GL_R8           },
                {"width",       e_fractal_width },
	        {"height",      e_fractal_height},
                {"mag_filter",  GL_LINEAR       },
                {"max_level",   0               },
	        {"auto_mipmap", 0               },
	};
	m_texture.init(tex_attrs);
	u_texture = m_texture.id();
	int32_t max_threads = omp_get_max_threads();
	omp_set_num_threads(max_threads);
}

void App::updateFractal()
{
	const auto c = m_fractparams.c;  // (0.03f, -0.2f);
	const auto thresh_squared = 256.0f;
	const auto zoom = m_fractparams.zoom;
	const auto offset = vec2f_t(m_fractparams.offset);
	for (auto y = 0; y < e_fractal_height; y++) {
		for (auto x = 0; x < e_fractal_width; x++) {
			Vec2f z;
			z.f[0] = zoom * (float(x) / float(e_fractal_width) - 0.5f) + offset.f[0];
			z.f[1] = zoom * (float(y) / float(e_fractal_height) - 0.5f) + offset.f[1];
			auto *ptr = m_buffer.data() + y * e_fractal_width + x;
			auto it = 0;
			for (it = 0; it < 256; it++) {
				Vec2f z_squared;
				z_squared.f[0] = z.f[0] * z.f[0] - z.f[1] * z.f[1];
				z_squared.f[1] = 2.0f * z.f[0] * z.f[1];
				z = z_squared + c;
				if ((z.f[0] * z.f[0] + z.f[1] * z.f[1]) > thresh_squared) break;
			}
			*ptr = it;
		}
	}
}

void App::render()
{
	auto usec0 = get_microsec();

	double now_time = getSeconds().current();
	m_fractparams.c
	        = Vec2f(1.5f - cosf(now_time * 0.4f) * 0.5f, 1.5f + cosf(now_time * 0.5f) * 0.5f) * 0.3f;
	m_fractparams.offset = Vec2f(cosf(now_time * 0.14f), cosf(now_time * 0.25f)) * 0.25f;
	m_fractparams.zoom = (sinf(now_time) + 1.3f) * 0.7f;
	updateFractal();
	m_texture.send(m_buffer.data(), GL_R8);
	m_shader.use();
	drawFullscreenQuad();
	m_cputime = get_microsec() - usec0;
	spu_printf(0, "cpu time %f\n", m_cputime);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("pmbfractal");
}  // namespace spu::pmbfractal
