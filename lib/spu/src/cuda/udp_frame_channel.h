//
// UdpFrameChannel :
//
#pragma once

#include "channel.h"
#include <ssys/udp_socket.h>
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <mutex>
#include <string>
#include <thread>

namespace spu::libspu::video {

static constexpr uint32_t c_udp_frame_magic = 0x53505632;  // SPV2
static constexpr uint32_t c_udp_frame_version = 2;
static constexpr uint32_t c_udp_data_header_size = sizeof(uint32_t) * 11;
static constexpr uint32_t c_udp_control_header_size = sizeof(uint32_t) * 6;
static constexpr uint32_t c_udp_frame_payload_size = c_chunk_size - c_udp_data_header_size;
static constexpr uint32_t c_udp_control_payload_size = c_chunk_size - c_udp_control_header_size;
static constexpr uint32_t c_udp_frame_socket_buffer_size = 4 * 1024 * 1024;
static constexpr uint32_t c_udp_frame_event_log_count = 8;
static constexpr uint32_t c_udp_frame_missing_log_count = 16;
static constexpr uint32_t c_udp_frame_queue_capacity = 4;
static constexpr uint32_t c_udp_frame_drop_permille_max = 1000;

struct UdpFramePacket {
	uint32_t magic = c_udp_frame_magic;
	uint32_t version = c_udp_frame_version;
	uint32_t session_id = 0;
	uint32_t frame_id = 0;
	uint32_t control_id = 0;
	uint32_t input_serial = 0;
	uint32_t fragment_index = 0;
	uint32_t fragment_count = 0;
	uint32_t frame_size = 0;
	uint32_t flags = 0;
	uint32_t payload_size = 0;
	uint8_t payload[c_udp_frame_payload_size];
};

static_assert(sizeof(UdpFramePacket) == c_chunk_size);

struct UdpControlPacket {
	enum Type : uint32_t {
		e_start = 1,
		e_stop = 2,
		e_control = 3,
		e_start_ack = 4,
		e_key_frame_request = 5,
	};

	uint32_t magic = c_udp_frame_magic;
	uint32_t version = c_udp_frame_version;
	uint32_t type = e_control;
	uint32_t session_id = 0;
	uint32_t control_id = 0;
	uint32_t payload_size = 0;
	uint8_t payload[c_udp_control_payload_size];
};

static_assert(sizeof(UdpControlPacket) == c_chunk_size);

static bool is_udp_control_type(uint32_t type)
{
	return type == UdpControlPacket::e_start || type == UdpControlPacket::e_stop
	    || type == UdpControlPacket::e_control || type == UdpControlPacket::e_start_ack
	    || type == UdpControlPacket::e_key_frame_request;
}

class UdpFrameSender : public Sender {
public:
	explicit UdpFrameSender(const Attrs &attrs)
	{
		m_isControlClockEnabled = attrs.get("control_clock", 0) != 0;
	}
	~UdpFrameSender() override
	{
		cancel();
		if (m_thread) {
			m_thread->join();
			delete m_thread;
		}
		m_sock.close();
	}

	void init(const char *desc, callback_t callback) override
	{
		auto addr_and_port = UdpSocket::parse(desc);
		m_sock.init(addr_and_port.port);
		m_sock.extend(c_udp_frame_socket_buffer_size, c_udp_frame_socket_buffer_size);
		m_bindAddr = UdpSocket::make_sockaddr_in(addr_and_port);
		m_addr = m_bindAddr;
		m_callback = callback;
		initDropInjector();
		m_thread = new std::thread([&] { backgroundLoop(); });
	}

	void cancel() override
	{
		if (!isCancel()) {
			Sender::cancel();
			sendStopToBoundSocket();
		}
	}

	uint32_t sampleControlId() override { return getLastControlId(); }

	bool consumeKeyFrameRequest() override { return m_isKeyFrameRequested.exchange(false); }

	bool waitControlClock() override
	{
		if (m_isControlClockEnabled == false) {
			return true;
		}

		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_isControlClockPrimed == false) {
			m_lastSentControlId = m_lastControlId;
			m_isControlClockPrimed = true;
			return true;
		}

		auto is_updated = [&] {
			return isCancel() || isPause() || m_lastControlId != m_lastSentControlId
			    || m_isKeyFrameRequested.load();
		};
		m_controlCv.wait(lock, is_updated);
		if (isCancel() || isPause()) {
			return false;
		}
		m_lastSentControlId = m_lastControlId;
		return true;
	}

