//
//$<<Header>>$
//

#pragma once

#include <array>
#include <cstring>
#include <blender/reader.hpp>

namespace spu::oglplus::imports {

class BlendFileReaderClient {
protected:
	static void adjust_ptr_size(BlendFileReader &reader, std::size_t ptr_size)
	{
		reader.m_ptr_size = ptr_size;
	}

	static bool eof(BlendFileReader &reader) { return reader.eof(); }

	static size_t position(const BlendFileReader &reader) { return reader.m_input.tell(); }

	static void go_to(const BlendFileReader &reader, size_t pos)
	{
		reader.m_input.rewind();
		reader.m_input.read(nullptr, pos);
	}

	static void read(BlendFileReader &reader, char *buffer, std::size_t max, const char *error_message)
	{
		reader.read(buffer, max, error_message);
	}

	static void raw_read(BlendFileReader &reader, char *buffer, std::size_t size, const char *error_message)
	{
		reader.raw_read(buffer, size, error_message);
	}

	template<std::size_t size>
	static std::array<char, size> read_array(BlendFileReader &reader, const char *error_message)
	{
		std::array<char, size> result;
		reader.raw_read(result.data(), size, error_message);
		return result;
	}

	static std::string read_str(BlendFileReader &reader, const char *error_message)
	{
		std::string buffer;
		reader.read_until(buffer, '\0', error_message);
		return buffer;
	}

	static char read_char(BlendFileReader &reader, const char *error_message)
	{
		return reader.read_char(error_message);
	}

	template<typename Int>
	static Int read_int(BlendFileReader &reader, /*Endian file_byte_order,*/ const char *error_message)
	{
		Int values;
		char *buffer = reinterpret_cast<char *>(&values);
		reader.raw_read(buffer, sizeof(Int), error_message);
		// return aux::reorderToNative(file_byte_order, values);
		return values;
	}

	static void skip(BlendFileReader &reader, const std::size_t size, const char *error_message)
	{
		reader.skip(size, error_message);
	}

	static void align(BlendFileReader &reader, const std::size_t size, const char *error_message)
	{
		reader.align(size, error_message);
	}

	bool expect(BlendFileReader &reader, const char *expected, std::size_t size, const char *error_message);

	char expect_one_of(
	        BlendFileReader &reader, const char *options, std::size_t size, const char *error_message);
};

}  // namespace spu::oglplus::imports
#include <blender/reader_client.ipp>
