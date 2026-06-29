//
// App :
//
#include "base_app.h"
namespace spu::multimaterial {
class App : public BaseApp {
public:
	enum { e_num_cubes = 256 };

	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;

private:
	SpuShader m_shader;
	SpuArray m_array;
	bool m_isTriangle = 1;
	struct {
		Mat4f u_modelview;
		Mat4f u_viewscreen;
	} ub_transforms[e_num_cubes];
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	{
		Attrs unif_attrs = {
		        {"TRANSFORM_BLOCK", ub_transforms},
		};
		loadShader(m_shader, "multimaterial/ubo-plus-base-instance.us", Attrs(), unif_attrs);
	}
	{
		const float c_vertex_positions[]
		        = {-0.25, +0.25, -0.25, -0.25, -0.25, -0.25, +0.25, -0.25, -0.25, +0.25, -0.25, -0.25,
		           +0.25, +0.25, -0.25, -0.25, +0.25, -0.25, +0.25, -0.25, -0.25, +0.25, -0.25, +0.25,
		           +0.25, +0.25, -0.25, +0.25, -0.25, +0.25, +0.25, +0.25, +0.25, +0.25, +0.25, -0.25,
		           +0.25, -0.25, +0.25, -0.25, -0.25, +0.25, +0.25, +0.25, +0.25, -0.25, -0.25, +0.25,
		           -0.25, +0.25, +0.25, +0.25, +0.25, +0.25, -0.25, -0.25, +0.25, -0.25, -0.25, -0.25,
		           -0.25, +0.25, +0.25, -0.25, -0.25, -0.25, -0.25, +0.25, -0.25, -0.25, +0.25, +0.25,
		           -0.25, -0.25, +0.25, +0.25, -0.25, +0.25, +0.25, -0.25, -0.25, +0.25, -0.25, -0.25,
		           -0.25, -0.25, -0.25, -0.25, -0.25, +0.25, -0.25, +0.25, -0.25, +0.25, +0.25, -0.25,
		           +0.25, +0.25, +0.25, +0.25, +0.25, +0.25, -0.25, +0.25, +0.25, -0.25, +0.25, -0.25};

		Attrs attrs = {
		        {"shader_id",    m_shader.id()},
		        {"a.a_position", 3            },
		};
		Attrs aux_attrs = {
		        {"a.a_instance_id", 1     },
		        {"format",          GL_INT},
		        {"oformat",         GL_INT},
		        {"divisor",         1     },
		};
		int32_t aux_dummy[e_num_cubes];
		for (auto i = 0; i < e_num_cubes; i++)
			aux_dummy[i] = i;
		m_array.init(attrs);
		m_array.aux(aux_attrs, 1);
		m_array.send(c_vertex_positions, 36);
		m_array.send(aux_dummy, e_num_cubes, 1);
	}
	struct DrawIndirectCmd  // use different structure for indexed object!
	{
		uint32_t count;
		uint32_t prim_count;
		uint32_t first;
		uint32_t base_instance;
	};
	DrawIndirectCmd ind[e_num_cubes];
	for (auto i = 0; i < e_num_cubes; i++) {
		ind[i].count = 36;
		ind[i].prim_count = e_num_cubes;
		ind[i].first = 0;
		ind[i].base_instance = i;  // does not work in old machine...
	}
	m_array.send(ind, 1, -2);
	auto &renderstate = getRenderstate();
	renderstate.flags.cull_face = true;
	renderstate.flags.ccw = false;
	renderstate.flags.depth_test = true;
	renderstate.depth_func = GL_LEQUAL;

	const Vec4f c_purple = {0.25, 0.0, 0.25, 1.0};
	spu_frame_set(-1, "bgcolor0", c_purple);
}

void App::menu() { ImGui::Checkbox("draw triangle", &m_isTriangle); }

void App::render()
{
	auto t = getSeconds().current();
	auto f = t * 0.3f;
	for (auto i = 0; i < e_num_cubes; i++) {
		float fi = 4.0 * i / float(e_num_cubes);
		sb6::Composition composition;
		composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
		ub_transforms[i].u_viewscreen = composition.viewscreen();
		ub_transforms[i].u_modelview
		        = c_unit.rot("XY", -f * 81.0 + fi, -f * 45.0 + fi)
		                  .trans({sinf(5.1 * f + fi) * 1.0f, cosf(7.7 * f + fi) * 1.0f,
		                          sinf(6.3 * f + fi) * cosf(1.5 * f + fi) * 2.0f})
		                  .trans({0.0, 0.0, -4.0});
	}
	m_shader.use();
	m_array.draw(m_isTriangle ? GL_TRIANGLES : GL_LINES);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("multimaterial");
}  // namespace spu::multimaterial
