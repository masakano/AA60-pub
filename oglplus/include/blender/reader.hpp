//
//$<<Header>>$
//

#pragma once

#include <cassert>
#include <istream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace spu::oglplus::imports {

class BlendFileReader : public BlendFileUtils {
	// private:
public:
	File &m_input;

	bool eof();
	void error(const std::string &message);
	char read_char(const char *error_message);
	void raw_read(char *buffer, std::size_t size, const char *error_message);
	void read(char *buffer, std::size_t max, const char *error_message);
	void read_until(std::string &str, char delimiter, const char *error_message);
	void skip(std::size_t size, const char *error_message);
	void align(std::size_t size, const char *error_message);

	friend class BlendFileReaderClient;

public:
	BlendFileReader(File &input) : BlendFileUtils(4), m_input(input) {}
};

}  // namespace spu::oglplus::imports
#include <blender/reader.ipp>
