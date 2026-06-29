//
// App :
//
#include "base_app.h"
namespace spu::clipdistance {
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;

private:
	sb6::Object m_object;
	SpuShader m_shader;
	Mat4f u_viewscreen;
	Mat4f u_modelview;
	Vec4f u_clip_plane;
	Vec4f u_clip_sphere;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	Attrs unif_attrs = {
	        {"u_viewscreen",  &u_viewscreen },
	        {"u_modelview",   &u_modelview  },
	        {"u_clip_plane",  &u_clip_plane },
	        {"u_clip_sphere", &u_clip_sphere},
	};
	loadShader(m_shader, "clipdistance/render.us", Attrs(), unif_attrs);

	const char *sym[] = {
	        "a.a_position",
	        "a.a_normal",
	        nullptr,
	};
	m_object.load("dragon.sbm", Attrs(), sym, m_shader.id());
}

void App::render()
{
	// uniform
	{
		auto f = getSeconds().current();
		auto plane_matrix = c_unit.rot("YX", -f * 7.3, -f * 6.0);

		sb6::Composition composition;
		composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
		u_viewscreen = composition.viewscreen();
		u_modelview = c_unit.trans({0.0, -4.0, 0.0}).rot("Y", -f * 0.34).trans({0.0, 0.0, -15.0});
		u_clip_plane = plane_matrix.c[0];
		u_clip_plane.f[3] = 0.0;
		u_clip_plane = normalize(u_clip_plane);
		u_clip_sphere = {
		        sinf(f * 0.7) * 3.0f, cosf(f * 1.9) * 3.0f, sinf(f * 0.1) * 3.0f, cosf(f * 1.7) + 2.5f};
		m_shader.use();
	}
	// renderstate
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.clip_distance0 = true;
		renderstate.flags.clip_distance1 = true;
		renderstate.use();
	}
	m_object.draw();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("clipdistance");
}  // namespace spu::clipdistance
