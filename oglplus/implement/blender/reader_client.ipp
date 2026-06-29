// #include <config/basic.hpp>

namespace spu::oglplus::imports {

inline bool BlendFileReaderClient::expect(
        BlendFileReader &reader, const char *expected, const std::size_t size, const char *error_message)
{
	char buffer[16] = {'\0'};
	assert(sizeof(buffer) > size);
	reader.read(buffer, size, error_message);
	if (std::strncmp(buffer, expected, size) != 0) {
		std::string msg("Expected '");
		msg.append(expected);
		msg.append("' instead of '");
		msg.append(buffer);
		msg.append("' in input");
		reader.error(msg);
	}
	return true;
}

inline char BlendFileReaderClient::expect_one_of(
        BlendFileReader &reader, const char *options, const std::size_t size, const char *error_message)
{
	std::size_t i;
	char c = read_char(reader, error_message);

	for (i = 0; i != size; ++i) {
		if (c == options[i]) {
			return c;
		}
	}
	std::string msg("Expected one of {");
	for (i = 0; i != size; ++i) {
		if (i != 0u) {
			msg.append(", ", 2);
		}
		const char tmp[3] = {'\'', options[i], '\''};
		msg.append(tmp, 3);
	}
	msg.append("} in input");
	reader.error(msg);
	return '\0';
}

}  // namespace spu::oglplus::imports
