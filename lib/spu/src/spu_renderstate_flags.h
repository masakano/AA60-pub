//
// Flags :
//
#pragma once

#include "spu_renderstate_lib.h"

namespace spu::libspu::spu_renderstate {
const Attrs c_symbolAttrs = {
        {"blend",              GL_BLEND                        },
        {"cull_face",          GL_CULL_FACE                    },
        {"depth_test",         GL_DEPTH_TEST                   },
        {"stencil_test",       GL_STENCIL_TEST                 },
        {"scissor_test",       GL_SCISSOR_TEST                 },
        {"conservative",       GL_CONSERVATIVE_RASTERIZATION_NV},
        {"point_sprite",       GL_POINT_SPRITE                 },
        {"program_point_size", GL_PROGRAM_POINT_SIZE           },
        {"fill_offset",        GL_POLYGON_OFFSET_FILL          },
        {"line_offset",        GL_POLYGON_OFFSET_LINE          },
        {"point_offset",       GL_POLYGON_OFFSET_POINT         },
        {"multisample",        GL_MULTISAMPLE                  },
        {"sample_shading",     GL_SAMPLE_SHADING               },
        {"depth_clamp",        GL_DEPTH_CLAMP                  },
        {"cube_map_seamless",  GL_TEXTURE_CUBE_MAP_SEAMLESS    },
        {"clip_distance0",     GL_CLIP_DISTANCE0               },
        {"clip_distance1",     GL_CLIP_DISTANCE1               },
        {"clip_distance2",     GL_CLIP_DISTANCE2               },
        {"clip_distance3",     GL_CLIP_DISTANCE3               },
        {"clip_distance4",     GL_CLIP_DISTANCE4               },
        {"clip_distance5",     GL_CLIP_DISTANCE5               },
        {"fill",               GL_POLYGON_MODE                 },
        {"ccw",                GL_FRONT_FACE                   },
        {"line_stipple",       GL_LINE_STIPPLE                 },
        {"srgb_encode",        GL_FRAMEBUFFER_SRGB             },
};

class Flags {
public:
	Flags(const Attrs &attrs)
	{
		auto is_probe = attrs.get("probe", false);
		for (const auto &symbol_attr: c_symbolAttrs) {
			attrs.addToLog(symbol_attr);
		}
		for (auto &attr: attrs) {
			auto bit = 1;
			auto is_found = false;
			for (const auto &symbol_attr: c_symbolAttrs) {
				if (attr.key() == symbol_attr.key()) {
					m_mask |= bit;
					m_scatters.push_back(bit);
					is_found = true;
					break;
				}
				bit <<= 1;
			}
			if (!is_probe && !is_found) {
				attrs.report("renderstate flags");
				aux_error(true, "%s : flag not found\n", attr.key().c_str());
			}
		}
	}

	bool use(uint32_t user_defined_flags)
	{
		auto flags = unpack(user_defined_flags);
		auto changes = (ms_flags ^ flags) & m_mask;
		auto bpos = 1u;
		auto is_changed = false;
		for (const auto &symbol_attr: c_symbolAttrs) {
			auto glenum = (int32_t)symbol_attr;
			auto is_enable = (flags & bpos) != 0u;

			if (glenum == 0) {
				/* do nothing */
			}
			else if (glenum == GL_BLEND && s_blend_func_channel != -1) {
				auto mask = 1u << s_blend_func_channel;
				auto is_enable_chan = (ms_blend_enables & mask) == mask;
				if (is_enable != is_enable_chan) {
					is_changed = true;
					if (is_enable) {
						F(glEnablei, glenum, s_blend_func_channel);
						ms_blend_enables |= mask;
					}
					else {
						F(glDisablei, glenum, s_blend_func_channel);
						ms_blend_enables &= ~mask;
					}
				}
			}
			else if ((changes & bpos) != 0u) {
				is_changed = true;
				switch (glenum) {
				case GL_POLYGON_MODE: {
					F(glPolygonMode, GL_FRONT_AND_BACK, is_enable ? GL_FILL : GL_LINE);
					break;
				}
				case GL_FRONT_FACE: {
					F(glFrontFace, is_enable ? GL_CCW : GL_CW);
					break;
				}
				case GL_BLEND: {
					if (is_enable) {
						F(glEnable, glenum);
					}
					else {
						F(glDisable, glenum);
					}
					ms_blend_enables = is_enable ? ~0u : 0;
					break;
				}
				default: {
					if (is_enable) {
						F(glEnable, glenum);
					}
					else {
						F(glDisable, glenum);
					}
					break;
				}
				}
			}
			bpos <<= 1;
		}
		ms_flags = (ms_flags & ~m_mask) | (flags & m_mask);
		return is_changed;
	}

	uint32_t get()
	{
		auto bpos = 1u;
		auto flags = 0u;
		for (const auto &symbol_attr: c_symbolAttrs) {
			if ((m_mask & bpos) != 0u) {
				auto glenum = (int32_t)symbol_attr;
				// special
				switch (glenum) {
				case 0:
					// do nothing
					break;

				case GL_BLEND: {
					auto chan = s_blend_func_channel;
					auto is_enabled = glIsEnabledi(glenum, chan == -1 ? 0 : chan);
					if (is_enabled) {
						flags |= bpos;
					}
					break;
				}
				case GL_POLYGON_MODE:
					if (getInt(glenum) == GL_FILL) {
						flags |= bpos;
					}
					break;
				case GL_FRONT_FACE:
					if (getInt(glenum) == GL_CCW) {
						flags |= bpos;
					}
					break;
				default: {
					auto is_enabled = glIsEnabled(glenum);
					if (is_enabled) {
						flags |= bpos;
					}
					break;
				}
				}
			}
			bpos <<= 1;
		}
		ms_flags = (ms_flags & ~m_mask) | (flags & m_mask);
		return pack();
	}

	void report(const std::string &prefix) const
	{
		auto flags = ms_flags;
		for (const auto &symbol_attr: c_symbolAttrs) {
			if ((m_mask & 0x1) != 0u) {
				aux_printf(
				        "\t%-24s : %d\n", (prefix + symbol_attr.key().c_str()).c_str(),
				        flags & 0x1);
			}
			flags >>= 1;
		}
	}

private:
	inline static uint32_t ms_flags = 0u;
	inline static uint32_t ms_blend_enables = 0u;

	uint32_t m_mask = 0u;
	std::vector<int32_t> m_scatters;

	uint32_t pack()
	{
		auto flags = ms_flags;
		auto user_defined_flags = 0u;
		auto bits = 1u;
		for (auto &scatter: m_scatters) {
			if ((flags & scatter) != 0u) {
				user_defined_flags |= bits;
			}
			bits <<= 1;
		}
		return user_defined_flags;
	}

	uint32_t unpack(uint32_t user_defined_flags)
	{
		auto flags = 0u;
		for (auto &scatter: m_scatters) {
			if ((user_defined_flags & 0x1) != 0u) {
				flags |= scatter;
			}
			user_defined_flags >>= 1;
		}
		return flags;
	}
};
}  // namespace spu::libspu::spu_renderstate