	bool sendFrame(const void *data, uint32_t size) override
	{
		FrameMetadata metadata;
		metadata.control_id = sampleControlId();
		return sendFrame(data, size, metadata);
	}

	bool sendFrame(const void *data, uint32_t size, const FrameMetadata &metadata) override
	{
		if (isCancel() || isPause()) {
			logInterruptedFrame(size);
			return false;
		}
		auto bytes = static_cast<const uint8_t *>(data);
		auto fragment_count
		        = std::max(1u, (size + c_udp_frame_payload_size - 1) / c_udp_frame_payload_size);
		auto frame_id = m_frameId++;
		if (metadata.isKeyFrame()) {
			aux_message(
			        0, "udp v2 send key frame: session=%u frame=%u control=%u size=%u\n",
			        m_sessionId, frame_id, metadata.control_id, size);
		}
		for (auto fragment_index = 0u; fragment_index < fragment_count; ++fragment_index) {
			if (isCancel() || isPause()) {
				logInterruptedFrame(size);
				return false;
			}
			auto offset = fragment_index * c_udp_frame_payload_size;
			auto payload_size = std::min(size - offset, c_udp_frame_payload_size);

			UdpFramePacket packet;
			packet.session_id = m_sessionId;
			packet.frame_id = frame_id;
			packet.control_id = metadata.control_id;
			packet.input_serial = metadata.input_serial;
			packet.flags = metadata.flags;
			packet.fragment_index = fragment_index;
			packet.fragment_count = fragment_count;
			packet.frame_size = size;
			packet.payload_size = payload_size;
			if (payload_size) {
				memcpy(packet.payload, bytes + offset, payload_size);
			}

			if (shouldDropPacket(frame_id, fragment_index)) {
				logDroppedPacket(frame_id, fragment_index, fragment_count);
				continue;
			}
			if (sendPacket(packet) < 0) {
				return false;
			}
		}
		return true;
	}

private:
	UdpSocket m_sock;
	sockaddr_in m_addr = {};
	sockaddr_in m_bindAddr = {};
	std::thread *m_thread = nullptr;
	std::mutex m_mutex;
	std::condition_variable m_controlCv;
	callback_t m_callback = nullptr;
	uint32_t m_sessionId = 0;
	uint32_t m_frameId = 0;
	uint32_t m_lastControlId = 0;
	uint32_t m_lastSentControlId = 0;
	uint32_t m_dropPermille = 0;
	uint32_t m_dropSeed = 0;
	uint32_t m_droppedPacketCount = 0;
	uint32_t m_interruptedFrameCount = 0;
	bool m_isControlClockEnabled = false;
	bool m_isControlClockPrimed = false;
	std::atomic_bool m_isKeyFrameRequested = false;

	void logInterruptedFrame(uint32_t size)
	{
		if (m_interruptedFrameCount < c_udp_frame_event_log_count) {
			aux_message(
			        0,
			        "udp v2 interrupt frame: interrupt=%u session=%u size=%u pause=%d cancel=%d\n",
			        m_interruptedFrameCount, m_sessionId, size, isPause(), isCancel());
		}
		m_interruptedFrameCount++;
	}

	void initDropInjector()
	{
		m_dropPermille = readEnvUInt("SPU_UDP_V2_DROP_PERMILLE", 0);
		m_dropPermille = std::min(m_dropPermille, c_udp_frame_drop_permille_max);
		m_dropSeed = readEnvUInt("SPU_UDP_V2_DROP_SEED", uint32_t(get_microsec()));
		if (m_dropPermille) {
			aux_message(
			        0, "udp v2 stress drop enabled: permille=%u seed=%u\n", m_dropPermille,
			        m_dropSeed);
		}
	}

	static uint32_t readEnvUInt(const char *name, uint32_t default_value)
	{
		auto *value = std::getenv(name);
		if (value == nullptr || *value == 0) {
			return default_value;
		}
		return uint32_t(std::strtoul(value, nullptr, 10));
	}

	bool shouldDropPacket(uint32_t frame_id, uint32_t fragment_index) const
	{
		if (m_dropPermille == 0) {
			return false;
		}

		auto x = frame_id * 1103515245u + fragment_index * 12345u + m_dropSeed;
		x ^= x >> 16;
		x *= 2246822519u;
		x ^= x >> 13;
		return (x % c_udp_frame_drop_permille_max) < m_dropPermille;
	}

