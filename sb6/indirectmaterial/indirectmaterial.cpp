//
// ArrayCommand :
//
#include "base_app.h"
namespace spu::indirectmaterial {

struct ArrayCommand {
	uint32_t count;
	uint32_t instance_count;
	uint32_t base_vertex;
	uint32_t base_instance;
};

struct MaterialProperties {
	Vec4f ambient;
	Vec4f diffuse;
	Vec4f specular;  // a: specular_power
};

struct FrameUniforms {
	Mat4f worldview;
	Mat4f viewscreen;
	Mat4f worldscreen;
};

/* clang-format off */
const char *preface = {
    "#version 440 core                                                                      \n"
    "#extension GL_ARB_shader_draw_parameters : require                                     \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;
	void loadShaders();

private:
	static constexpr int32_t c_model_matrix_slot = 4;
	enum { e_num_materials = 100, e_num_draws = 16384 };
	SpuShader m_shader;
	FrameUniforms u_frame_data;
	MaterialProperties u_materials[e_num_materials];
	sb6::Object m_object;
	int32_t m_frameCount;
	int32_t m_drawsPerFrame;
	float m_drawsPerFrameF = 4.0f;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	loadShaders();

	{
		const char *sym[] = {
		        "a.a_position",
		        "a.a_normal",
		        nullptr,
		};
		m_object.load("asteroids.sbm", Attrs(), sym, m_shader.id());
	}

	auto array_id = m_object.getArray().id();
	std::vector<ArrayCommand> coms(e_num_draws);
	auto num_objects = m_object.modelCount();

	for (auto i = 0; i < e_num_draws; i++) {
		m_object.getModelInfo(i % num_objects, coms[i].base_vertex, coms[i].count);
		coms[i].instance_count = 1;
		coms[i].base_instance = i % e_num_materials;
	}
	spu_array_send(array_id, coms.data(), coms.size(), -2);
	Attrs array_attrs = {
	        {"a.+0",  0	                  },
	        {"nelem", e_num_draws * sizeof(Mat4f)},
	};
	spu_array_aux(array_id, array_attrs, c_model_matrix_slot);
	for (auto i = 0; i < e_num_materials; i++) {
		auto f = float(i) / float(e_num_materials);
		u_materials[i].ambient
		        = {(sinf(f * 3.7) + 1.0f) * 0.1f, (sinf(f * 5.7 + 3.0f) + 1.0f) * 0.1f,
		           (sinf(f * 4.3 + 2.0f) + 2.0f) * 0.1f, 0.1};

		u_materials[i].diffuse
		        = {(sinf(f * 9.9 + 6.0) + 1.0f) * 0.4f, (sinf(f * 3.1 + 2.5) + 2.0f) * 0.4f,
		           (sinf(f * 7.2 + 9.0) + 2.0f) * 0.4f, 0.4};
		u_materials[i].specular = {
		        (sinf(f * 1.6 + 4.0) + 19.0f) * 0.6f,
		        (sinf(f * 0.8 + 2.7) + 19.0f) * 0.6f,
		        (sinf(f * 5.2 + 8.0) + 19.0f) * 0.6f,
		        1.0,
		};
		u_materials[i].specular.a = 200.0 + sinf(f) * 50.0;
	}
	auto &renderstate = getRenderstate();
	renderstate.flags.depth_test = true;
	renderstate.flags.cull_face = true;
	renderstate.depth_func = GL_LEQUAL;
	// renderstate.use();
}

void App::render()
{
	auto t = getSeconds().current();
	sb6::Composition composition;
	composition.lookat(
	        Vec3f(30.0 * cosf(t * 0.023), 30.0 * cosf(t * 0.023), 30.0 * sinf(t * 0.037) - 200.0), ezero(),
	        normalize(Vec3f(0.1 - cosf(t * 0.1) * 0.3, 1.0, 0.0)));
	composition.perspective(viewport(0), 50.0, 1.0, 2000.0);
	u_frame_data.worldview = composition.worldview();
	u_frame_data.viewscreen = composition.viewscreen();
	u_frame_data.worldscreen = u_frame_data.viewscreen * u_frame_data.worldview;
	m_shader.use();
	auto array_id = m_object.getArray().id();
	auto *matrices = static_cast<Mat4f *>(spu_array_map(array_id, GL_MAP_WRITE_BIT, c_model_matrix_slot));
	m_drawsPerFrame = int(m_drawsPerFrameF) * 512;
	auto f = t * 0.1f;
	for (auto i = 0; i < m_drawsPerFrame; i++) {
		auto m = c_unit.trans(
		                 {sinf(f * 7.3) * 70.0f, sinf(f * 3.7 + 2.0) * 70.0f,
		                  sinf(f * 2.9 + 8.0) * 70.0f})
		       * c_unit.rot("xyz", radians(f * 330.0), radians(f * 490.0), radians(f * 250.0));
		matrices[i] = m;
		f += 3.1;
	}
	spu_array_unmap(array_id, c_model_matrix_slot);
	spu_array_draw(array_id, GL_TRIANGLES, 0, m_drawsPerFrame);
	m_frameCount++;
}

void App::loadShaders()
{
	Attrs shader_attrs = {
	        {"preface", preface},
	};
	Attrs unif_attrs = {
	        {"FRAME_DATA", &u_frame_data  },
	        {"MATERIALS",  &u_materials[0]},
	};
	loadShader(m_shader, "indirectmaterial/render.us", shader_attrs, unif_attrs);
}

void App::menu() { ImGui::SliderFloat("draw per frame", &m_drawsPerFrameF, 1, e_num_draws / 512); }

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("indirectmaterial");
}  // namespace spu::indirectmaterial
