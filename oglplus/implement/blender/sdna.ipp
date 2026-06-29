// #include <config/basic.hpp>

namespace spu::oglplus::imports {

inline std::size_t BlendFileSDNA::field_elem_count(const std::string &def)
{
	std::size_t n = 1;
	auto p = def.find('[');
	auto e = std::string::npos;
	while (p != e) {
		std::size_t k = 0;
		do {
			++p;
			if (p == e) {
				throw std::runtime_error("Invalid field name in blend file");
			}
			char c = def[p];
			if (c == ']') {
				break;
			}
			if ((c < '0') || (c > '9')) {
				throw std::runtime_error("Invalid field name in blend file");
			}
			k *= 10;
			k += c - '0';
		} while (true);

		if (k == 0) {
			throw std::runtime_error("Invalid array size in blend file");
		}
		n *= k;
		p = def.find('[', p);
	}
	return n;
}

inline std::string BlendFileSDNA::field_name_from_def(std::string result)
{
	std::size_t i = 0;
	std::size_t n = result.size();
	while (i != n) {
		char c = result[i];
		if (!is_field_name_char(c)) {
			int32_t k = 1;
			if (c == '[') {
				while ((i + k != n) && (result[i + k] != ']')) {
					++k;
				}
			}
			else if (c == '(') {
				if (result[i + 1] == '*') {
					++k;
				}
				else {
					while ((i + k != n) && (result[i + k] != ')')) {
						++k;
					}
				}
			}
			for (std::size_t j = i + k; j != n; ++j) {
				result[j - k] = result[j];
			}
			n -= k;
		}
		else {
			++i;
		}
	}
	result.resize(n);
	return result;
}

inline std::string BlendFileSDNA::elem_field_suffix(uint64_t i)
{
	std::string result;
	if (i == 0) {
		result.append("0", 1);
	}
	else {
		std::size_t l = 1;
		uint64_t d = 1;
		while (l < 24) {
			if (i / d == 0) {
				break;
			}
			l += 1;
			d *= 10;
		}
		--l;
		d /= 10;
		result.resize(l);
		for (auto p = 0u; p != l; ++p) {
			result[p] = '0' + i / d;
			i %= d;
			d /= 10;
		}
	}
	return result;
}

inline std::size_t BlendFileSDNA::struct_flat_field_count(uint32_t struct_index)
{
	if (struct_index == invalid_struct_index()) {
		return 1;
	}

	const StructInfo &si = m_structs[struct_index];

	if (si.flat_fields) {
		return si.flat_fields->field_count();
	}

	std::size_t result = 0;
	std::size_t f = 0;
	std::size_t fn = si.field_count();
	while (f != fn) {
		std::size_t elem_count = si.field_elem_counts[f];
		uint32_t fsi = m_type_structs[si.field_type_indices[f]];
		if (si.field_ptr2_flags[f]) {
			++result;
		}
		else if (si.field_ptr_flags[f]) {
			++result;
		}
		else if (fsi == invalid_struct_index()) {
			++result;
		}
		else {
			result += struct_flat_field_count(fsi) * elem_count;
		}
		++f;
	}
	return result;
}

inline const std::shared_ptr<BlendFileSDNA::FlatStructInfo> &BlendFileSDNA::struct_flatten_fields(
        std::size_t struct_index)
{
	std::size_t field_index = 0;
	std::size_t offset = 0;

	const StructInfo &si = m_structs[struct_index];
	std::shared_ptr<FlatStructInfo> &result = m_structs[struct_index].flat_fields;

	if (result) {
		return result;
	}

	const std::size_t fc = struct_flat_field_count(struct_index);
	result = std::make_shared<FlatStructInfo>(fc);

	std::size_t f = 0;
	std::size_t fn = si.field_count();
	while (f != fn) {
		uint16_t fti = si.field_type_indices[f];
		uint32_t fsi = m_type_structs[fti];

		const std::string &fd = m_names[si.field_name_indices[f]];
		std::string fldn(field_name_from_def(fd));

		bool is_ptr = si.field_ptr2_flags[f] || si.field_ptr_flags[f];
		bool is_plain = fsi == invalid_struct_index();

		std::size_t elem_count = si.field_elem_counts[f];

		if (is_ptr || is_plain) {
			std::size_t size = is_ptr ? m_ptr_size : std::size_t(m_type_sizes[fti]);

			result->field_names[field_index] = fldn;
			result->field_map[&result->field_names[field_index]] = field_index;
			result->field_structs[field_index] = struct_index;
			result->field_indices[field_index] = f;

			std::size_t align_diff = this->align_diff(offset, size);
			if (align_diff != 0u) {
				offset += size - align_diff;
			}
			result->field_offsets[field_index] = offset;
			offset += size * elem_count;

			++field_index;
		}
		else {
			const std::shared_ptr<FlatStructInfo> &nffi = struct_flatten_fields(fsi);

			assert(nffi);

			std::size_t gn = nffi->field_count();
			for (auto e = 0u; e != elem_count; ++e) {
				std::string efn = fldn;
				if (e != 0u) {
					efn.append(elem_field_suffix(e));
				}
				for (auto g = 0u; g != gn; ++g) {
					const std::string &ffn = nffi->field_names[g];
					std::string nfn = efn;
					nfn.append(".", 1);
					nfn.append(ffn);

					std::size_t nfs = nffi->field_structs[g];
					std::size_t nfi = nffi->field_indices[g];

					result->field_names[field_index] = nfn;
					result->field_map[&result->field_names[field_index]] = field_index;
					result->field_structs[field_index] = nfs;
					result->field_indices[field_index] = nfi;

					uint16_t nfti = m_structs[nfs].field_type_indices[nfi];
					uint16_t nfec = m_structs[nfs].field_elem_counts[nfi];
					bool is_ptr2 = m_structs[nfs].field_ptr_flags[nfi];
					bool is_ptr3 = m_structs[nfs].field_ptr2_flags[nfi];

					std::size_t size;
					if (is_ptr2 || is_ptr3) {
						size = m_ptr_size;
					}
					else {
						size = m_type_sizes[nfti];
					}

					std::size_t align_diff = this->align_diff(offset, size);
					if (align_diff != 0u) {
						offset += size - align_diff;
					}
					result->field_offsets[field_index] = offset;
					offset += size * nfec;
					++field_index;
				}
			}
		}
		++f;
	}

	assert(result->field_map.size() == fc);
	assert(offset == m_type_sizes[si.type_index]);

	return result;
}

inline BlendFileSDNA::BlendFileSDNA(BlendFileReader &bfr, const BlendFileInfo &bfi)
        : BlendFileUtils(bfi.pointerSize())
{
	align(bfr, 4, "Failed to skip DNA block padding");
	expect(bfr, "SDNA", 4, "Failed to read code in DNA block");
	expect(bfr, "NAME", 4, "Failed to read code in DNA block");

	uint32_t i;
	uint32_t j;
	uint32_t n;
	uint32_t k;
	n = read_int<uint32_t>(bfr, /*bfi.byteOrder(),*/ "Failed to read name count from DNA block");
	m_names.reserve(n);
	for (i = 0; i != n; ++i) {
		m_names.push_back(read_str(bfr, "Failed to read name from DNA block"));
	}

	align(bfr, 4, "Failed to skip DNA block padding");
	expect(bfr, "TYPE", 4, "Failed to read code in DNA block");

	n = read_int<uint32_t>(bfr, /*bfi.byteOrder(),*/ "Failed to read type name count from DNA block");
	m_type_names.reserve(n);

	for (i = 0; i != n; ++i) {
		m_type_names.push_back(read_str(bfr, "Failed to read type name from DNA block"));
		if (m_type_map.find(&m_type_names.back()) != m_type_map.end()) {
			std::string what("Multiple definitions of type '");
			what.append(m_type_names.back());
			what.append("' in DNA block");
			throw std::runtime_error(what);
		}
		m_type_map[&m_type_names.back()] = i;
	}
	assert(m_type_names.size() == m_type_map.size());

	align(bfr, 4, "Failed to skip DNA block padding");
	expect(bfr, "TLEN", 4, "Failed to read code in DNA block");

	m_type_sizes.reserve(n);
	for (i = 0; i != n; ++i) {
		m_type_sizes.push_back(read_int<uint16_t>(
		        bfr, /*bfi.byteOrder(),*/ "Failed to read type size from DNA block"));
	}

	align(bfr, 4, "Failed to skip DNA block padding");
	expect(bfr, "STRC", 4, "Failed to read code in DNA block");

	k = read_int<uint32_t>(bfr, /*bfi.byteOrder(),*/ "Failed to read number of structures from DNA block");
	m_type_structs.resize(n, k);
	n = k;

	m_structs.resize(n);
	uint16_t ti;
	for (i = 0; i != n; ++i) {
		StructInfo &si = m_structs[i];
		ti = read_int<uint16_t>(
		        bfr, /*bfi.byteOrder(),*/
		        "Failed read structure type index from DNA block");
		si.type_index = ti;
		k = read_int<uint16_t>(
		        bfr, /*bfi.byteOrder(),*/ "Failed to read number of fields from DNA block");

		if (m_type_structs[ti] != n) {
			std::string what("Duplicate definitions of structure '");
			what.append(m_type_names[ti]);
			what.append("' in DNA block");
			throw std::runtime_error(what);
		}
		m_type_structs[ti] = i;

		si.field_type_indices.resize(k);
		si.field_name_indices.resize(k);
		si.field_elem_counts.resize(k);
		si.field_ptr_flags.resize(k);
		si.field_ptr2_flags.resize(k);
		si.field_array_flags.resize(k);

		for (j = 0; j != k; ++j) {
			si.field_type_indices[j] = read_int<uint16_t>(
			        bfr, /*bfi.byteOrder(),*/
			        "Failed read field type index from DNA block");

			si.field_name_indices[j] = read_int<uint16_t>(
			        bfr, /*bfi.byteOrder(),*/
			        "Failed read field name index from DNA block");

			const std::string &def = m_names[si.field_name_indices[j]];

			bool is_ptr = field_is_ptr(def);
			bool is_ptr_to_ptr = field_is_ptr_to_ptr(def);
			bool is_fn_ptr = field_is_fn_ptr(def);
			bool is_array = field_is_array(def);

			si.field_elem_counts[j] = is_array ? field_elem_count(def) : 1;
			si.field_ptr_flags[j] = is_ptr;
			si.field_ptr2_flags[j] = is_ptr_to_ptr || is_fn_ptr;
			si.field_array_flags[j] = is_array;
		}
	}
	init_type_id_index<void>("void");
	init_type_id_index<char>("char");
	init_type_id_index<uint8_t>("uchar");
	init_type_id_index<int16_t>("short");
	init_type_id_index<uint16_t>("ushort");
	init_type_id_index<int>("int");
	init_type_id_index<uint32_t>("uint");
	init_type_id_index<long>("long");
	init_type_id_index<u_long>("ulong");
	init_type_id_index<long>("long");
	init_type_id_index<u_long>("ulong");
	init_type_id_index<int64_t>("int64_t");
	init_type_id_index<uint64_t>("uint64_t");
	init_type_id_index<float>("float");
	init_type_id_index<double>("double");
}

}  // namespace spu::oglplus::imports
