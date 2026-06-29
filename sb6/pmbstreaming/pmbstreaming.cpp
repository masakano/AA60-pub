//
// App :
//
#include "base_app.h"
namespace spu::pmbstreaming {
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
	void shutdown();

protected:
	static constexpr auto c_count = 16;
	struct MATRICES {
		Mat4f modelview;
		Mat4f viewscreen;
	};
	MATRICES u_constants[c_count];
	uint32_t u_tex;
	sb6::Object m_object;
	SpuShader m_shader;
	std::vector<Vec3f> m_offsets;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	std::string count_str = std::to_string(c_count);
	Attrs shader_attrs = {
	        {"preface",   preface  },
	        {"def_count", count_str},
	};
	Attrs unif_attrs = {
	        {"constants", &u_constants[0]},
	        {"tex",       &u_tex         },
	};
	loadShader(m_shader, "pmbstreaming/pmbstreaming.us", shader_attrs, unif_attrs);

	const char *sym[] = {
	        "a.a_position",  "a.a_normal",   "a.a_tangent",
	        "a.a_bitangent", "a.a_texcoord", /*"a.a_draw_id",*/ nullptr,
	};

	m_object.load("torus_nrms_tc.sbm", Attrs(), sym, m_shader.id());

	u_tex = sb6::ktx::load("pattern1.ktx");
	auto &renderstate = getRenderstate();
	renderstate.flags.depth_test = true;
	renderstate.depth_func = GL_LESS;
	// renderstate.use();
	//  draw ID
	{
		std::vector<int32_t> data;
		for (auto i = 0; i < c_count; i++) {
			data.push_back(i);
		}
		Attrs attrs = {
		        {"divisor",     1          },
                        {"format",      GL_INT     },
                        {"oformat",     GL_INT     },
		        {"a.a_draw_id", 1          },
                        {"nelem",       data.size()},
                        {"data",        data.data()},
		};
		spu_array_aux(m_object.getArray().id(), attrs, 5);
	}
	// offsets
	for (auto y = 0; y < 4; y++) {
		for (auto x = 0; x < 4; x++) {
			m_offsets.push_back(Vec3f((x - 1.5) * 3, (y - 1.5) * 3, -12));
		}
	}
	assert(m_offsets.size() <= c_count);
}

void App::render()
{
	auto t = getSeconds().current();
	Mat4f viewscreen;
	sb6::Composition composition;
	composition.perspective(viewport(0), 60.0, 0.1, 1800.0);
	viewscreen = composition.viewscreen();
	for (auto i = 0u; i < m_offsets.size(); i++) {
		Mat4f modelview
		        = c_unit.trans(m_offsets[i]) * c_unit.rot("XZY", -t * 35.3, -t * 17.75, -t * 43.75);

		u_constants[i].modelview = modelview;
		u_constants[i].viewscreen = viewscreen;
		t += 0.25;
	}
	m_shader.use();
	m_object.draw(m_offsets.size());
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("pmbstreaming");
}  // namespace spu::pmbstreaming
