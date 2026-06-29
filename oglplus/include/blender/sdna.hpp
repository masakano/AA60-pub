//
//$<<Header>>$
//

#pragma once

#include <map>
#include <memory>
#include <vector>

namespace spu::oglplus::imports {

class BlendFileSDNA : public BlendFileReaderClient, public BlendFileUtils {
private:
	std::vector<std::string> m_names;
	std::vector<std::string> m_type_names;
	std::vector<uint16_t> m_type_sizes;
	std::vector<uint32_t> m_type_structs;

	std::size_t invalid_type_index() const { return m_type_sizes.size(); }

	bool type_is_struct(uint32_t type) const { return m_type_structs[type] != invalid_struct_index(); }

	struct PstrLess {
		using ptr = const std::string *;
		bool operator()(ptr a, ptr b) const
		{
			assert(a && b);
			return *a < *b;
		}
	};

	struct FlatStructInfo {
		std::vector<std::string> field_names;
		std::vector<uint16_t> field_structs;
		std::vector<uint16_t> field_indices;
		std::vector<uint32_t> field_offsets;

		std::map<const std::string *, std::size_t, PstrLess> field_map;

		FlatStructInfo(std::size_t field_count)
		        : field_names(field_count), field_structs(field_count), field_indices(field_count),
		          field_offsets(field_count)
		{
		}

		FlatStructInfo(const FlatStructInfo &) = delete;

		std::size_t field_count() const
		{
			assert(field_names.size() == field_structs.size());
			assert(field_names.size() == field_indices.size());
			assert(field_names.size() == field_offsets.size());
			return field_names.size();
		}
	};

	struct StructInfo {
		uint16_t type_index;

		std::vector<uint16_t> field_type_indices;
		std::vector<uint16_t> field_name_indices;
		std::vector<uint16_t> field_elem_counts;
		std::vector<bool> field_ptr_flags;
		std::vector<bool> field_ptr2_flags;
		std::vector<bool> field_array_flags;
		std::shared_ptr<FlatStructInfo> flat_fields;

		std::size_t field_count() const
		{
			assert(field_type_indices.size() == field_name_indices.size());
			assert(field_type_indices.size() == field_elem_counts.size());
			assert(field_type_indices.size() == field_ptr_flags.size());
			assert(field_type_indices.size() == field_ptr2_flags.size());
			assert(field_type_indices.size() == field_array_flags.size());
			return field_type_indices.size();
		}
	};

	static bool field_is_ptr(const std::string &def)
	{
		if (def.empty()) {
			return false;
		}
		return def[0] == '*';
	}

	static bool field_is_ptr_to_ptr(const std::string &def)
	{
		if (def.size() < 2) {
			return false;
		}
		return (def[0] == '*') && (def[1] == '*');
	}

	static bool field_is_fn_ptr(const std::string &def)
	{
		if (def.empty()) {
			return false;
		}
		return def.back() == ')';
	}

	static bool field_is_array(const std::string &def)
	{
		if (def.empty()) {
			return false;
		}
		return def.back() == ']';
	}

	static std::size_t field_elem_count(const std::string &def);

	std::vector<StructInfo> m_structs;

	std::size_t invalid_struct_index() const { return m_structs.size(); }

	std::map<const std::string *, std::size_t, PstrLess> m_type_map;

	static bool is_field_name_char(char c)
	{
		return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c == '_');
	}

	static std::string field_name_from_def(std::string result);

	static std::string elem_field_suffix(uint64_t i);

	std::size_t struct_flat_field_count(uint32_t struct_index);

	const std::shared_ptr<FlatStructInfo> &struct_flatten_fields(std::size_t struct_index);

	static std::size_t &type_id_seq()
	{
		static std::size_t seq = 0;
		return seq;
	}

	template<typename T> static std::size_t type_id(T *)
	{
		static std::size_t tid = type_id_seq()++;
		return tid;
	}

	std::vector<std::size_t> m_type_id_to_type_index;

	template<typename T> void init_type_id_index(const std::string &type_name)
	{
		std::size_t tid = type_id(static_cast<T *>(nullptr));
		if (m_type_id_to_type_index.size() <= tid) {
			m_type_id_to_type_index.resize(tid + 4, invalid_type_index());
		}

		auto pos = m_type_map.find(&type_name);
		if (pos == m_type_map.end()) {
			m_type_id_to_type_index[tid] = invalid_type_index();
		}
		else {
			m_type_id_to_type_index[tid] = pos->second;
		}
	}

	template<typename T> std::size_t find_type_index() const
	{
		std::size_t tid = type_id(static_cast<T *>(nullptr));
		if (m_type_id_to_type_index.size() > tid) {
			return m_type_id_to_type_index[tid];
		}
		{
			return invalid_type_index();
		}
	}

	template<typename T> bool is_known_type() const { return find_type_index<T>() != invalid_type_index(); }

	template<typename T> bool type_matches(std::size_t type_index) const
	{
		return find_type_index<T>() == type_index;
	}

	friend class BlendFile;
	friend class BlendFileType;
	friend class BlendFileStruct;
	friend class BlendFileStructRange;
	friend class BlendFileStructField;
	friend class BlendFileStructFieldRange;
	friend class BlendFileFlattenedStruct;
	friend class BlendFileFlattenedStructField;
	friend class BlendFileFlattenedStructFieldRange;
	friend class BlendFileBlockData;
	friend class BlendFileFlatStructBlockData;

public:
	BlendFileSDNA(BlendFileReader &bfr, const BlendFileInfo &bfi);
	BlendFileSDNA(const BlendFileSDNA &) = delete;
};

}  // namespace spu::oglplus::imports
#include <blender/sdna.ipp>
