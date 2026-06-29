//
// Application :
//
#include "renderstate.h"
#include "capture.h"
#include "c_ball_png.h"

namespace spu {

class Application {
public:
	Renderstate m_renderstate;

	uint32_t m_shader_id = 0;
	uint32_t m_array_id = 0;
	uint32_t u_albedo = 0;

	const char *m_names[1] = {"u_albedo"};
	void *m_ptrs[1] = {&u_albedo};
	int m_locs[1] = {0};

	struct Vertex {
		Vec4f p;
		Vec4f c;
	};

	std::vector<Vertex> m_vertices;
	std::vector<Vec4f> m_velocities;

	Application()
	{
		// renderstate
		m_renderstate.init();

		// texture
		{
			File::embed(c_ball_png_name, c_ball_png_data, sizeof(c_ball_png_data));
			u_albedo = spu_inventory_new("texture", "ball.png", Attrs());
			spu_inventory_sync(u_albedo, 0);
		}
		{
			Attrs shader_attrs = {
				{"use_unif_block", true},
			};
			m_shader_id = spu_inventory_new("shader", "simple.us", shader_attrs);
			spu_shader_loc(m_shader_id, m_names, m_locs, 0, 0, 1);
		}

		// array
		{
			m_velocities = {
			        {+0.02, +0.07, 0, 0},
			        {+0.03, -0.05, 0, 0},
			        {-0.05, +0.03, 0, 0},
			        {-0.07, -0.02, 0, 0},
			};

			m_vertices = {
			        {{-0.5, -0.5, 0.0, 1.0}, {1.0, 0.0, 0.0, 1.0}},
			        {{+0.5, -0.5, 0.0, 1.0}, {0.0, 1.0, 0.0, 1.0}},
			        {{+0.5, +0.5, 0.0, 1.0}, {0.0, 0.0, 1.0, 1.0}},
			        {{-0.5, +0.5, 0.0, 1.0}, {0.0, 1.0, 1.0, 1.0}},
			};
			Attrs attrs = {
			        {"shader_id",    m_shader_id      },
                                {"a.a_position", 4                },
                                {"a.a_color",    4                },
			        {"data",         m_vertices.data()},
                                {"nelem",        m_vertices.size()},
			};
			m_array_id = spu_array_new(attrs);
		}
		// renderstate
		{
			m_renderstate.m_flags.program_point_size = true;
			m_renderstate.m_flags.point_sprite = 1;
			m_renderstate.set();
		}
	}

	~Application()
	{
		spu_inventory_delete(m_shader_id);
		spu_array_delete(m_array_id);
		spu_texture_delete(u_albedo);
		spu_graphics_shutdown();
	}

	void play()
	{
		for (auto i = 0u; i < m_vertices.size(); i++) {
			m_vertices[i].p += m_velocities[i];

			if (m_vertices[i].p.x > +0.9) m_velocities[i].x *= -1;
			if (m_vertices[i].p.y > +0.9) m_velocities[i].y *= -1;

			if (m_vertices[i].p.x < -0.9) m_velocities[i].x *= -1;
			if (m_vertices[i].p.y < -0.9) m_velocities[i].y *= -1;

			spu_array_send(m_array_id, m_vertices.data(), m_vertices.size());
		}
		spu_frame_clear(-1);
		spu_shader_use(m_shader_id, m_locs, m_ptrs, 1);
		spu_array_draw(m_array_id, GL_POINTS);
	}
};
}  // namespace spu

using namespace spu;
int main(int, const char *argv[])
{
	Attrs attrs(argv + 1);
	attrs.trace("main", false);

	Vec4i window = {32, 32, 1280, 720};

	g_message.level = attrs.get("message.level", 0);

	// init
	Attrs def_attrs = {
	        {"window", window},
	        {"device", "glfw"},
	};
	attrs.prepend(def_attrs);
	spu_graphics_init(attrs);

	auto capture = new Capture(window);
	auto app = new Application();

	bool is_open = true;
	while (is_open) {
		capture->begin();
		app->play();
		capture->end();
		is_open = spu_graphics_swap();  // need check
	}

	delete app;
	delete capture;

	spu_graphics_shutdown();  // redundant
	return 0;
}
