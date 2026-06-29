//
// SenderTweakbar :
//
#include "base_app.h"
#include "ffmpeg_frame_reader.h"
#include <gsys/canvas/copy.h>
#include <algorithm>
#include <cstdio>
#include <spu++/spu_backoffice.h>

namespace spu {

class SenderApp : public BaseApp {
public:
	explicit SenderApp(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;

private:
	gs_canvas::Copy m_copy;
	SpuTexture m_texture;
	std::vector<RGBA8> m_pixels;
	FFMpegFrameReader m_reader;
	uint32_t m_embedValue = 0;
	uint32_t m_controlSerial = 0;
	uint64_t m_usec = 0;
};

void SenderApp::init(const Attrs &attrs)
{
	GsPage::init(attrs);

	Attrs texture_attrs = {
	        {"iformat",     GL_SRGB8_ALPHA8},
                {"width",       c_width        },
                {"height",      c_height       },
                {"max_level",   0              },
	        {"auto_mipmap", 0              },
	};
	m_texture.init(texture_attrs);
	m_pixels.resize(c_width * c_height, {0xff, 0xff, 0xff, 0xff});
	m_reader.init(paths, c_width, c_height);

	Attrs canvas_attrs = {
	        {"use_unif_block", false         },
	        {"def_vflip",      1             },
	        {"path",           "flip_copy.us"},
	};
	m_copy.init(canvas_attrs);
	m_copy.u_color0 = m_texture.id();

	auto *backoffice = getBackoffice();
	backoffice->setSenderEmbedValuePtr(&m_embedValue);
	m_usec = get_microsec();
}

void SenderApp::render()
{
	constexpr auto c_min_fps = 1u;
	constexpr auto c_max_fps = 240u;
	constexpr auto c_heartbeat_fps = 2u;

	auto control = getSenderControl();
	if (m_controlSerial != control.serial) {
		m_controlSerial = control.serial;
		printf("sender apply control: serial=%u enable=%u fps=%u flags=%u\n", control.serial,
		       control.enable, control.fps, control.flags);
	}

	auto fps = std::clamp(control.fps, c_min_fps, c_max_fps);
	if (control.enable == 0) {
		fps = c_heartbeat_fps;
	}
	auto frame_usec = uint64_t(1000000 / fps);

	auto dusec = get_microsec() - m_usec;
	if (dusec < frame_usec) {
		sleep_microsec(frame_usec - dusec);
	}
	m_usec = get_microsec();

	auto embed_value = m_reader.readFrame(m_pixels.data());
	if (embed_value != FFMpegFrameReader::c_invalid_serial) {
		m_embedValue = embed_value;
	}
	m_texture.send(m_pixels.data(), GL_SRGB8_ALPHA8);
	m_copy.render();
}

static ObjectRegistry<GsCanvas>::Creator<SenderApp> page_creator("sender");
}  // namespace spu