	void logDroppedPacket(uint32_t frame_id, uint32_t fragment_index, uint32_t fragment_count)
	{
		if (m_droppedPacketCount < c_udp_frame_event_log_count * 4) {
			aux_message(
			        0, "udp v2 stress drop: drop=%u frame=%u fragment=%u/%u permille=%u\n",
			        m_droppedPacketCount, frame_id, fragment_index, fragment_count, m_dropPermille);
		}
		m_droppedPacketCount++;
	}

	uint32_t getLastControlId()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_lastControlId;
	}

	int32_t sendPacket(const UdpFramePacket &packet)
	{
		auto packet_size = c_udp_data_header_size + packet.payload_size;
		sockaddr_in addr = {};
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			addr = m_addr;
		}
		return int32_t(m_sock.sendto(addr, &packet, packet_size));
	}

	void sendStopToBoundSocket()
	{
		UdpSocket sock;
		sock.init(0);
		UdpControlPacket packet;
		packet.type = UdpControlPacket::e_stop;
		sock.sendto(m_bindAddr, &packet, c_udp_control_header_size);
	}

	bool isValidControl(const UdpControlPacket &packet, int32_t packet_size) const
	{
		return packet_size >= int32_t(c_udp_control_header_size) && packet.magic == c_udp_frame_magic
		    && packet.version == c_udp_frame_version
		    && packet.payload_size <= c_udp_control_payload_size
		    && packet_size == int32_t(c_udp_control_header_size + packet.payload_size)
		    && is_udp_control_type(packet.type);
	}

	void sendStartAck(const sockaddr_in &addr, const UdpControlPacket &start_packet)
	{
		UdpControlPacket packet;
		packet.type = UdpControlPacket::e_start_ack;
		packet.session_id = m_sessionId;
		packet.control_id = start_packet.control_id;
		m_sock.sendto(addr, &packet, c_udp_control_header_size);
	}

	void backgroundLoop()
	{
		while (!isCancel()) {
			UdpControlPacket packet;
			sockaddr_in addr = {};
			auto packet_size = int32_t(m_sock.recvfrom(addr, &packet, sizeof(packet)));
			if (packet_size <= 0 || !isValidControl(packet, packet_size)) {
				continue;
			}
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_addr = addr;
				if (packet.type == UdpControlPacket::e_start
				    || packet.type == UdpControlPacket::e_control) {
					m_lastControlId = packet.control_id;
				}
			}
			if (packet.type == UdpControlPacket::e_start
			    || packet.type == UdpControlPacket::e_control) {
				m_controlCv.notify_all();
			}

			if (packet.payload_size && m_callback
			    && (packet.type == UdpControlPacket::e_start
			        || packet.type == UdpControlPacket::e_control)) {
				(*m_callback)(&packet.payload_size);
			}

			switch (packet.type) {
			case UdpControlPacket::e_start:
				if (packet.session_id != m_sessionId || isPause()) {
					aux_message(
					        0, "start v2: session=%u control=%u\n", packet.session_id,
					        packet.control_id);
					m_sessionId = packet.session_id;
					m_frameId = 0;
					m_isControlClockPrimed = false;
					m_isKeyFrameRequested = true;
					resume();
				}
				sendStartAck(addr, packet);
				break;
			case UdpControlPacket::e_stop:
				aux_message(0, "stop v2...\n");
				pause();
				m_controlCv.notify_all();
				break;
			case UdpControlPacket::e_control: break;
			case UdpControlPacket::e_key_frame_request:
				aux_message(
				        0, "udp v2 key frame request received: control=%u\n",
				        packet.control_id);
				m_isKeyFrameRequested = true;
				m_controlCv.notify_all();
				break;
			default: break;
			}
		}
	}
};

class UdpFrameReceiver : public Receiver {
public:
	~UdpFrameReceiver() override
	{
		cancel();
		if (m_thread) {
			m_thread->join();
			delete m_thread;
		}
		m_sock.close();
	}

	void init(const char *desc, callback_t callback) override
	{
		auto addr_and_port = UdpSocket::parse(desc);
		m_sock.init(0);
		m_sock.extend(c_udp_frame_socket_buffer_size, c_udp_frame_socket_buffer_size);
		m_sock.timeout(c_timeout_msec);
		m_remoteAddr = UdpSocket::make_sockaddr_in(addr_and_port);
		m_callback = callback;
		m_sessionId = uint32_t(get_microsec());
		resume();
		m_thread = new std::thread([&] { backgroundLoop(); });
		sendControl(UdpControlPacket::e_start);
	}

