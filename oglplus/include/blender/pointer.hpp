//
//$<<Header>>$
//

#pragma once

#include <blender/range.hpp>

namespace spu::oglplus::imports {

class BlendFilePointerBase {
public:
	using ValueType = uint64_t;

	operator bool() const { return m_value != 0; }

	bool operator!() const { return m_value == 0; }

	friend bool operator==(const BlendFilePointerBase &a, const BlendFilePointerBase &b)
	{
		return a.m_value == b.m_value;
	}

	friend bool operator!=(const BlendFilePointerBase &a, const BlendFilePointerBase &b)
	{
		return a.m_value != b.m_value;
	}

	ValueType value() const { return m_value; }

protected:
	BlendFilePointerBase() = default;

	BlendFilePointerBase(ValueType value, std::size_t type_index) : m_value(value), m_type_index(type_index)
	{
	}

private:
	friend class BlendFile;
	uint64_t m_value{0};
	std::size_t m_type_index{0};
};

template<uint32_t Level> class BlendFilePointerTpl : public BlendFilePointerBase {
protected:
	friend class BlendFile;
	friend class BlendFileBlock;
	friend class BlendFileBlockData;
	friend class BlendFileFlatStructBlockData;

	BlendFilePointerTpl() : BlendFilePointerBase() {}

	BlendFilePointerTpl(BlendFilePointerBase::ValueType value, std::size_t type_index)
	        : BlendFilePointerBase(value, type_index)
	{
	}
};

using BlendFilePointer = BlendFilePointerTpl<1>;
using BlendFilePointerToPointer = BlendFilePointerTpl<2>;

}  // namespace spu::oglplus::imports
