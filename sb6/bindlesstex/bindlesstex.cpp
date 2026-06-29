//
// App :
//
#include "base_app.h"
namespace spu::bindlesstex {
/* clang-format off */
const char *preface = {
    "#version 440 core                                                                      \n"
    "#extension GL_ARB_shader_draw_parameters : require                                     \n"
    "#extension GL_ARB_bindless_texture : require                                           \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;
	enum { e_num_textures = 384, e_texture_levels = 5, e_texture_size = (1 << (e_texture_levels - 1)) };
	struct UBMatrices {
		Mat4f view;
		Mat4f projection;
		Mat4f model[e_num_textures];
	};
	SpuShader m_shader;
	SpuTexture m_textures[e_num_textures];  // normal handle
	UBMatrices ub_matrices;
	uint64_t ub_textures[e_num_textures * 2];  // bindless handle
	sb6::Object m_object;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// texture
	{
		std::vector<uint32_t> tex_data(32 * 32, 0);
		std::vector<uint32_t> mutated_data(32 * 32, 0);
		for (auto i = 0; i < 32; i++) {
			for (auto j = 0; j < 32; j++) {
				union RGBA8 {
					struct {
						uint8_t r, g, b, a;
					};
					uint32_t ui;
				} rgba;
				rgba.r = (i ^ j) << 3;
				rgba.g = (i ^ j) << 3;
				rgba.b = (i ^ j) << 3;
				rgba.a = 0;
				tex_data[i * 32 + j] = rgba.ui;
			}
		}
		for (auto i = 0; i < e_num_textures; i++) {
			auto r = rand();
			for (auto j = 0; j < 32 * 32; j++) {
				mutated_data[j] = (tex_data[j] & r) | 0x20202020;
			}
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D      },
                                {"iformat",     GL_RGBA8           },
			        {"width",       e_texture_size     },
                                {"height",      e_texture_size     },
			        {"data",        mutated_data.data()},
                                {"auto_mipmap", 1                  },
			};
			m_textures[i].init(attrs);
			m_textures[i].get("bindless_id", &ub_textures[i * 2]);
		}
	}
	// shader
	{
		Attrs shader_attrs = {
		        {"preface", preface},
		};
		Attrs unif_attrs = {
		        {"MATRIX_BLOCK",  &ub_matrices   },
		        {"TEXTURE_BLOCK", &ub_textures[0]},
		};
		loadShader(m_shader, "bindlesstex/render.us", shader_attrs, unif_attrs);
	}
	// mode
	{
		const char *sym[] = {
		        "a.a_position", "a.a_normal", "a.a_tangent", "a.a_bitangent", "a.a_texcoord", nullptr,
		};
		m_object.load("torus_nrms_tc.sbm", Attrs(), sym, m_shader.id());
	}
	// renderstate
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = true;
	}
}

void App::render()
{
	auto f = getSeconds().current();
	sb6::Composition composition;
	composition.perspective(viewport(0), 70.0, 0.1, 500.0);
	auto u_viewscreen = composition.viewscreen();
	auto angle = f;
	auto angle2 = 0.7 * f;
	auto angle3 = 0.1 * f;
	ub_matrices.view = c_unit.trans({0.0, 0.0, -80.0});
	ub_matrices.projection = u_viewscreen;
	for (auto i = 0; i < e_num_textures; i++) {
		ub_matrices.model[i] = c_unit.trans(
		                               {float(i % 32) * 4.0f - 62.0f, float(i >> 5) * 6.0f - 33.0f,
		                                15.0f * sinf(angle * 0.19) + 3.0f * cosf(angle2 * 6.26)
		                                        + 40.0f * sinf(angle3)})
		                     * c_unit.rot("x", radians(-angle) * 130.0)
		                     * c_unit.rot("z", radians(-angle) * 140.0);

		angle += 1.0;
		angle2 += 4.1;
		angle3 += 0.01;
	}
	m_shader.use();
	m_object.draw(e_num_textures);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("bindlesstex");
}  // namespace spu::bindlesstex