	void cancel() override
	{
		if (!isCancel()) {
			sendControl(UdpControlPacket::e_stop, false);
		}
		Receiver::cancel();
		m_queueCv.notify_all();
	}

	bool receiveFrame(std::vector<uint8_t> *frame) override
	{
		ReceivedFrame received_frame;
		if (receiveFrame(&received_frame) == false) {
			return false;
		}
		*frame = std::move(received_frame.bitstream);
		return true;
	}

	bool receiveFrame(ReceivedFrame *frame) override
	{
		std::deque<ReceivedFrame> frames;
		if (receiveFrames(&frames) == false) {
			return false;
		}
		*frame = std::move(frames.back());
		return true;
	}

	bool receiveFrames(std::deque<std::vector<uint8_t>> *frames) override
	{
		frames->clear();
		std::deque<ReceivedFrame> received_frames;
		if (receiveFrames(&received_frames) == false) {
			return false;
		}
		while (!received_frames.empty()) {
			frames->push_back(std::move(received_frames.front().bitstream));
			received_frames.pop_front();
		}
		return true;
	}

	bool receiveFrames(std::deque<ReceivedFrame> *frames) override
	{
		frames->clear();
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_queueCv.wait(lock, [&] { return isCancel() || !m_frames.empty(); });
		if (m_frames.empty()) {
			return false;
		}
		while (!m_frames.empty()) {
			frames->push_back(std::move(m_frames.front()));
			m_frames.pop_front();
		}
		lock.unlock();
		sendControl(UdpControlPacket::e_control);
		return true;
	}

private:
	UdpSocket m_sock;
	sockaddr_in m_remoteAddr = {};
	callback_t m_callback = nullptr;
	std::thread *m_thread = nullptr;
	std::mutex m_controlMutex;
	std::mutex m_queueMutex;
	std::condition_variable m_queueCv;
	std::deque<ReceivedFrame> m_frames;
	uint32_t m_sessionId = 0;
	uint32_t m_frameId = 0;
	uint32_t m_frameSize = 0;
	uint32_t m_fragmentCount = 0;
	uint32_t m_receivedCount = 0;
	uint32_t m_dropCount = 0;
	uint32_t m_queueDropCount = 0;
	uint32_t m_controlId = 0;
	uint32_t m_lastControlPayloadSize = 0;
	uint32_t m_startControlId = 0;
	bool m_isStartAcked = false;
	bool m_isFrameSynced = false;
	uint64_t m_lastKeyFrameRequestUsec = 0;
	uint8_t m_lastControlPayload[c_udp_control_payload_size] = {};
	std::vector<uint8_t> m_frame;
	FrameMetadata m_frameMetadata;
	std::vector<uint8_t> m_receivedFragments;

	bool isControlPayloadChanged(const UdpControlPacket &packet) const
	{
		if (packet.payload_size != m_lastControlPayloadSize) {
			return true;
		}
		return packet.payload_size
		    && memcmp(packet.payload, m_lastControlPayload, packet.payload_size) != 0;
	}

	void rememberControlPayload(const UdpControlPacket &packet)
	{
		m_lastControlPayloadSize = packet.payload_size;
		if (packet.payload_size) {
			memcpy(m_lastControlPayload, packet.payload, packet.payload_size);
		}
	}

	bool sendControl(
	        uint32_t type, bool is_callback_enabled = true, bool is_payload_change_required = false)
	{
		std::lock_guard<std::mutex> lock(m_controlMutex);

		UdpControlPacket packet;
		packet.type = type;
		packet.session_id = m_sessionId;

		auto is_payload_control = packet.type == UdpControlPacket::e_start
		                       || packet.type == UdpControlPacket::e_control;
		if (is_payload_control && is_callback_enabled && m_callback) {
			(*m_callback)(&packet.payload_size);
			assert(packet.payload_size <= c_udp_control_payload_size);
		}
		if (is_payload_control && is_payload_change_required
		    && isControlPayloadChanged(packet) == false) {
			return false;
		}
		if (is_payload_control) {
			rememberControlPayload(packet);
		}
		packet.control_id = m_controlId++;
		if (packet.type == UdpControlPacket::e_start) {
			m_startControlId = packet.control_id;
			m_isStartAcked = false;
			m_isFrameSynced = false;
			m_lastKeyFrameRequestUsec = 0;
		}
		auto packet_size = c_udp_control_header_size + packet.payload_size;
		m_sock.sendto(m_remoteAddr, &packet, packet_size);
		return true;
	}

