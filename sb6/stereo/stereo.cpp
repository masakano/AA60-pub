//
// App :
//
#include "base_app.h"
namespace spu::stereo {
class App : public BaseApp {
public:
	enum { e_render_full, e_render_right, e_render_left };
	enum { e_object_count = 4 };

	class StereoObject : public sb6::Object {
	public:
		Mat4f model_matrix;
	};

	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;
	void renderScene();

	SpuShader m_shader;
	StereoObject m_objects[e_object_count];
	float m_separation = 2.0;
	int32_t m_mode = 0;

	Mat4f u_viewscreen;
	Mat4f u_modelview;
	Vec4f u_albedo;
	Vec4f u_specular_albedo;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	Attrs unif_attrs = {
	        {"u_viewscreen",    &u_viewscreen     },
	        {"u_modelview",     &u_modelview      },
	        {"diffuse_albedo",  &u_albedo         },
	        {"specular_albedo", &u_specular_albedo},
	};
	loadShader(m_shader, "stereo/stereo-render.us", Attrs(), unif_attrs);

	const char *const object_names[] = {"dragon.sbm", "sphere.sbm", "cube.sbm", "torus.sbm"};

	const char *sym[] = {
	        "a.a_position",
	        "a.a_normal",
	        nullptr,
	};
	for (auto i = 0; i < e_object_count; i++) {
		m_objects[i].load(object_names[i], Attrs(), sym, m_shader.id());
	}
	auto &renderstate = getRenderstate();
	renderstate.flags.depth_test = true;
	renderstate.use();
}

void App::menu()
{
	ImGui::Combo("mode", &m_mode, "full\0right\0left\0\0");
	ImGui::SliderFloat("separation", &m_separation, 0.0, 4.0);
}

void App::render()
{
	auto f = getSeconds().current();
	m_objects[0].model_matrix = c_unit.trans({0.0, -4.0, 0.0}).rot("XY", -20.0, -f * 14.5);
	m_objects[1].model_matrix = c_unit.scale(2.0)
	                                    .trans({sinf(f * 0.37) * 12.0f, cosf(f * 0.37) * 12.0f, 0.0f})
	                                    .rot("Y", -f * 3.7);
	m_objects[2].model_matrix = c_unit.scale(2.0)
	                                    .rot("Z", -f * 99.0)
	                                    .trans({sinf(f * 0.25) * 10.0f, cosf(f * 0.25) * 10.0f, 0.0f})
	                                    .rot("Y", -f * 6.45);
	m_objects[3].model_matrix = c_unit.scale(2.0)
	                                    .rot_axis(radians(f * 120.3), Vec3f(0.707106, 0.0, 0.707106))
	                                    .trans({sinf(f * 0.51) * 14.0f, cosf(f * 0.51) * 14.0f, 0.0f})
	                                    .rot("Y", -f * 5.25);
	auto &renderstate = getRenderstate();
	renderstate.flags.depth_test = true;
	renderstate.use();
	renderScene();
	spu_frame_set(-1, "bgcolor0", c_gray);
}

void App::renderScene()
{
	sb6::Composition composition;
	composition.perspective(viewport(0), 50.0, 1.0, 200.0);
	auto view_position = Vec3f(0.0, 0.0, 40.0);
	sb6::Composition left_composition;
	left_composition.lookat(view_position - Vec3f(m_separation, 0.0, 0.0), Vec3f(0.0, 0.0, -50.0), ey());
	auto modelview_left = left_composition.worldview();
	sb6::Composition right_composition;
	right_composition.lookat(view_position + Vec3f(m_separation, 0.0, 0.0), Vec3f(0.0, 0.0, -50.0), ey());
	auto modelview_right = right_composition.worldview();
	const Vec3f diffuse_colors[] = {
	        {1.0, 0.6, 0.3},
	        {0.2, 0.8, 0.9},
	        {0.3, 0.9, 0.4},
	        {0.5, 0.2, 1.0},
	};
	int32_t chan[2];
	switch (m_mode) {
	case e_render_full:
		chan[0] = 0;
		chan[1] = 1;
		break;
	case e_render_right:
		chan[0] = 1;
		chan[1] = 1;
		break;
	case e_render_left:
		chan[0] = 0;
		chan[1] = 0;
		break;
	default: assert(0);
	}
	auto j = chan[getSeconds().count() & 1];
	u_viewscreen = composition.viewscreen();
	for (auto i = 0; i < 4; i++) {
		u_modelview = (j == 0 ? modelview_left : modelview_right) * m_objects[i].model_matrix;
		u_specular_albedo = Vec4f(0.3, 0.3, 0.3, 1.0);
		u_albedo = diffuse_colors[i];
		m_shader.use();
		m_objects[i].draw();
	}
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("stereo");
}  // namespace spu::stereo
