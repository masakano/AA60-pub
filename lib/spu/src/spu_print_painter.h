//
// UniMat :
//
#pragma once
#include "spu_object.h"
#include "spu_print_shader.h"
#include "spu_print_font.h"
#include "spu_print_character.h"
#include "uniform.h"

#if 0
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif
#endif

namespace spu::libspu::spu_print {

struct UniMat {
	float c[4][4] = {{0}};

	UniMat() { c[0][0] = c[1][1] = c[2][2] = c[3][3] = 1; }

	UniMat(float s0, float s1, float s2, float t0, float t1, float t2)
	{
		c[0][0] = s0, c[1][1] = s1, c[2][2] = s2;
		c[3][0] = t0, c[3][1] = t1, c[3][2] = t2;
		c[3][3] = 1;
	}

	friend UniMat operator*(const UniMat &m0, const UniMat &m1)
	{
		auto s0 = m0.c[0][0] * m1.c[0][0];
		auto s1 = m0.c[1][1] * m1.c[1][1];
		auto s2 = m0.c[2][2] * m1.c[2][2];

		auto t0 = m0.c[3][0] + m0.c[0][0] * m1.c[3][0];
		auto t1 = m0.c[3][1] + m0.c[1][1] * m1.c[3][1];
		auto t2 = m0.c[3][2] + m0.c[2][2] * m1.c[3][2];

		return UniMat(s0, s1, s2, t0, t1, t2);
	}

#ifdef MAINTENANCE
	void report(const char *str)
	{
		printf("%s:\n", str);
		for (auto i = 0; i < 4; i++) {
			printf("    %8.3f %8.3f %8.3f %8.3f\n", c[0][i], c[1][i], c[2][i], c[3][i]);
		}
	}

	friend vec3f_t operator*(const UniMat &m, const vec3f_t &v)
	{
		auto x = m.c[0][0] * v.x + m.c[3][0];
		auto y = m.c[1][2] * v.y + m.c[3][1];
		auto z = m.c[2][2] * v.z + m.c[3][2];
		return vec3f_t(x, y, z);
	}
#endif
};

struct Status {
	union Flags {
		struct {
			uint32_t fill      : 1 = 1;
			uint32_t blend     : 1 = 1;
			uint32_t cull_face : 1 = 0;
			uint32_t depth_test: 1 = 0;
		};
		uint32_t bits;
	};
	struct Blendfunc {
		uint32_t sc, dc, sa, da;
	};
	Flags flags = {
	        {1, 1, 0, 0}
        };
	Blendfunc blendfunc = {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA};
	uint32_t depthfunc = GL_LESS;
};

class Painter {
public:
	struct Vertex {
		float x, y, z, u, v, r, g, b;
	};

	Painter(const Attrs &attrs)
	{
		// array
		{
			Attrs attrs0 = {
			        {"shader_id",    ms_shaderId},
			        {"a.a_position", 3          },
			        {"a.a_texcoord", 2          },
			        {"a.a_color",    3          },
			};
			m_arrayId = spu_array_new(attrs0);

			UniMat unit;
			Attrs attrs1 = {
			        {"divisor",               1    },
			        {"a.a_instance_nodetext", 16   },
			        {"nelem",                 1    },
			        {"data",                  &unit},
			};
			spu_array_aux(m_arrayId, attrs1, 1);
		}

		// uniform
		{
			m_unif.init(ms_shaderId);
			m_unif.addUniform("u_textscreen", &u_textscreen);
			m_unif.addUniform("u_color", &u_color);
			m_unif.addUniform("u_font_texture", &ms_textureId);
			m_unif.addUniform("u_smoothstep", &u_smoothstep);
		}

		set(attrs);
	}

	void set(const Attrs &attrs)
	{
		auto origin = attrs.get("origin", vec2f_t(5, -15));
		auto color = attrs.get("color", vec3f_t(1, 1, 1));
		auto pitch = attrs.get("pitch", 10.0f);
		auto depth_test = attrs.get("depth_test", false);

		// renderstate
		{
			m_printStatus.flags.depth_test = depth_test;
		}

		// texscreen
		{
			auto pmod = [](float x, float m) { return std::fmod(std::fmod(x, m) + m, m); };

			vec4f_t viewport;
			spu_frame_get(-1, "viewport0", &viewport);

			origin.x = pmod(origin.x, viewport.sx); 
			origin.y = pmod(origin.y, viewport.sy);

			auto s0 = 2.0f / viewport.sx;
			auto s1 = 2.0f / viewport.sy;
			auto s2 = 1.0f;

			auto t0 = -1.0f;
			auto t1 = -1.0f;
			auto t2 = 0.0f;

			auto fragscreen = UniMat(s0, s1, s2, t0, t1, t2);
			auto textfrag = UniMat(pitch, pitch, 1, origin.x, origin.y, 0);

			u_textscreen = fragscreen * textfrag;
			u_smoothstep = 1.0f / (pitch * 1.2f);
			u_color = color;
		}
	}

