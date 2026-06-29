//
// App :
//
#include "base_app.h"
namespace spu::cubicbezier {
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;
	SpuArray m_array;
	SpuShader m_tessShader;
	SpuShader m_cpShader;
	Mat4f u_viewscreen;
	Mat4f u_modelview;
	Mat4f u_modelscreen;
	Vec4f u_draw_color;

	bool m_isShowPoints = 1;
	bool m_isShowCage = 1;
	bool m_isWireframe = 0;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// tess shader
	{
		Attrs unif_attrs = {
		        {"u_modelview",   &u_modelview  },
		        {"u_viewscreen",  &u_viewscreen },
		        {"u_modelscreen", &u_modelscreen},
		};
		loadShader(m_tessShader, "cubicbezier/cubicbezier.us", Attrs(), unif_attrs);
	}
	// control point32_t shader
	{
		Attrs unif_attrs = {
		        {"u_modelscreen", &u_modelscreen},
		        {"u_draw_color",  &u_draw_color },
		};
		loadShader(m_cpShader, "cubicbezier/draw-control-points.us", Attrs(), unif_attrs);
	}
	// array
	{
		const std::vector<uint16_t> c_indices
		        = {0, 1, 1, 2, 2, 3,  4, 5, 5, 6, 6, 7,  8, 9, 9, 10, 10, 11, 12, 13, 13, 14, 14, 15,
		           0, 4, 4, 8, 8, 12, 1, 5, 5, 9, 9, 13, 2, 6, 6, 10, 10, 14, 3,  7,  7,  11, 11, 15};
		Attrs attrs = {
		        {"shader_id",    m_tessShader.id()},
		        {"a.a_position", 3                },
		};
		m_array.init(attrs);
		m_array.send(c_indices, -1, sizeof(c_indices[0]));
	}
	// environment
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = true;
		spu_frame_set(-1, "bgcolor0", c_gray);
	}
}

void App::menu()
{
	ImGui::Checkbox("show cage", &m_isShowCage);
	ImGui::Checkbox("show points", &m_isShowPoints);
	ImGui::Checkbox("wireframe", &m_isWireframe);
}

void App::render()
{
	auto t = getSeconds().current();  // sec
	// patch
	{
		std::vector<vec3f_t> patch_initializer = {
		        {-1.00, -1.00, 0.0},
                        {-0.33, -1.00, 0.0},
                        {0.33,  -1.00, 0.0},
                        {1.00,  -1.00, 0.0},
		        {-1.0,  -0.33, 0.0},
                        {-0.33, -0.33, 0.0},
                        {0.33,  -0.33, 0.0},
                        {1.0,   -0.33, 0.0},
		        {-1.0,  0.33,  0.0},
                        {-0.33, 0.33,  0.0},
                        {0.33,  0.33,  0.0},
                        {1.0,   0.33,  0.0},
		        {-1.0,  1.0,   0.0},
                        {-0.33, 1.0,   0.0},
                        {0.33,  1.0,   0.0},
                        {1.00,  1.0,   0.0},
		};
		for (auto &data: patch_initializer) {
			auto i = &data - &patch_initializer[0];
			auto fi = i / 16.0f;
			data.f[2] = sinf(t * (0.2 + fi * 0.3));
		}
		m_array.send(patch_initializer);
	}
	sb6::Composition composition;
	composition.perspective(viewport(0), 50.0, 1.0, 1000.0);
	u_viewscreen = composition.viewscreen();
	u_modelview = c_unit.rot("XY", -t * 17.0, -t * 10.0).trans({0.0, 0.0, -4.0});
	u_modelscreen = u_viewscreen * u_modelview;
	// patch
	{
		auto &renderstate = getRenderstate();
		if (m_isWireframe) {
			renderstate.flags.fill = false;
		}
		else {
			renderstate.flags.fill = true;
		}
		renderstate.use();

		m_tessShader.use();
		m_array.set("patch_vertices", 16);
		m_array.draw(GL_PATCHES, 0, 16, 1, GL_ARRAY_BUFFER);
	}
	if (m_isShowPoints) {
		auto &renderstate = getRenderstate();
		renderstate.point_size = 9.0;
		renderstate.use();
		u_draw_color = {0.2, 0.7, 0.9, 1.0};
		m_cpShader.use();
		m_array.draw(GL_POINTS, 0, 16, 1, GL_ARRAY_BUFFER);
	}
	if (m_isShowCage) {
		u_draw_color = {0.7, 0.9, 0.2, 1.0};
		m_cpShader.use();
		m_array.draw(GL_LINES, 0, 48);
	}
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("cubicbezier");
}  // namespace spu::cubicbezier
