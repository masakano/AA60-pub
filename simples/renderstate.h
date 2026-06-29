//
// Renderstate :
//
//
#include <spu/spu.h>
#include <smath/vec.h>

namespace spu {
class Renderstate {
public:
	union Flags {
		struct {
			uint32_t fill        : 1;
			uint32_t blend       : 1;
			uint32_t cull_face   : 1;
			uint32_t ccw         : 1;
			uint32_t depth_test  : 1;
			uint32_t stencil_test: 1;
			uint32_t scissor_test: 1;
			uint32_t point_sprite      : 1;
			uint32_t program_point_size: 1;
			uint32_t fill_offset       : 1;
			uint32_t line_offset       : 1;
			uint32_t point_offset      : 1;
			uint32_t line_stipple      : 1;
			uint32_t sample_shading   : 1;
			uint32_t cube_map_seamless: 1;
			uint32_t depth_clamp      : 1;
			uint32_t clip_distance0   : 1;
			uint32_t clip_distance1   : 1;
			uint32_t clip_distance2   : 1;
			uint32_t clip_distance3   : 1;
			uint32_t clip_distance4   : 1;
			uint32_t clip_distance5   : 1;
		};
		uint32_t bits;
	} m_flags;

	void init()
	{
		Attrs attrs = {
		        {"flags.fill",               0       },
		        {"flags.blend",              0       },
		        {"flags.cull_face",          0       },
		        {"flags.ccw",                0       },
		        {"flags.depth_test",         0       },
		        {"flags.stencil_test",       0       },
		        {"flags.scissor_test",       0       },
		        {"flags.point_sprite",       0       },
		        {"flags.program_point_size", 0       },
		        {"flags.fill_offset",        0       },
		        {"flags.line_offset",        0       },
		        {"flags.point_offset",       0       },
		        {"flags.line_stipple",       0       },
		        {"flags.sample_shading",     0       },
		        {"flags.cube_map_seamless",  0       },
		        {"flags.depth_clamp",        0       },
		        {"flags.clip_distance0",     0       },
		        {"flags.clip_distance1",     0       },
		        {"flags.clip_distance2",     0       },
		        {"flags.clip_distance3",     0       },
		        {"flags.clip_distance4",     0       },
		        {"flags.clip_distance5",     0       },
		        {"flags",                    &m_flags},
		};
		m_renderstateId = spu_renderstate_new(attrs);
	}

	void get() { spu_renderstate_get(m_renderstateId); }
	void set() { spu_renderstate_use(m_renderstateId); }

private:
	uint32_t m_renderstateId = 0;
};
}  // namespace spu
