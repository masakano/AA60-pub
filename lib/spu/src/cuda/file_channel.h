//
// FileChannel :
//
#pragma once

#include "channel.h"

namespace spu::libspu::video {

static constexpr uint32_t c_file_frame_magic = 0x53504652;  // SPFR
static constexpr uint32_t c_file_frame_legacy_version = 1;
static constexpr uint32_t c_file_frame_metadata_version = 2;
static constexpr uint32_t c_file_frame_version = 3;

struct FileFrameLegacyHeader {
	uint32_t magic = c_file_frame_magic;
	uint32_t version = c_file_frame_version;
	uint32_t frame_id = 0;
	uint32_t frame_size = 0;
};

static_assert(sizeof(FileFrameLegacyHeader) == sizeof(uint32_t) * 4);

struct FileFrameHeader : public FileFrameLegacyHeader {
	uint32_t control_id = 0;
	uint32_t input_serial = 0;
	uint32_t flags = 0;
};

static_assert(sizeof(FileFrameHeader) == sizeof(uint32_t) * 7);

class FileSender : public Sender {
public:
	void init(const char *desc, callback_t) override
	{
		printf("FileSender: open %s\n", desc);
		m_file.open(desc, "wb");  // auto close in destructor
		resume();
	}

	bool sendFrame(const void *data, uint32_t size) override
	{
		FrameMetadata metadata;
		return sendFrame(data, size, metadata);
	}

	bool sendFrame(const void *data, uint32_t size, const FrameMetadata &metadata) override
	{
		FileFrameHeader header;
		header.frame_id = m_frameId++;
		header.frame_size = size;
		header.control_id = metadata.control_id;
		header.input_serial = metadata.input_serial;
		header.flags = metadata.flags;
		m_file.write(&header, sizeof(header));
		if (size) {
			m_file.write(data, size);
		}
		return true;
	}

private:
	File m_file;
	uint32_t m_frameId = 0;
};

class FileReciever : public Receiver {
public:
	void init(const char *desc, callback_t) override
	{
		m_file.open(desc, "rb");  // auto close in destructor
		resume();
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
		frame->bitstream.clear();
		frame->metadata = {};

		FileFrameLegacyHeader header;
		auto header_size = m_file.read(&header, sizeof(header), false);
		if (header_size == 0) {
			return false;
		}
		if (header_size != sizeof(header)) {
			aux_message(0, "invalid video file frame header size: %zu\n", header_size);
			return false;
		}
		if (header.magic != c_file_frame_magic
		    || (header.version != c_file_frame_legacy_version
		        && header.version != c_file_frame_metadata_version
		        && header.version != c_file_frame_version)) {
			aux_message(
			        0, "invalid video file frame header: magic=0x%08x version=%u\n", header.magic,
			        header.version);
			return false;
		}
		frame->metadata.frame_id = header.frame_id;
		if (header.version == c_file_frame_metadata_version || header.version == c_file_frame_version) {
			uint32_t metadata[3] = {};
			auto metadata_count = header.version == c_file_frame_version ? 3u : 2u;
			auto metadata_size = m_file.read(metadata, sizeof(uint32_t) * metadata_count, false);
			if (metadata_size != sizeof(uint32_t) * metadata_count) {
				aux_message(0, "invalid video file frame metadata size: %zu\n", metadata_size);
				return false;
			}
			frame->metadata.control_id = metadata[0];
			frame->metadata.input_serial = metadata[1];
			frame->metadata.flags = metadata[2];
		}

		frame->bitstream.resize(header.frame_size);
		if (header.frame_size == 0) {
			return true;
		}
		auto read_size = m_file.read(frame->bitstream.data(), header.frame_size, false);
		if (read_size != header.frame_size) {
			aux_message(
			        0, "invalid video file frame payload: frame=%u read=%zu/%u\n", header.frame_id,
			        read_size, header.frame_size);
			frame->bitstream.clear();
			return false;
		}
		return true;
	}

private:
	File m_file;
};
}  // namespace spu::libspu::video
