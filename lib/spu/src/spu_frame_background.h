//
// Backgrounds :
//
#pragma once
#include "spu_object.h"

namespace spu::libspu::spu_frame {

class Backgrounds final {
public:
	void activate(int32_t attach_point)
	{
		for (auto &bg_value: m_bgValues) {
			if (bg_value.attach_point == attach_point) {  // not target
				bg_value.is_attach = true;
				return;
			}
		}
		aux_error(true, "bad attach point (%s)\n", opengl_const(attach_point));
	}

	void set(const Attrs &attrs)
	{
		attrs.peek("bgdepth_stencil", "use 'bgdepth/bgstencil' instead");
		for (auto &bg_value: m_bgValues) {
			switch (bg_value.attach_point) {
			case GL_DEPTH_ATTACHMENT:
				bg_value.f = attrs.get<float>(bg_value.name, bg_value.f);
				m_dsValue.f = attrs.get<float>(bg_value.name, m_dsValue.f);
				break;
			case GL_STENCIL_ATTACHMENT:
				bg_value.i = attrs.get<int32_t>(bg_value.name, bg_value.i);
				m_dsValue.i = attrs.get<int32_t>(bg_value.name, m_dsValue.i);
				break;
			default: bg_value.fv = attrs.get<vec4f_t>(bg_value.name, bg_value.fv); break;
			}
		}
	}

	int32_t get(const hash32_t &key, void *value)  // return internal pointer
	{
		aux_error(key == "bgdepth_stencil", "use 'bgdepth/bgstencil' instead\n");

		for (auto &bg_value: m_bgValues) {
			if (key == bg_value.name) {
				switch (bg_value.attach_point) {
				case GL_DEPTH_ATTACHMENT: *(float *)value = bg_value.f; return sizeof(float *);
				case GL_STENCIL_ATTACHMENT:
					*(int32_t *)value = bg_value.i;
					return sizeof(int32_t **);
				default: *(vec4f_t *)value = bg_value.fv; return sizeof(vec4f_t *);
				}
			}
		}
		return 0;
	}

	void clear() const
	{
		for (const auto &bg_value: m_bgValues) {
			auto i = &bg_value - &m_bgValues[0];
			if (bg_value.is_attach) {
				switch (bg_value.attach_point) {
				case GL_DEPTH_ATTACHMENT:
					if (bg_value.f >= 0.0) {
						F(glClearBufferfv, GL_DEPTH, 0, &bg_value.f);
					}
					break;
				case GL_STENCIL_ATTACHMENT:
					if (bg_value.i >= 0) {
						F(glClearBufferiv, GL_STENCIL, 0, &bg_value.i);
					}
					break;
				case GL_DEPTH_STENCIL_ATTACHMENT:
					if (bg_value.f >= 0.0) {
						F(glClearBufferfi, GL_DEPTH_STENCIL, 0, bg_value.f, bg_value.i);
					}
					break;
				default: {
					auto &fv = bg_value.fv;
					if (fv.x >= 0.0 && fv.y >= 0.0 && fv.z >= 0.0) {
						F(glClearBufferfv, GL_COLOR, i, fv.f);
					}
					break;
				}
				}
			}
		}
	}

	void report()
	{
		for (auto &bg_value: m_bgValues) {
			const char ac = bg_value.is_attach ? 'a' : '-';
			switch (bg_value.attach_point) {
			case GL_DEPTH_ATTACHMENT:
				aux_printf("\t%-16s : %c %7.3f\n", bg_value.name.c_str(), ac, bg_value.f);
				break;
			case GL_STENCIL_ATTACHMENT:
				aux_printf("\t%-16s : %c  0x%04x\n", bg_value.name.c_str(), ac, bg_value.i);
				break;

			case GL_DEPTH_STENCIL_ATTACHMENT:
				aux_printf("\t%-16s : %c\n", bg_value.name.c_str(), ac);
				break;
			default:
				aux_printf(
				        "\t%-16s : %c %7.3f %7.3f %7.3f %7.3f\n", bg_value.name.c_str(), ac,
				        bg_value.fv.x, bg_value.fv.y, bg_value.fv.z, bg_value.fv.w);
				break;
			}
		}
	}

private:
	static constexpr auto c_bgValueCount = 13;  // color * 10 + depth + stencil + depth_stencil

	struct BgValue {
		hash32_t name;
		bool is_attach;
		int32_t attach_point;
		vec4f_t fv;
		float f;
		int32_t i;
	};

	BgValue m_bgValues[c_bgValueCount] = {
	        {"bgcolor0",        false, GL_COLOR_ATTACHMENT0,        {0, 0, 0, 0}, 1, 0},
	        {"bgcolor1",        false, GL_COLOR_ATTACHMENT1,        {0, 0, 0, 0}, 1, 0},
	        {"bgcolor2",        false, GL_COLOR_ATTACHMENT2,        {0, 0, 0, 0}, 1, 0},
	        {"bgcolor3",        false, GL_COLOR_ATTACHMENT3,        {0, 0, 0, 0}, 1, 0},
	        {"bgcolor4",        false, GL_COLOR_ATTACHMENT4,        {0, 0, 0, 0}, 1, 0},
	        {"bgcolor5",        false, GL_COLOR_ATTACHMENT5,        {0, 0, 0, 0}, 1, 0},
	        {"bgcolor6",        false, GL_COLOR_ATTACHMENT6,        {0, 0, 0, 0}, 1, 0},
	        {"bgcolor7",        false, GL_COLOR_ATTACHMENT7,        {0, 0, 0, 0}, 1, 0},
	        {"bgcolor8",        false, GL_COLOR_ATTACHMENT8,        {0, 0, 0, 0}, 1, 0},
	        {"bgcolor9",        false, GL_COLOR_ATTACHMENT9,        {0, 0, 0, 0}, 1, 0},
	        {"bgdepth",         false, GL_DEPTH_ATTACHMENT,         {1, 1, 1, 1}, 1, 0},
	        {"bgstencil",       false, GL_STENCIL_ATTACHMENT,       {0, 0, 0, 0}, 1, 0},
	        {"bgdepth_stencil", false, GL_DEPTH_STENCIL_ATTACHMENT, {1, 1, 1, 1}, 1, 0},
	};
	BgValue &m_dsValue = m_bgValues[c_bgValueCount - 1];  // depth_stencil
};
}  // namespace spu::libspu::spu_frame
