//
// CandidateDraw :
//
#include "base_app.h"
namespace spu::cullindirect {

struct CandidateDraw {
	Vec4f sphere_center;
	float sphere_radius;
	uint32_t first;
	uint32_t count;
	uint32_t: 32;
};

struct ArrayCommand {
	uint32_t count;
	uint32_t instance_count;
	uint32_t base_vertex;
	uint32_t base_instance;
};

struct TransformBuffer {
	Mat4f modelview;
	Mat4f viewscreen;
	Mat4f modelscreen;
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
	void render() override;
	// void menu() { m_isMenu = false;} // need fix

	static constexpr int32_t def_local_size = 16;
	static constexpr int32_t c_candidate_count = 1024;

	sb6::Object m_object;
	SpuShader m_cullShader;
	SpuShader m_drawShader;
	SpuArray m_array;
	float m_fps = 0;
	TransformBuffer u_transforms;
	Mat4f u_matrices[c_candidate_count];
	uint32_t u_texture = 0;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	uint32_t first, count;
	// shaders
	{
		{
			Attrs shader_attrs = {
			        {"def_local_size", def_local_size},
			};
			Attrs unif_attrs = {
			        {"MODEL_MATRIX_BLOCK", &u_matrices[0]},
			        {"TRANSFORM_BLOCK",    &u_transforms },
			};
			loadShader(m_cullShader, "cullindirect/cull.us", shader_attrs, unif_attrs);
		}

		{
			Attrs shader_attrs = {
			        {"preface", preface},
			};

			Attrs unif_attrs = {
			        {"u_texture", &u_texture},
			};
			loadShader(m_drawShader, "cullindirect/render.us", shader_attrs, unif_attrs);
		}
		// link
		{
			const char *unif_names[] = {
			        "MODEL_MATRIX_BLOCK",
			        "TRANSFORM_BLOCK",
			        0,
			};
			uint32_t unif_types[2] = {0, 0};  // ubo
			spu_shader_loc(m_cullShader.id(), unif_names, 0, 0, unif_types, 2);

			Attrs set_attrs = {
			        {"MODEL_MATRIX_BLOCK.buffer_id", &unif_types[0]},
			        {"TRANSFORM_BLOCK.buffer_id",    &unif_types[1]},
			};
			spu_shader_set(m_drawShader.id(), set_attrs);
		}
	}
	// compute array
	// uint32_t parameter_id;
	uint32_t command_id;
	{
		Attrs attrs1 = {
		        {"a.+0",  0  },
		        {"nelem", 256},
		};
		Attrs attrs2 = {
		        {"a.+1",  0		                        }, // shader storage
		        {"nelem", c_candidate_count * sizeof(CandidateDraw)},
		};
		Attrs attrs3 = {
		        {"a.+2",  0		                       }, // shader storage
		        {"nelem", c_candidate_count * sizeof(ArrayCommand)},
		};
		m_array.init(Attrs());
		m_array.aux(attrs1, 1);
		m_array.aux(attrs2, 2);
		m_array.aux(attrs3, 3);
		m_array.get("3.buffer_id", &command_id);
	}
	// draw
	{
		Attrs attrs = {
		        {"command_id", command_id},
		};
		const char *syms[] = {
		        "a.a_position",
		        "a.a_normal",
		        nullptr,
		};
		m_object.load("asteroids.sbm", attrs, syms, m_drawShader.id());
	}
	// set object  data to compute m_shader
	{
		std::vector<CandidateDraw> cdraws(c_candidate_count);
		for (auto i = 0; i < c_candidate_count; i++) {
			m_object.getModelInfo(i % m_object.modelCount(), first, count);
			cdraws[i].sphere_center = Vec4f(0, 0, 0, 1);
			cdraws[i].sphere_radius = 4.0;
			cdraws[i].first = first;
			cdraws[i].count = count;
		}
		m_array.send(cdraws.data(), c_candidate_count * sizeof(CandidateDraw), 2);
	}
	u_texture = sb6::ktx::load("rocks.ktx");

	{
		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;
	}
}

void App::render()
{
	auto t = getSeconds().current();
	static uint8_t zeros[256];
	// compute
	{
		m_array.send(&zeros, 256, 1);
		for (auto i = 0; i < c_candidate_count; i++) {
			auto f = i / 127.0f + t * 0.025f;
			auto g = i / 127.0f;
			auto axis = Vec3f(sinf(g * 35.0), cosf(g * 75.0), cosf(g * 39.0));
			auto trans = Vec3f(sinf(f * 3.0), cosf(f * 5.0), cosf(f * 9.0));

			u_matrices[i] = c_unit.rot_axis(radians(t * 140.0), axis).trans({70.0 * trans});
		}
		sb6::Composition composition;
		composition.lookat(Vec3f(150.0 * cosf(t * 0.1), 0.0, 150.0 * sinf(t * 0.1)), ezero(), ey());
		composition.perspective(viewport(0), 50.0, 1.0, 2000.0);
		auto modelview = composition.worldview();
		auto viewscreen = composition.viewscreen();
		u_transforms.modelview = modelview;
		u_transforms.viewscreen = viewscreen;
		u_transforms.modelscreen = viewscreen * modelview;
		m_cullShader.use();

		std::vector<ArrayCommand> commands(c_candidate_count, {0, 0, 0, 0});
		m_object.getArray().send(commands, -2);
		m_array.draw(0xffff, c_candidate_count / def_local_size, 1, 1);
	}
	// draw
	{
		spu_graphics_memory_barrier(GL_COMMAND_BARRIER_BIT);
		m_drawShader.use();
#if 0
		{
			std::vector<ArrayCommand> commands(c_candidate_count);
			m_object.getArray().recv(commands.data(), commands.size(), -2);
			printf("command:\n");
			for (auto &c: commands) {
				printf("\t%5d %5d %5d %5d\n", c.count, c.instance_count, c.base_vertex,
				       c.base_instance);
				if (c.count == 0) break;
			}
			printf("\n");
		}
#endif
		m_object.getArray().draw(GL_TRIANGLES, 0, c_candidate_count);
	}
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("cullindirect");
}  // namespace spu::cullindirect
