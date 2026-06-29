//
// Ack :
//
#include "renderstate.h"
#include "c_ball_png.h"

namespace spu {

namespace {

void default_sender_callback(void *arg)
{
	struct Ack {
		int32_t size;
		SpuPad pad;
	};
	SpuPad *pad = nullptr;
	spu_graphics_get("pad", &pad);
	auto *ack = static_cast<Ack *>(arg);
	*pad = ack->pad;
}
}  // namespace

class Application {
public:
	Renderstate m_renderstate;

	u_int m_shader_id = 0;
	u_int m_array_id = 0;
	u_int m_frame_id = 0;
	u_int m_texture_id = 0;
	u_int m_depth_id = 0;
	u_int u_albedo = 0;

	const char *m_names[1] = {"u_albedo"};
	void *m_ptrs[1] = {&u_albedo};
	int m_locs[1] = {0};

	struct Vertex {
		Vec4f p;
		Vec4f c;
	};

	std::vector<Vertex> m_vertices;
	std::vector<Vec4f> m_velocities;

	Application(const Attrs &attrs)
	{
		Vec4i window = {0, 0, 1280, 720};
		Vec4f viewport = {0, 0, 1280, 720};

		// debug
		{
			int message_level = attrs.get("message.level", 0);
			bool is_aux_printf = attrs.get("message.printf", 1) != 0;
			bool is_aux_query = attrs.get("message.query", 0) != 0;

			g_message.output = is_aux_printf ? g_message.output : nullptr;
			g_message.level = message_level;
			g_message.is_query = is_aux_query;
		}

		// init graphics
		{
			Attrs graphics_attrs = {
			        {"window", window},
			        {"device", "egl" },
			};
			spu_graphics_init(graphics_attrs);
		}

		// texture
		{
			File::embed(c_ball_png_name, c_ball_png_data, sizeof(c_ball_png_data));
			u_albedo = spu_inventory_new("texture", "ball.png", Attrs());
			spu_inventory_sync(u_albedo, 0);
		}

		// shader
		{
			m_shader_id = spu_inventory_new("shader", "simple.us", Attrs());
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

			m_renderstate.init();
			m_renderstate.set();
		}

		// frame buffer
		{
			Attrs color_attrs = {
			        {"target",     GL_TEXTURE_2D},
                                {"iformat",    GL_RGBA8     },
                                {"width",      window.sx    },
			        {"height",     window.sy    },
                                {"max_levvel", 0            },
			};
			m_texture_id = spu_texture_new(color_attrs);

			Attrs depth_attrs = {
			        {"target",  GL_RENDERBUFFER      },
			        {"iformat", GL_DEPTH_COMPONENT32F},
			        {"width",   window.sx            },
			        {"height",  window.sy            },
			};
			m_depth_id = spu_texture_new(depth_attrs);

			Attrs frame_attrs = {
			        {"viewport0", viewport    },
			        {"color0",    m_texture_id},
			        {"epth",      m_depth_id  },
			};
			m_frame_id = spu_frame_new(frame_attrs);
		}
		// encoder
		{
			Attrs dec_attrs = {
			        {"path",          "127.0.0.1:12345"      },
			        {"width",         window.sx              },
			        {"height",        window.sy              },
			        {"bitrate",       32 * 1000 * 1000       },
			        {"target_fps",    60                     },
			        {"control_clock", 1                      },
			        {"callback",      default_sender_callback},
			        {"texture_id",    m_texture_id           },
			};
			spu_video_enc_new(dec_attrs);
		}
	}

	~Application()
	{
		spu_inventory_delete(m_shader_id);
		spu_array_delete(m_array_id);
		spu_texture_delete(u_albedo);
		spu_frame_delete(m_frame_id);
		spu_texture_delete(m_texture_id);
		spu_texture_delete(m_depth_id);
	}

	void play()
	{
		for (u_int i = 0; i < 4; i++) {
			m_vertices[i].p += m_velocities[i];

			if (m_vertices[i].p.x > +0.9) m_velocities[i].x *= -1;
			if (m_vertices[i].p.y > +0.9) m_velocities[i].y *= -1;

			if (m_vertices[i].p.x < -0.9) m_velocities[i].x *= -1;
			if (m_vertices[i].p.y < -0.9) m_velocities[i].y *= -1;

			spu_array_send(m_array_id, m_vertices.data(), m_vertices.size());
		}

		spu_frame_begin(m_frame_id);
		spu_frame_clear(m_frame_id);
		spu_shader_use(m_shader_id, m_locs, m_ptrs, 1);
		spu_array_draw(m_array_id, GL_POINTS);
		spu_frame_end();
		spu_video_enc(nullptr);
	}
};
}  // namespace spu

using namespace spu;
int main(int, const char *argv[])
{
	Attrs attrs(argv + 1);
	attrs.trace("main", false);

	auto app = new Application(attrs);

	while (1) {
		app->play();
		spu_graphics_swap();  // with flush
	}
	delete app;
	spu_graphics_shutdown();
	return 0;
}
