//
//$<<Header>>$
//

#pragma once

#include <type_traits>

namespace spu::oglplus::imports {

template<typename T> class BlendFileFlatStructTypedFieldData;

template<typename T> class BlendFileFlatStructTypedFieldDataImpl {
private:
	BlendFileFlattenedStructField m_flat_field;
	const BlendFileBlockData *m_block_data_ref;
	std::size_t m_offset;

	friend class BlendFileFlatStructTypedFieldData<T>;

protected:
	BlendFileFlatStructTypedFieldDataImpl(
	        const BlendFileFlattenedStructField &flat_field, const BlendFileBlockData &block_data_ref,
	        std::size_t offset)
	        : m_flat_field(flat_field), m_block_data_ref(&block_data_ref), m_offset(offset)
	{
	}

	BlendFileFlatStructTypedFieldDataImpl(const BlendFileFlatStructTypedFieldDataImpl &tmp)
	        : m_flat_field(tmp.m_flat_field), m_block_data_ref(tmp.m_block_data_ref), m_offset(tmp.m_offset)
	{
	}

	template<uint32_t Level>
	BlendFilePointerTpl<Level> do_get(
	        BlendFilePointerTpl<Level> * /*selector*/, std::size_t block_element,
	        std::size_t field_element) const
	{
		return m_block_data_ref->template get_pointer<Level>(
		        m_flat_field, block_element, field_element, m_offset);
	}

	BlendFilePointer do_get(
	        void ** /*selector*/, std::size_t block_element, std::size_t field_element) const
	{
		return m_block_data_ref->getPointer(m_flat_field, block_element, field_element, m_offset);
	}

	BlendFilePointerToPointer do_get(
	        void *** /*selector*/, std::size_t block_element, std::size_t field_element) const
	{
		return m_block_data_ref->getPointerToPointer(
		        m_flat_field, block_element, field_element, m_offset);
	}

	std::string do_get(
	        std::string * /*selector*/, std::size_t block_element, std::size_t field_element) const
	{
		return m_block_data_ref->getString(m_flat_field, block_element, field_element, m_offset);
	}

	char do_get(char * /*selector*/, std::size_t block_element, std::size_t field_element) const
	{
		return m_block_data_ref->getInt<char>(m_flat_field, block_element, field_element, m_offset);
	}

	float do_get(float * /*selector*/, std::size_t block_element, std::size_t field_element) const
	{
		return m_block_data_ref->getFloat<float>(m_flat_field, block_element, field_element, m_offset);
	}

	double do_get(double * /*selector*/, std::size_t block_element, std::size_t field_element) const
	{
		return m_block_data_ref->getFloat<double>(m_flat_field, block_element, field_element, m_offset);
	}

	template<typename Int>
	typename std::enable_if<std::is_integral<Int>::value, Int>::type do_get(
	        Int * /*selector*/, std::size_t block_element, std::size_t field_element) const
	{
		return m_block_data_ref->getInt<Int>(m_flat_field, block_element, field_element, m_offset);
	}
};

template<typename T> class BlendFileFlatStructTypedFieldData : public BlendFileFlatStructTypedFieldDataImpl<T> {
private:
	using base_t = BlendFileFlatStructTypedFieldDataImpl<T>;
	static const base_t &that();

	friend class BlendFileFlatStructBlockData;

	BlendFileFlatStructTypedFieldData(
	        const BlendFileFlattenedStructField &flat_field, const BlendFileBlockData &block_data_ref,
	        std::size_t offset)
	        : base_t(flat_field, block_data_ref, offset)
	{
	}

public:
	using value_type_t = decltype(that().do_get(static_cast<T *>(nullptr), 0, 0));
	using ValueType = typename BlendFileFlatStructTypedFieldData<T>::value_type_t;

	BlendFileFlatStructTypedFieldData(const BlendFileFlatStructTypedFieldData &tmp)
	        : base_t(static_cast<base_t>(tmp))
	{
	}

	ValueType get(std::size_t block_element, std::size_t field_element) const
	{
		return this->do_get(static_cast<T *>(nullptr), block_element, field_element);
	}

	ValueType get() const { return get(0, 0); }

	operator ValueType() const { return get(0, 0); }
};

class BlendFileFlatStructBlockData {
private:
	BlendFileFlattenedStruct m_flat_struct;
	BlendFileBlock m_block;
	BlendFileBlockData m_data;
	std::size_t m_offset;

	friend class BlendFile;

	BlendFileFlatStructBlockData(
	        const BlendFileFlattenedStruct &flat_struct, const BlendFileBlock &block,
	        const BlendFileBlockData &data, std::size_t offset)
	        : m_flat_struct(flat_struct), m_block(block), m_data(data), m_offset(offset)
	{
	}

	template<typename T> static T adjust_value(T *ptr)
	{
		assert(ptr);
		return *ptr;
	}

	static BlendFilePointer adjust_value(void **) { return BlendFilePointer(); }

	template<typename T> struct AdjustType {
		using type = decltype(adjust_value(static_cast<T *>(nullptr)));
	};

public:
	bool isStructure() const { return m_flat_struct.isStructure(); }

	const BlendFileFlattenedStruct &structure() const { return m_flat_struct; }

	const std::string &structureName() const { return structure().name(); }

	std::size_t structureSize() const { return structure().size(); }

	BlendFileFlattenedStructFieldRange structureFields() const { return structure().fields(); }

	BlendFileFlattenedStructField StructureFieldByName(const std::string &field_name) const
	{
		return structure().fieldByName(field_name);
	}

	BlendFileFlattenedStructField operator/(const std::string &field_name) const
	{
		return structure().fieldByName(field_name);
	}

	const BlendFileBlock &block() const { return m_block; }

	std::string blockCode() const { return m_block.code(); }

	uint32_t blockSize() const { return m_block.size(); }

	uint32_t blockElementCount() const { return m_block.elementCount(); }

	BlendFilePointer blockPointer() const { return m_block.pointer(); }

	const BlendFileBlockData &blockData() const { return m_data; }

	template<class Visitor>
	void blockDataValueVisit(
	        Visitor visitor, const BlendFileFlattenedStructField &flat_field, std::size_t block_element = 0,
	        std::size_t field_element = 0) const
	{
		blockData().valueVisit(visitor, flat_field, block_element, field_element, m_offset);
	}

	template<typename T> BlendFileFlatStructTypedFieldData<T> Field(const std::string &field_name) const
	{
		return BlendFileFlatStructTypedFieldData<T>(
		        structure().fieldByName(field_name), blockData(), m_offset);
	}

	template<typename T>
	typename AdjustType<T>::type tryGet(
	        const std::string &field_name, T default_value, std::size_t block_element,
	        std::size_t field_element) const
	{
		try {
			return Field<T>(field_name).get(block_element, field_element);
		}
		catch (...) {
		}
		return adjust_value(&default_value);
	}

	template<typename T> typename AdjustType<T>::type tryGet(const std::string &field_name, T default_value)
	{
		return tryGet<T>(field_name, default_value, 0, 0);
	}
};

}  // namespace spu::oglplus::imports
