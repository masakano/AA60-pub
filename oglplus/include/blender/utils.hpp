//
//$<<Header>>$
//

#pragma once

namespace spu::oglplus::imports {

class BlendFileUtils {
protected:
	std::size_t m_ptr_size;

	BlendFileUtils(std::size_t ptr_size) : m_ptr_size(ptr_size) {}

	template<typename Int> Int align_diff(Int offset, std::size_t size)
	{
		if (size > m_ptr_size) {
			size = m_ptr_size;
		}
		return offset % size;
	}

	template<typename Int> Int do_align_offset(Int offset, std::size_t size)
	{
		Int diff = align_diff(offset, size);
		if (diff) {
			offset += size - diff;
		}
		return offset;
	}
};

}  // namespace spu::oglplus::imports
