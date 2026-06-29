//
// Channel :
//
#pragma once

#include <ssys/ssys.h>
#include <atomic>
#include <cstddef>
#include <deque>
#include <utility>
#include <vector>

namespace spu::libspu::video {

static constexpr size_t c_chunk_size = 1024;
static constexpr uint32_t c_timeout_msec = 4 * 16;

class Channel {
public:
	using callback_t = void (*)(void *);

	virtual ~Channel() = default;

	virtual void init(const char *desc, callback_t callback) = 0;
	virtual void resume()
	{
		m_isPause = false;
		m_isCancel = false;
	}
	virtual void pause() { m_isPause.store(true); }
	virtual void cancel() { m_isCancel.store(true); }

	bool isCancel() const { return m_isCancel.load(); }
	bool isPause() const { return m_isPause.load(); }

private:
	std::atomic_bool m_isCancel = false;
	std::atomic_bool m_isPause = true;
};

struct FrameMetadata {
	enum : uint32_t {
		e_key_frame = 1 << 0,
	};

	uint32_t frame_id = 0;
	uint32_t control_id = 0;
	uint32_t input_serial = 0;
	uint32_t flags = 0;

	bool isKeyFrame() const { return (flags & e_key_frame) != 0; }
};

struct ReceivedFrame {
	std::vector<uint8_t> bitstream;
	FrameMetadata metadata;
};

class Receiver : public Channel {
public:
	virtual bool receiveFrame(std::vector<uint8_t> *frame) = 0;
	virtual bool receiveFrame(ReceivedFrame *frame)
	{
		frame->metadata = {};
		return receiveFrame(&frame->bitstream);
	}
	virtual bool receiveFrames(std::deque<std::vector<uint8_t>> *frames)
	{
		frames->clear();

		std::vector<uint8_t> frame;
		if (receiveFrame(&frame) == false) {
			return false;
		}
		frames->push_back(std::move(frame));
		return true;
	}
	virtual bool receiveFrames(std::deque<ReceivedFrame> *frames)
	{
		frames->clear();

		ReceivedFrame frame;
		if (receiveFrame(&frame) == false) {
			return false;
		}
		frames->push_back(std::move(frame));
		return true;
	}
};

class Sender : public Channel {
public:
	virtual uint32_t sampleControlId() { return 0; }
	virtual bool consumeKeyFrameRequest() { return false; }
	virtual bool waitControlClock() { return true; }
	virtual bool sendFrame(const void *data, uint32_t size) = 0;
	virtual bool sendFrame(const void *data, uint32_t size, const FrameMetadata &)
	{
		return sendFrame(data, size);
	}
};

}  // namespace spu::libspu::video