	void requestKeyFrame(const char *reason, uint32_t frame_id)
	{
		auto now_usec = get_microsec();
		if (m_lastKeyFrameRequestUsec && now_usec - m_lastKeyFrameRequestUsec < c_timeout_msec * 1000) {
			return;
		}
		m_lastKeyFrameRequestUsec = now_usec;
		aux_message(0, "udp v2 key frame request: reason=%s frame=%u\n", reason, frame_id);
		sendControl(UdpControlPacket::e_key_frame_request, false);
	}

	void loseFrameSync(const char *reason, uint32_t frame_id)
	{
		m_isFrameSynced = false;
		requestKeyFrame(reason, frame_id);
	}

	bool isStartAcked()
	{
		std::lock_guard<std::mutex> lock(m_controlMutex);
		return m_isStartAcked;
	}

	bool isFrameQueueEmpty()
	{
		std::lock_guard<std::mutex> lock(m_queueMutex);
		return m_frames.empty();
	}

	void handleReceiveTimeout()
	{
		if (isStartAcked()) {
			if (isFrameQueueEmpty()) {
				sendControl(UdpControlPacket::e_control);
			}
		}
		else {
			sendControl(UdpControlPacket::e_start);
		}
	}

	bool isValidControl(const UdpControlPacket &packet, int32_t packet_size) const
	{
		return packet_size >= int32_t(c_udp_control_header_size) && packet.magic == c_udp_frame_magic
		    && packet.version == c_udp_frame_version && packet.session_id == m_sessionId
		    && packet.payload_size <= c_udp_control_payload_size
		    && packet_size == int32_t(c_udp_control_header_size + packet.payload_size)
		    && is_udp_control_type(packet.type);
	}

	void acceptStartAck(const UdpControlPacket &packet)
	{
		std::lock_guard<std::mutex> lock(m_controlMutex);
		if (packet.control_id != m_startControlId) {
			return;
		}
		if (m_isStartAcked == false) {
			aux_message(
			        0, "udp v2 start ack: session=%u control=%u\n", packet.session_id,
			        packet.control_id);
		}
		m_isStartAcked = true;
	}

	void acceptControl(const UdpControlPacket &packet)
	{
		switch (packet.type) {
		case UdpControlPacket::e_start_ack: acceptStartAck(packet); break;
		default: break;
		}
	}

	void backgroundLoop()
	{
		while (!isCancel()) {
			uint8_t packet_buffer[c_chunk_size] = {};
			sockaddr_in recv_addr = {};
			auto packet_size
			        = int32_t(m_sock.recvfrom(recv_addr, packet_buffer, sizeof(packet_buffer)));
			if (packet_size <= 0) {
				handleReceiveTimeout();
				continue;
			}

			UdpControlPacket control_packet = {};
			memcpy(&control_packet, packet_buffer, size_t(packet_size));
			if (isValidControl(control_packet, packet_size)) {
				acceptControl(control_packet);
				continue;
			}

			UdpFramePacket packet = {};
			memcpy(&packet, packet_buffer, size_t(packet_size));
			if (!isValidData(packet, packet_size)) {
				continue;
			}
			acceptData(packet);
		}
	}

	bool isValidData(const UdpFramePacket &packet, int32_t packet_size) const
	{
		if (packet_size < int32_t(c_udp_data_header_size) || packet.magic != c_udp_frame_magic
		    || packet.version != c_udp_frame_version || packet.session_id != m_sessionId
		    || packet.payload_size > c_udp_frame_payload_size
		    || packet_size != int32_t(c_udp_data_header_size + packet.payload_size)) {
			return false;
		}
		if (packet.fragment_count == 0 || packet.fragment_index >= packet.fragment_count) {
			return false;
		}
		auto offset = packet.fragment_index * c_udp_frame_payload_size;
		return offset + packet.payload_size <= packet.frame_size;
	}

