//
// Application :
//
#include <gsys/canvas/gs_page.h>
#include <gsys/canvas/gs_site.h>
#include "c_ball_png.h"

namespace spu {

class Application : public gs_canvas::GsPage {
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

	explicit Application(const char *name) : gs_canvas::GsPage(name) {}

	void init(const Attrs &attrs) override
	{
		GsPage::init(attrs);

		// texture
		{
			File::embed(c_ball_png_name, c_ball_png_data, sizeof(c_ball_png_data));
			u_albedo = spu_inventory_new("texture", "ball.png", Attrs());
			spu_inventory_sync(u_albedo, 0);
		}

		// shader
		{
			Attrs shader_attrs = {
			        {"use_unif_block", true},
			};

			Attrs unif_attrs = {
			        {"u_albedo", &u_albedo},
			};
			m_shader.init("simple.us", shader_attrs);
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

	void render() override
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
static ObjectRegistry<GsCanvas>::Creator<Application> page_creator("app");
}  // namespace spu

using namespace spu;

int main(int, const char *argv[])
{
	Attrs attrs(argv + 1);
	attrs.trace("main", false);
	gs_canvas::GsSite::main(attrs);
	return 0;
}
