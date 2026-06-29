//
// Application :
//
#include "renderstate.h"
#include "capture.h"
#include "c_ball_png.h"
#include <smath/mat4f.h>

namespace spu {

static constexpr uint32_t c_width = 1280;
static constexpr uint32_t c_height = 720;

class Application {
public:
	Renderstate m_renderstate;

	uint32_t m_shader_id = 0;
	uint32_t m_array_id = 0;
	Vec3f m_rot = {0, 0, 0};

	struct {
		Mat4f nodeworld;
		Mat4f worldview;
		Mat4f viewscreen;
		Vec4f colors[8] = {
			{1,0,0,1},
			{0,1,0,1},
			{0,0,1,1},
			{0,1,1,1},
			{1,0,1,1},
			{1,1,0,1},
			{1,1,1,1},
			{0,0,0,1},
		};
	} u;

	static constexpr uint32_t c_unif_count = 4;

	const char *m_names[c_unif_count] = {
	        "u_nodeworld",
	        "u_worldview",
	        "u_viewscreen",
	        "u_colors",
	};
	void *m_ptrs[c_unif_count] = {
	        &u.nodeworld,
	        &u.worldview,
	        &u.viewscreen,
	        &u.colors,
	};
	int m_locs[c_unif_count] = {0};

	Application()
	{
		// renderstate
		m_renderstate.init();

		// matrix
		{
			float fovy = 45.0;
			float near = 1.0;
			float far = 8.0;
			float aspect = float(c_width) / float(c_height);
			u.viewscreen.set_projection(&fovy, &aspect, &near, &far);


			Vec3f eye = {0, +4, -4};
			Vec3f dir = -eye;
			Vec3f up = {0, 1, 0};
			Mat4f viewworld;
			viewworld.set_orientation(&eye, &dir, &up);
			u.worldview = viewworld.unitary_inverse();
		}

		// shader
		{
			Attrs shader_attrs = {
				{"use_unif_block", true},
			};
			m_shader_id = spu_inventory_new("shader", "simple_3d.src.us", shader_attrs);
			spu_shader_loc(m_shader_id, m_names, m_locs, 0, 0, c_unif_count);
		}

		// array
		{
			Attrs attrs = {
			        {"shader_id",    m_shader_id},
			        {"a.a_position", 4          },
			};
			m_array_id = spu_array_new(attrs);

			/*
			    7------6
			   /|     /|
			  3------2 |
			  | 4----|-5
			  |/     |/
			  0------1
			 */
			// clang-format off
			std::vector<Vec4f> vertices = {
			        {-1, -1, -1, 1}, {+1, -1, -1, 1}, {+1, +1, -1, 1}, {-1, +1, -1, 1},
                                {-1, -1, +1, 1}, {+1, -1, +1, 1}, {+1, +1, +1, 1}, {-1, +1, +1, 1},
			};
			std::vector<int32_t> indices = { // QUADS
			        0, 1, 2, 3, 1, 5, 6, 2, 5, 4, 7, 6, 4, 0, 3, 7, 2, 6, 7, 3, 5, 1, 0, 4
			};
			// clang-format on

			spu_array_send(m_array_id, vertices.data(), vertices.size(), 0);
			spu_array_send(m_array_id, indices.data(), indices.size(), -1);
		}
		// renderstate
		{
			m_renderstate.m_flags.depth_test = 1;
			m_renderstate.set();
		}
	}

	~Application()
	{
		spu_inventory_delete(m_shader_id);
		spu_array_delete(m_array_id);
		spu_graphics_shutdown();
	}

	void play()
	{
		m_rot.x += 0.003;
		m_rot.y += 0.005;
		m_rot.z += 0.007;
		u.nodeworld = Mat4f().rot("xyz", m_rot.x, m_rot.y, m_rot.z);

		spu_frame_clear(-1);
		spu_shader_use(m_shader_id, m_locs, m_ptrs, c_unif_count);
		spu_array_draw(m_array_id, GL_QUADS);

		//printf("---------------- REPORT ---------------------\n");
		//spu_shader_report(m_shader_id);
	}
};
}  // namespace spu

using namespace spu;
int main(int, const char *argv[])
{
	Attrs attrs(argv + 1);
	attrs.trace("main", false);

	Vec4i window = {32, 32, c_width, c_height};

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
