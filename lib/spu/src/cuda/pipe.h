//
// Inventory :
//
#pragma once

#include <spu/spu.h>  // must be top
#include "channel.h"
#include <cuda_gl_interop.h>
#include <deque>
#include <future>
#include <unordered_map>
#include <vector>

class NvDecoder;
class NvEncoderCuda;
struct NvEncOutputFrame;

namespace spu::libspu::video {

#ifdef ck
#undef ck
#endif
#define ck(e) aux_error(e < 0, "General error code=%d\n", e);

class Inventory {
public:
	~Inventory();
	cudaGraphicsResource_t get(uint32_t texture_id, uint32_t flags);
	void erase(uint32_t texture_id);

private:
	std::unordered_map<uint32_t, cudaGraphicsResource_t> m_resources;
};

class Pipe {
public:
	static constexpr uint32_t c_bitrate = 32 * 1000 * 1000;
	static constexpr uint32_t c_target_fps = 60;

protected:
	class ScopedMappedArray {
	public:
		explicit ScopedMappedArray(Pipe &pipe);
		~ScopedMappedArray();

		ScopedMappedArray(const ScopedMappedArray &) = delete;
		ScopedMappedArray &operator=(const ScopedMappedArray &) = delete;

		cudaArray_t array() const { return m_array; }

	private:
		Pipe &m_pipe;
		cudaArray_t m_array = nullptr;
		bool m_isOwner = false;
	};

	hash32_t m_codec = "h264";
	Inventory m_inventory;

	cudaGraphicsResource_t m_resource = nullptr;
	uint32_t m_resourceFlag = 0;
	uint32_t m_textureId = 0;
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	bool m_isMapped = false;
	Pipe(const Attrs &attrs, uint32_t flag);
	virtual ~Pipe();
	void mapResource();
	void unmapResource();
};

class PipeEncoder : public Pipe {
public:
	PipeEncoder(const Attrs &attrs, Sender *sender);
	~PipeEncoder();

	bool encode(const uint32_t *embed_value);
	void restart();
	void setForceIFrame(bool is_force_iframe) { m_isForceIFrame = is_force_iframe; }

private:
	Sender *m_sender = nullptr;
	NvEncoderCuda *m_encoder = nullptr;
	uint32_t m_bitrate = 0;
	uint32_t m_targetFPS = 0;
	uint32_t m_restartCount = 0;
	bool m_isForceIFrame = false;
	std::future<bool> m_future;
	uint32_t m_inputSerial = 0;
	bool encode_background();
	bool send_encoded_frames(const std::vector<NvEncOutputFrame> &packets, const FrameMetadata &metadata);
	void alloc();
	void dispose();
};

class PipeDecoder : public Pipe {
public:
	PipeDecoder(const Attrs &attrs, Receiver *receiver);
	~PipeDecoder();
	bool decode(uint32_t *embed_value);

private:
	Receiver *m_receiver = nullptr;
	NvDecoder *m_decoder = nullptr;
	int64_t m_count = 0;
	void *m_deviceBuffer = nullptr;
	bool receive_bitstreams(std::deque<ReceivedFrame> *bitstreams);
	uint8_t *decode_one_frame(const ReceivedFrame &frame);
	void alloc();
	void dispose();
};
}  // namespace spu::libspu::video
