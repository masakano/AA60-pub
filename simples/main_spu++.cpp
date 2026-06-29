//
// Application :
//
#include <spu++/spu++.h>
#include "c_ball_png.h"

namespace spu {

class Application {
public:
	SpuRenderstate m_renderstate;
	SpuShader m_shader;
	SpuArray m_array;
	uint32_t u_albedo = 0;

	struct Vertex {
		Vec4f p;
		Vec4f c;
	};

	std::vector<Vertex> m_vertices;
	std::vector<Vec4f> m_velocities;

	Application()
	{
		// texture
		{
			File::embed(c_ball_png_name, c_ball_png_data, sizeof(c_ball_png_data));
			u_albedo = spu_inventory_new("texture", "ball.png", Attrs());
			spu_inventory_sync(u_albedo, 0);
		}

		// shader
		{
			Attrs unif_attrs = {
			        {"u_albedo", &u_albedo},
			};
			m_shader.init("simple.us", Attrs());
			m_shader.addUniforms(unif_attrs);
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
			        {"shader_id",    m_shader.id()    },
                                {"a.a_position", 4                },
                                {"a.a_color",    4                },
			        {"data",         m_vertices.data()},
                                {"nelem",        m_vertices.size()},
			};
			m_array.init(attrs);
		}

		// renderstate
		{
			m_renderstate.flags.program_point_size = true;
			m_renderstate.flags.point_sprite = true;
			m_renderstate.use();
		}
	}

	~Application() { spu_texture_delete(u_albedo); }

	void play()
	{
		for (auto i = 0; i < 4; i++) {
			m_vertices[i].p += m_velocities[i];

			if (m_vertices[i].p.x > +0.9) m_velocities[i].x *= -1;
			if (m_vertices[i].p.y > +0.9) m_velocities[i].y *= -1;

			if (m_vertices[i].p.x < -0.9) m_velocities[i].x *= -1;
			if (m_vertices[i].p.y < -0.9) m_velocities[i].y *= -1;

			m_array.send(m_vertices.data(), m_vertices.size());
		}
		spu_frame_clear(0);
		m_renderstate.use();
		m_shader.use();
		m_array.draw(GL_POINTS);
	}
};
}  // namespace spu

using namespace spu;
int main(int, const char *argv[])
{
	Attrs attrs(argv + 1);
	attrs.trace("main", false);

	Vec4i window = {32, 32, 1280, 720};
	Attrs def_attrs = {
	        {"window", window},
	};
	attrs.prepend(def_attrs);
	SpuRenderstate::startup(attrs);

	g_message.level = attrs.get("message.level", 0);

	auto app = new Application();
	bool is_open = true;
	while (is_open) {
		app->play();
		is_open = spu_graphics_swap();  // need check
	}
	delete app;
	SpuRenderstate::shutdown();
	return 0;
}
