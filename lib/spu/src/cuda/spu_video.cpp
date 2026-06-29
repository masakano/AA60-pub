//
// Sender :
//
#include <spu/spu.h>

#include "ffmpeg.h"
#include "pipe.h"
#include "file_channel.h"
#include "udp_frame_channel.h"

namespace spu {
namespace libspu::video {

class EncodeHandle {
public:
	explicit EncodeHandle(const Attrs &attrs)
	{
		auto path = attrs.get<const char *>("path", nullptr);
		auto callback = attrs.get<Sender::callback_t>("callback", nullptr);
		if (path == nullptr || callback == nullptr) {
			attrs.report("encodeHandle");
			aux_error(true, "path or calllback isnullptr\n");
		}

		if (strcmp(path + strlen(path) - 4, ".mp4") == 0) {
			auto texture_id = attrs.get("texture_id", 0);
			assert(texture_id);
			m_ffmpeg = new FFMpegEncoder(path, texture_id);
		}
		else {
			bool is_use_udp = strchr(path, ':');

			if (is_use_udp) {
				m_sender = new UdpFrameSender(attrs);
			}
			else {
				m_sender = new FileSender();
			}
			m_sender->init(path, callback);
			m_encoder = new PipeEncoder(attrs, m_sender);
		}
	}

	~EncodeHandle()
	{
		delete m_encoder;
		delete m_sender;
		delete m_ffmpeg;
	}

	bool encode(const uint32_t *embed_value)
	{
		if (m_ffmpeg) {
			m_ffmpeg->encode();
			return true;
		}
		return m_encoder->encode(embed_value);
	}

private:
	Sender *m_sender = nullptr;
	PipeEncoder *m_encoder = nullptr;
	FFMpegEncoder *m_ffmpeg = nullptr;
};

class DecodeHandle {
public:
	explicit DecodeHandle(const Attrs &attrs)
	{
		auto path = attrs.get<const char *>("path", nullptr);
		auto callback = attrs.get<Receiver::callback_t>("callback", nullptr);
		assert(path && callback);

		if (strchr(path, ':')) {
			m_receiver = new UdpFrameReceiver();
		}
		else {
			m_receiver = new FileReciever();
		}
		m_receiver->init(path, callback);
		m_decoder = new PipeDecoder(attrs, m_receiver);
	}

	~DecodeHandle()
	{
		m_receiver->cancel();
		delete m_decoder;
		delete m_receiver;
	}

	bool decode(uint32_t *embed_value) { return m_decoder->decode(embed_value); }

private:
	Receiver *m_receiver = nullptr;
	PipeDecoder *m_decoder = nullptr;
};
}  // namespace libspu::video

// !! currently multiple stream does not supported !!
using namespace libspu;
using namespace libspu::video;

static EncodeHandle *s_encode_handle = nullptr;
static DecodeHandle *s_decode_handle = nullptr;

bool spu_video_enc_new(const Attrs &attrs)
{
	if (!s_encode_handle) {
		s_encode_handle = new EncodeHandle(attrs);
		return true;
	}
	return false;
}

bool spu_video_enc(const uint32_t *embed_value)
{
	if (s_encode_handle) {
		return s_encode_handle->encode(embed_value);
	}
	return false;
}

bool spu_video_enc_delete()
{
	if (s_encode_handle) {
		delete s_encode_handle;
		s_encode_handle = nullptr;
		printf("video_enc_done (true)..\n");
		return true;
	}
	return false;
}

bool spu_video_dec_new(const Attrs &attrs)
{
	if (!s_decode_handle) {
		s_decode_handle = new DecodeHandle(attrs);
		return true;
	}
	return false;
}

bool spu_video_dec(uint32_t *embed_value)
{
	if (s_decode_handle) {
		return s_decode_handle->decode(embed_value);
	}
	return false;
}

bool spu_video_dec_delete()
{
	if (s_decode_handle) {
		delete s_decode_handle;
		s_decode_handle = nullptr;
		return true;
	}
	return false;
}
}  // namespace spu