	~Painter() { spu_array_delete(m_arrayId); }

	void begin()
	{
		m_position = {0, 0, 0};
		m_vertices.clear();
	}

	void puts(const char *text)
	{
		for (; *text; text++) {
			switch (*text) {
			case '\n': {
				m_position.x = 0;
				m_position.y -= 2;  // not scale
				break;
			}
			case '\t': {
				auto tab = 8;
				m_position.x = floor((m_position.x + tab) / tab) * tab;
				break;
			}
			default: {
				putc(*text);
				m_position.x += 1;
			}
			}
		}
	}

	void putc(const uint8_t c)
	{
		auto &c_font = c_font_Monospace;
		auto fs = fontScale();
		auto r = 1.0 / fs;

		auto info = charInfo(c);
		if (info.codePoint == 0) {
			m_position.x += 1;
			return;
		}

		auto aw = float(c_font.width);
		auto ah = float(c_font.height);

		auto cw = float(info.width);
		auto ch = float(info.height);

		auto ox = float(info.originX);
		auto oy = float(info.originY);

		auto x0 = m_position.x - ox * r;
		auto y0 = m_position.y - (ch - oy) * r;

		auto x1 = x0 + cw * r;
		auto y1 = y0 + ch * r;

		auto u0 = float(info.x);
		auto v1 = float(info.y);

		auto v0 = v1 + ch;
		auto u1 = u0 + cw;

		auto z = m_position.z;

		auto add_vertex = [&](float x, float y, float z, float u, float v) {
			// m_vertices.emplace_back(x, y, z, u, v, 1.0f, 1.0f, 1.0f);
			m_vertices.push_back({x, y, z, u, v, 1.0f, 1.0f, 1.0f});
		};

		add_vertex(x0, y0, z, u0 / aw, 1.0f - v0 / ah);
		add_vertex(x1, y0, z, u1 / aw, 1.0f - v0 / ah);
		add_vertex(x1, y1, z, u1 / aw, 1.0f - v1 / ah);
		add_vertex(x0, y1, z, u0 / aw, 1.0f - v1 / ah);
	}

	void end()
	{
		if (!m_vertices.empty()) {
			pushStatus();
			m_unif.use();
			spu_array_send(m_arrayId, m_vertices.data(), m_vertices.size(), 0);
			spu_array_draw(m_arrayId, GL_QUADS);
			popStatus();
		}
	}

	CharInfo charInfo(const int8_t c) const
	{
		auto &font = c_font_Monospace;
		for (auto i = 0; i < font.characterCount; i++) {
			auto &info = font.charInfos[i];
			if (info.codePoint == c) {
				return info;
			}
		}
		return {};
	}

	float fontScale() const
	{
		const float c_spacing = 2.0f;
		return c_font_Monospace.size / 2.0f + c_spacing;
	}

	inline static int32_t get(const hash32_t &key, void *value)
	{
		auto ret = 0;
		if ((ret = getvalue(key, value, "shader_id"_h32, ms_shaderId))) {
			return ret;
		}
		if ((ret = getvalue(key, value, "texture_id"_h32, ms_textureId))) {
			return ret;
		}
		if ((ret = getvalue(key, value, "renderstate_id"_h32, ms_renderstateId))) {
			return ret;
		}
		return ret;
	}

	inline static void startup()  // no shutdown. trust buddies.
	{
		//  texture
		{
			Attrs texture_attrs = {
			        {"iformat",     GL_R8},
			        {"max_level",   0    },
			        {"auto_mipmap", 0    },
			};
			File::embed(c_font_png_name, c_font_png_data, sizeof(c_font_png_data));
			ms_textureId = spu_inventory_new("texture", "font.png", texture_attrs);
		}

		// renderstate
		{
			Attrs attrs = {
			        {"flags.fill",       0                   },
			        {"flags.blend",      0                   },
			        {"flags.cull_face",  0                   },
			        {"flags.depth_test", 0                   },
			        {"flags",            &ms_status.flags    },
			        {"blend_func",       &ms_status.blendfunc},
			        {"depth_func",       &ms_status.depthfunc},
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

	uint32_t m_arrayId = 0;
	Uniform m_unif;

	std::vector<Vertex> m_vertices;
	Status m_initialStatus;
	Status m_printStatus;
	vec3f_t m_position = {0, 0, 0};

	UniMat u_textscreen;
	vec4f_t u_color = {1, 1, 1, 1};
	float u_smoothstep = 0.1;

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
}  // namespace spu::libspu::spu_print

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
