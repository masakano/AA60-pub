//
// App :
//
#include "base_app.h"
namespace spu::springmass {

enum {
	e_points_x = 50,
	e_points_y = 50,
	e_points_total = (e_points_x * e_points_y),
};

class App : public BaseApp {
public:
	void render() override;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	SpuShader m_renderShader;

	SpuComputeArray m_compArray;

	SpuArray m_drawArray;
	SpuTexture m_posTexture;
	int32_t m_iterationsPerFrame = 16;
	uint32_t u_tex_position;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	std::vector<Vec4f> initial_positions(e_points_total);
	std::vector<Vec4f> initial_velocities(e_points_total);
	std::vector<Vec4i> connection_vectors(e_points_total);

	// shaders
	{
		Attrs unif_attrs = {
		        {"tex_position", &u_tex_position},
		};
		loadShader(m_renderShader, "springmass/render.us", Attrs(), unif_attrs);
	}
	// initial vectors
	auto n = 0;
	for (auto j = 0; j < e_points_y; j++) {
		auto fj = float(j) / float(e_points_y);
		for (auto i = 0; i < e_points_x; i++) {
			auto fi = i / float(e_points_x);
			initial_positions[n]
			        = {(fi - 0.5f) * float(e_points_x), (fj - 0.5f) * float(e_points_y),
			           0.6f * sinf(fi) * cosf(fj), 1.0f};
			initial_velocities[n] = ezero<Vec4f>();

			connection_vectors[n] = Vec4i(-1);
			if (j != (e_points_y - 1)) {
				if (i != 0) connection_vectors[n].i[0] = n - 1;
				if (j != 0) connection_vectors[n].i[1] = n - e_points_x;
				if (i != (e_points_x - 1)) connection_vectors[n].i[2] = n + 1;
				if (j != (e_points_y - 1)) connection_vectors[n].i[3] = n + e_points_x;
			}
			n++;
		}
	}
	// compute
	{
		auto &array = m_compArray;
		auto &shader = array.getShader();

		Attrs unif_attrs = {
		        {"tex_position", &u_tex_position},
		};
		loadShader(shader, "springmass/compute.us", Attrs(), unif_attrs);

		Attrs attrs0 = {
		        {"shader_id",         shader.id()   },
		        {"a.a_position_mass", 4             },
		        {"nelem",             e_points_total},
		};
		Attrs attrs1 = {
		        {"a.a_velocity", 4             },
		        {"nelem",        e_points_total},
		};
		Attrs attrs2 = {
		        {"a.a_connection", 4             },
		        {"nelem",          e_points_total},
		};
		Attrs attrs3 = {
		        {"a.a_tf_position_mass", 4             },
		        {"nelem",                e_points_total},
		};
		Attrs attrs4 = {
		        {"a.a_tf_velocity", 4             },
		        {"nelem",           e_points_total},
		};
		array.aux(attrs0, 0);
		array.aux(attrs1, 1);
		array.aux(attrs2, 2);
		array.aux(attrs3, 3);
		array.aux(attrs4, 4);

		array.send(initial_positions, 0);
		array.send(initial_velocities, 1);
		array.send(connection_vectors, 2);

		array.getDim().x = e_points_total / 100;  // local_size_x = 100
		                                          // array.m_gy = 1;
		                                          // array.m_gz = 1;
	}

	{
		Attrs attrs = {
		        {"target",    GL_TEXTURE_BUFFER},
		        {"iformat",   GL_RGBA32F       },
		        {"buffer_id", 0                }, // place holder
		};
		int32_t buffer_id = 0;
		spu_array_get(m_compArray.id(), "0.buffer_id", &buffer_id);
		attrs.replace("buffer_id", buffer_id);
		m_posTexture.init(attrs);
		u_tex_position = m_posTexture.id();
	}
	{
		constexpr uint32_t lines = (e_points_x - 1) * e_points_y + (e_points_y - 1) * e_points_x;
		int32_t index[lines * 2];
		int32_t *e = index;
		for (auto j = 0; j < e_points_y; j++) {
			for (auto i = 0; i < e_points_x - 1; i++) {
				*e++ = i + j * e_points_x;
				*e++ = 1 + i + j * e_points_x;
			}
		}
		for (auto i = 0; i < e_points_x; i++) {
			for (auto j = 0; j < e_points_y - 1; j++) {
				*e++ = i + j * e_points_x;
				*e++ = e_points_x + i + j * e_points_x;
			}
		}
		Attrs attr0 = {
		        {"shader_id",    m_renderShader.id()},
		        {"a.a_position", 4                  },
		        {"nelem",        e_points_total     },
		};
		m_drawArray.init(attr0);
		m_drawArray.link(0, m_compArray, 0);  // position
		m_drawArray.send(index, lines * 2, -1, 4);
	}
	// renderstate
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.program_point_size = false;
		renderstate.point_size = 4.0;
	}
}

void App::render()
{
	for (auto i = m_iterationsPerFrame; i != 0; --i) {
		m_compArray.compute();
		m_compArray.copy(0, m_compArray, 3);
		m_compArray.copy(1, m_compArray, 4);
	}
	m_renderShader.use();
	m_drawArray.draw(GL_POINTS);
	m_drawArray.draw(GL_LINES);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("springmass");
}  // namespace spu::springmass
