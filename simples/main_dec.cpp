//
// Application :
//
#include "renderstate.h"
#include "c_ball_png.h"

namespace spu {

class Application {
public:
	Renderstate &m_renderstate;

	u_int m_shader_id = 0;
	u_int m_array_id = 0;
	// u_int m_decode_id = 0;
	u_int m_texture_id = 0;

	const char *m_names[1] = {"u_color0"};
	void *m_ptrs[1] = {&m_texture_id};
	int m_locs[1] = {0};

	Application(Renderstate &renderstate, const Vec4i &window) : m_renderstate(renderstate)
	{
		int width = window.sx;
		int height = window.sy;

		// shader
		{
			m_shader_id = spu_inventory_new("shader", "copy.us", Attrs());
			spu_shader_loc(m_shader_id, m_names, m_locs, 0, 0, 1);
		}

		// array
		{
			Attrs attrs = {
			        {"nelem", 4},
			};
			m_array_id = spu_array_new(attrs);
		}

		// texture
		{
			Attrs attrs = {
			        {"target",  GL_TEXTURE_2D},
			        {"iformat", GL_RGBA8     },
			        {"width",   width        },
			        {"height",  height       },
			};
			m_texture_id = spu_texture_new(attrs);
		}
		// decoder
		{
			Attrs attrs = {
			        {"path",       "127.0.0.1:12345"},
			        {"callback",   receiver_callback},
			        {"texture_id", m_texture_id     },
			};
			/*m_decode_id = */ spu_video_dec_new(attrs);
		}
	}

	~Application()
	{
		spu_inventory_delete(m_shader_id);
		spu_array_delete(m_array_id);
		spu_texture_delete(m_texture_id);
		spu_video_dec_delete();
		spu_graphics_shutdown();
	}

	void play()
	{
		spu_video_dec(nullptr);
		spu_shader_use(m_shader_id, m_locs, m_ptrs, 1);
		spu_array_draw(m_array_id, GL_TRIANGLE_STRIP);
	}

	static void receiver_callback(void *arg)
	{
		struct Ack {
			int32_t size;
			SpuPad pad;
		};

		SpuPad *pad = nullptr;
		spu_graphics_get("pad", &pad);

		auto *ack = static_cast<Ack *>(arg);
		ack->size = sizeof(ack->pad);
		ack->pad = *pad;
	}
};
}  // namespace spu

using namespace spu;
int main(int /*argc*/, const char *argv[])
{
	Attrs attrs(argv + 1);
	attrs.trace("main", false);

	Renderstate renderstate;
	Vec4i window = {32, 32, 1280, 720};

	// message level
	{
		g_message.level = attrs.get("message.level", 0);
	}

	// init
	{
		Attrs attrs = {
		        {"window", window},
		        {"device", "glfw"},
		};
		spu_graphics_init(attrs);
		renderstate.init();
		renderstate.get();
		renderstate.set();
	}

	// run application
	{
		auto app = new Application(renderstate, window);
		bool is_open = true;
		while (is_open) {
			app->play();
			is_open = spu_graphics_swap();  // need check
		}
		delete app;
	}
	return 0;
}
