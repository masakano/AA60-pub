//
// Capture :
//
#pragma once
#include <spu/spu.h>

namespace spu {

// #define USE_ENCODER

#ifndef USE_ENCODER

class Capture {
public:
	Capture(const Vec4i &) {}
	void begin() {}
	void end() {}
};

#else
class Capture {
public:
	explicit Capture(const Vec4i &window)
	{
		Vec4f viewport = {0.0f, 0.0f, float(window.sx), float(window.sy)};  // float

		Attrs color_attrs = {
		        {"target",     GL_TEXTURE_2D},
                        {"iformat",    GL_RGBA8     },
                        {"width",      window.sx    },
		        {"height",     window.sy    },
                        {"max_levvel", 0            },
		};
		m_color_id = spu_texture_new(color_attrs);

		Attrs depth_attrs = {
		        {"target",  GL_RENDERBUFFER      },
		        {"iformat", GL_DEPTH_COMPONENT32F},
		        {"width",   window.sx            },
		        {"height",  window.sy            },
		};
		m_depth_id = spu_texture_new(depth_attrs);

		Attrs frame_attrs = {
		        {"viewport0", viewport.fv},
		        {"color0",    m_color_id },
		        {"epth",      m_depth_id },
		};
		m_frame_id = spu_frame_new(frame_attrs);

		Attrs encoder_attrs = {
		        {"path",         "127.0.0.1:12345|127.0.0.1:12346"},
		        {"callback",     nullptr                          },
		        {"callback_arg", nullptr                          },
		};
		m_encode_id = spu_video_enc_new(encoder_attrs);
	}

	void begin() { spu_frame_begin(m_frame_id, 0); }

	void end()
	{
		spu_frame_end();
		spu_video_enc(m_encode_id, m_color_id);
	}

	~Capture()
	{
		spu_frame_delete(m_frame_id);
		spu_texture_delete(m_color_id);
		spu_texture_delete(m_depth_id);
	}

private:
	u_int m_frame_id = 0;
	u_int m_color_id = 0;
	u_int m_depth_id = 0;
	u_int m_encode_id = 0;
};
#endif
}  // namespace spu
