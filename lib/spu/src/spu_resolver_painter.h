//
// Uniform :
//
#pragma once
#include "spu_object.h"
/*
#include "spu_print_shader.h"
#include "spu_print_font.h"
#include "spu_print_character.h"
*/

namespace spu::libspu::spu_frame {

class Uniform {
public:
	void setShader(uint32_t shaderId) { m_shaderId = shaderId; }
	bool addUniform(const std::string &name, void *ptr)
	{
		int32_t loc;
		uint32_t size;
		uint32_t type;
		const auto *name_cstr = name.c_str();
		spu_shader_loc(m_shaderId, &name_cstr, &loc, &size, &type, 1);

		if (loc >= 0) {
			m_names.push_back(name);
			m_ptrs.push_back(ptr);
			m_locs.push_back(loc);
			m_sizes.push_back(size);
			return true;
		}
		return false;
	}

	void use() const { spu_shader_use(m_shaderId, m_locs.data(), m_ptrs.data(), m_locs.size()); }

private:
	uint32_t m_shaderId;
	std::vector<std::string> m_names;
	std::vector<int32_t> m_locs;
	std::vector<void *> m_ptrs;
	std::vector<uint32_t> m_sizes;
};

struct Status {
	union Flags {
		struct {
			uint32_t fill      : 1 = 1;
			uint32_t blend     : 1 = 0;
			uint32_t cull_face : 1 = 0;
			uint32_t depth_test: 1 = 0;
		};
		uint32_t bits;
	};
};

class Painter {
public:
	Painter(const Attrs &attrs)
	{
		m_unif.setShader(ms_shaderId);
		m_unif.addUniform("u_color0", &u_color0);
		m_unif.addUniform("u_color1", &u_color1);
		m_unif.addUniform("u_color2", &u_color2);
		m_unif.addUniform("u_color3", &u_color3);
		m_unif.addUniform("u_color4", &u_color4);
		m_unif.addUniform("u_color5", &u_color5);
		m_unif.addUniform("u_color6", &u_color6);
		m_unif.addUniform("u_color7", &u_color7);
		m_unif.addUniform("u_color8", &u_color8);
		m_unif.addUniform("u_color9", &u_color9);
		m_unif.addUniform("u_depth", &u_depth);
	}

	~Painter() { spu_shader_delete(m_shaderId); }

	void render()
	{
		pushStatus();
		m_unif.use();
		spu_array_draw(0, GL_TRIANGLE_STRIP);
		popStatus();
	}

	inline static void startup()  // no shutdown. trust buddies.
	{
		// renderstate
		{
			Attrs attrs = {
			        {"flags.fill",       0               },
                                {"flags.blend",      0               },
                                {"flags.cull_face",  0               },
			        {"flags.depth_test", 0               },
                                {"flags",            &ms_status.flags},
			};
			ms_renderstateId = spu_renderstate_new(attrs);
		}

		// shader
		{
			Attrs attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			ms_shaderId = spu_shader_new(attrs);
		}
	}

private:
	inline static uint32_t ms_renderstateId = 0;
	inline static uint32_t ms_shaderId = 0;
	inline static uint32_t ms_textureId = 0;
	inline static Status ms_status;

	Uniform m_unif;

	void pushStatus()
	{
		spu_renderstate_get(ms_renderstateId);
		m_initialStatus = ms_status;
		ms_status = m_printStatus;
		spu_renderstate_use(ms_renderstateId);
		m_unif.use();
	}

	void popStatus()
	{
		spu_renderstate_get(ms_renderstateId);  // just flush
		ms_status = m_initialStatus;
		spu_renderstate_use(ms_renderstateId);
		spu_shader_use(0);  // for legacy program
	}
};
}  // namespace spu::libspu::spu_frame
