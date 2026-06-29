

namespace spu::oglplus::imports {

inline bool BlendFileReader::eof() { return m_input.tell() >= m_input.size(); }

inline void BlendFileReader::error(const std::string &message)
{
	std::stringstream ss;
	ss << "Blend file read error at byte ";
	ss << m_input.tell();
	ss << ": ";
	ss << message;
	throw std::runtime_error(ss.str());
}

inline char BlendFileReader::read_char(const char *error_message)
{
	uint8_t c = '\0';
	if (m_input.getchar(c) == false) {
		error(error_message);
	}
	return c;
}

inline void BlendFileReader::raw_read(char *buffer, std::size_t size, const char *error_message)
{
	assert(size != 0);
	if (m_input.read(buffer, size, false) != size) {
		error(error_message);
	}
}

inline void BlendFileReader::read(char *buffer, std::size_t max, const char *error_message)
{
	assert(max != 0);
	size_t len = 0;
	if ((len = m_input.read(buffer, max)) <= 0) {
		error(error_message);
	}
	else {
		buffer[len] = '\0';
	}
}

inline void BlendFileReader::read_until(std::string &str, char delimiter, const char *error_message)
{
	uint8_t c;
	while (m_input.getchar(c)) {
		if (c == delimiter) {
			return;
		}
		str += c;
	}
	error(error_message);
}

inline void BlendFileReader::skip(std::size_t size, const char *error_message)
{
	if (m_input.read(nullptr, size, false) != size) {
		error(error_message);
	}
}

inline void BlendFileReader::align(const std::size_t size, const char *error_message)
{
	std::streampos input_pos = m_input.tell();
	const std::size_t mod = align_diff(input_pos, size);
	if (mod != 0) {
		skip(size - mod, error_message);
		assert(m_input.tell() % size == 0);
	}
}

}  // namespace spu::oglplus::imports