	void logIncompleteFrameDrop(const UdpFramePacket &next_packet)
	{
		std::string missing;
		auto missing_count = uint32_t(0);
		for (auto i = 0u; i < m_fragmentCount; ++i) {
			if (m_receivedFragments[i]) {
				continue;
			}
			missing_count++;
			if (missing_count <= c_udp_frame_missing_log_count) {
				missing += " ";
				missing += std::to_string(i);
			}
		}
		aux_message(
		        0,
		        "udp v2 drop incomplete frame: drop=%u frame=%u received=%u/%u missing=%u%s%s next_frame=%u next_fragment=%u\n",
		        m_dropCount++, m_frameId, m_receivedCount, m_fragmentCount, missing_count,
		        missing.c_str(), missing_count > c_udp_frame_missing_log_count ? " ..." : "",
		        next_packet.frame_id, next_packet.fragment_index);
		loseFrameSync("incomplete", m_frameId);
	}

	void startFrame(const UdpFramePacket &packet)
	{
		m_frameId = packet.frame_id;
		m_frameSize = packet.frame_size;
		m_fragmentCount = packet.fragment_count;
		m_receivedCount = 0;
		m_frameMetadata.frame_id = packet.frame_id;
		m_frameMetadata.control_id = packet.control_id;
		m_frameMetadata.input_serial = packet.input_serial;
		m_frameMetadata.flags = packet.flags;
		m_frame.assign(m_frameSize, 0);
		m_receivedFragments.assign(m_fragmentCount, 0);
	}

	void pushFrame()
	{
		auto is_key_frame = m_frameMetadata.isKeyFrame();
		bool is_queue_cleared = false;
		uint32_t dropped_frame_id = 0;
		size_t dropped_queue_size = 0;
		{
			std::lock_guard<std::mutex> lock(m_queueMutex);
			if (m_frames.size() >= c_udp_frame_queue_capacity) {
				is_queue_cleared = true;
				dropped_frame_id = m_frames.front().metadata.frame_id;
				dropped_queue_size = m_frames.size();
				m_frames.clear();
			}
		}
		if (is_queue_cleared) {
			aux_message(
			        0, "udp v2 queue drop: drop=%u frame=%u queued=%zu capacity=%u\n",
			        m_queueDropCount++, dropped_frame_id, dropped_queue_size,
			        c_udp_frame_queue_capacity);
			if (is_key_frame == false) {
				loseFrameSync("queue", dropped_frame_id);
				return;
			}
		}

		if (is_key_frame) {
			if (m_isFrameSynced == false) {
				aux_message(0, "udp v2 frame sync: frame=%u\n", m_frameMetadata.frame_id);
			}
			m_isFrameSynced = true;
			m_lastKeyFrameRequestUsec = 0;
		}
		else if (m_isFrameSynced == false) {
			requestKeyFrame("sync", m_frameMetadata.frame_id);
			return;
		}

		{
			std::lock_guard<std::mutex> lock(m_queueMutex);
			ReceivedFrame frame;
			frame.bitstream = std::move(m_frame);
			frame.metadata = m_frameMetadata;
			m_frames.push_back(std::move(frame));
		}
		m_queueCv.notify_one();
	}

	void acceptData(const UdpFramePacket &packet)
	{
		if (m_receivedFragments.empty() || int32_t(packet.frame_id - m_frameId) > 0) {
			if (!m_receivedFragments.empty() && m_receivedCount != m_fragmentCount) {
				logIncompleteFrameDrop(packet);
			}
			startFrame(packet);
		}
		else if (packet.frame_id != m_frameId) {
			aux_message(
			        1, "udp v2 stale fragment: current_frame=%u packet_frame=%u fragment=%u\n",
			        m_frameId, packet.frame_id, packet.fragment_index);
			return;
		}
		if (packet.frame_size != m_frameSize || packet.fragment_count != m_fragmentCount) {
			return;
		}
		if (m_receivedFragments[packet.fragment_index]) {
			aux_message(
			        1, "udp v2 duplicate fragment: frame=%u fragment=%u\n", packet.frame_id,
			        packet.fragment_index);
			return;
		}

		auto offset = packet.fragment_index * c_udp_frame_payload_size;
		memcpy(m_frame.data() + offset, packet.payload, packet.payload_size);
		m_receivedFragments[packet.fragment_index] = 1;
		m_receivedCount++;
		if (m_receivedCount == m_fragmentCount) {
			pushFrame();
			m_receivedFragments.clear();
		}
	}
};
}  // namespace spu::libspu::video
