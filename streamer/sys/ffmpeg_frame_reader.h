//
// FFMpegFrameReader :
//
#pragma once
#include "common.h"

namespace spu {

class FFMpegFrameReader {
public:
	~FFMpegFrameReader() { close(); }

	void init(const std::vector<std::filesystem::path> &paths, int32_t width, int32_t height)
	{
		m_width = width;
		m_height = height;
		for (auto &path: paths) {
			auto full_path = File::searchPath(path);
			m_paths.push_back(full_path);
		}
	}

	uint32_t readFrame(RGBA8 *pixels)
	{
		if (m_paths.empty()) {
			return c_invalid_serial;
		}

		auto frame_size = size_t(m_width) * m_height * sizeof(RGBA8);
		while (true) {
			if (m_pipe == nullptr && !openNext()) {
				return c_invalid_serial;
			}

			auto *dst = reinterpret_cast<uint8_t *>(pixels);
			auto offset = size_t(0);
			while (offset < frame_size) {
				auto read_size = fread(dst + offset, 1, frame_size - offset, m_pipe);
				if (read_size == 0) {
					break;
				}
				offset += read_size;
			}
			if (offset == frame_size) {
				auto serial = ((m_fileIndex & 0xffffu) << 16) | (m_frameIndex & 0xffffu);
				m_frameIndex++;
				return serial;
			}
			close();
		}
	}

	inline static constexpr uint32_t c_invalid_serial = ~0u;

private:
	std::vector<std::filesystem::path> m_paths;
	FILE *m_pipe = nullptr;
	int32_t m_width = 0;
	int32_t m_height = 0;
	uint32_t m_nextFileIndex = 0;
	uint32_t m_fileIndex = 0;
	uint32_t m_frameIndex = 0;

	static std::string shellQuote(const std::string &text)
	{
		std::string out = "'";
		for (auto c: text) {
			if (c == '\'') {
				out += "'\\''";
			}
			else {
				out += c;
			}
		}
		out += "'";
		return out;
	}

	bool openNext()
	{
		close();
		if (m_paths.empty()) {
			return false;
		}

		for (auto attempt = 0u; attempt < m_paths.size(); ++attempt) {
			m_fileIndex = m_nextFileIndex;
			m_frameIndex = 0;
			auto path = m_paths[m_fileIndex];
			m_nextFileIndex = (m_nextFileIndex + 1) % m_paths.size();
			auto filter = string_printf(
			        "scale=%d:%d:force_original_aspect_ratio=decrease,pad=%d:%d:-1:-1:color=black",
			        m_width, m_height, m_width, m_height);

			auto command = string_printf(
			        "ffmpeg -hide_banner -loglevel error -i %s -vf %s -an -f rawvideo -pix_fmt rgba -",
			        shellQuote(path.string()).c_str(), shellQuote(filter).c_str());

			printf("command=[%s]\n", command.c_str());
			m_pipe = popen(command.c_str(), "r");
			if (m_pipe) {
				aux_message(0, "video source: %s\n", path.string().c_str());
				return true;
			}
		}
		return false;
	}

	void close()
	{
		if (m_pipe) {
			pclose(m_pipe);
			m_pipe = nullptr;
		}
	}
};
}  // namespace spu
