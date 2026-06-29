//
// Gathered uniform source preprocessing :
//
#include "spu_shader_gathered_uniform_source.h"
#include <spu/GL/gl.h>

namespace {
struct SourceLoc {
	int32_t index = 0;
	int32_t lineno = 1;
};

struct UniformDecl {
	std::string type;
	std::string name;
	std::string member;
	std::string array;
	SourceLoc loc;
	uint32_t gl_type = 0;
	uint32_t nelem = 1;
	uint32_t offset = 0;
	uint32_t size = 0;
	uint32_t stride = 0;
	uint32_t elem_size = 0;
	uint32_t block_size = 0;
	uint32_t align = 0;
};

struct UniformTypeInfo {
	const char *name;
	uint32_t gl_type;
	uint32_t elem_size;
	uint32_t block_size;
	uint32_t align;
	uint32_t array_align;
};

bool is_identifier_char(char c) { return std::isalnum(uint8_t(c)) != 0 || c == '_'; }

uint32_t align_up(uint32_t value, uint32_t align) { return (value + align - 1) / align * align; }

const UniformTypeInfo *find_uniform_type_info(const std::string &type)
{
	static const UniformTypeInfo infos[] = {
	        {"float", GL_FLOAT,             4,   4,   4,  16},
	        {"vec2",  GL_FLOAT_VEC2,        8,   8,   8,  16},
	        {"vec3",  GL_FLOAT_VEC3,        12,  12,  16, 16},
	        {"vec4",  GL_FLOAT_VEC4,        16,  16,  16, 16},
	        {"int",   GL_INT,               4,   4,   4,  16},
	        {"ivec2", GL_INT_VEC2,          8,   8,   8,  16},
	        {"ivec3", GL_INT_VEC3,          12,  12,  16, 16},
	        {"ivec4", GL_INT_VEC4,          16,  16,  16, 16},
	        {"uint",  GL_UNSIGNED_INT,      4,   4,   4,  16},
	        {"uvec2", GL_UNSIGNED_INT_VEC2, 8,   8,   8,  16},
	        {"bool",  GL_BOOL,              4,   4,   4,  16},
	        {"uvec3", GL_UNSIGNED_INT_VEC3, 12,  12,  16, 16},
	        {"uvec4", GL_UNSIGNED_INT_VEC4, 16,  16,  16, 16},
	        {"dvec2", GL_DOUBLE_VEC2,       16,  16,  16, 32},
	        {"dvec3", GL_DOUBLE_VEC3,       24,  24,  32, 32},
	        {"dvec4", GL_DOUBLE_VEC4,       32,  32,  32, 32},
	        {"mat4",  GL_FLOAT_MAT4,        64,  64,  16, 16},
	        {"dmat2", GL_DOUBLE_MAT2,       32,  64,  32, 32},
	        {"dmat3", GL_DOUBLE_MAT3,       72,  96,  32, 32},
	        {"dmat4", GL_DOUBLE_MAT4,       128, 128, 32, 32},
	};
	for (const auto &info: infos) {
		if (type == info.name) {
			return &info;
		}
	}
	return nullptr;
}

uint32_t parse_array_count(const std::string &array)
{
	if (array.empty()) {
		return 1;
	}
	aux_error(
	        array.size() < 3 || array.front() != '[' || array.back() != ']', "invalid uniform array '%s'\n",
	        array.c_str());
	auto count_str = array.substr(1, array.size() - 2);
	for (auto c: count_str) {
		aux_error(
		        std::isdigit(uint8_t(c)) == 0, "uniform array size must be numeric: '%s'\n",
		        array.c_str());
	}
	auto count = uint32_t(std::stoul(count_str));
	aux_error(count == 0u, "uniform array size must be positive: '%s'\n", array.c_str());
	return count;
}

std::string trim_string(const std::string &str)
{
	auto begin = str.find_first_not_of(" \t\r\n");
	if (begin == std::string::npos) {
		return "";
	}
	auto end = str.find_last_not_of(" \t\r\n");
	return str.substr(begin, end - begin + 1);
}

std::string strip_line_comment(const std::string &line)
{
	auto end = line.find("//");
	auto block = line.find("/*");
	if (end == std::string::npos || (block != std::string::npos && block < end)) {
		end = block;
	}
	return line.substr(0, end);
}

bool is_opaque_uniform_type(const std::string &type)
{
	return type.starts_with("sampler") || type.starts_with("isampler") || type.starts_with("usampler")
	    || type.starts_with("image") || type.starts_with("iimage") || type.starts_with("uimage")
	    || type == "atomic_uint";
}

bool parse_line_directive(const std::string &line, SourceLoc &loc)
{
	SourceLoc new_loc;
	if (sscanf(line.c_str(), "#line %d %d", &new_loc.lineno, &new_loc.index) == 2) {
		loc = new_loc;
		return true;
	}
	return false;
}

bool parse_initialized_uniform(const std::string &line, std::string &symbol)
{
	auto code = trim_string(strip_line_comment(line));
	if (!code.starts_with("uniform ")) {
		return false;
	}
	if (code.find('{') != std::string::npos || code.empty() || code.back() != ';') {
		return false;
	}

	auto rest = trim_string(code.substr(7));
	auto type_end = rest.find_first_of(" \t");
	if (type_end == std::string::npos) {
		return false;
	}
	auto type = rest.substr(0, type_end);
	if (is_opaque_uniform_type(type)) {
		return false;
	}

	auto name = trim_string(rest.substr(type_end + 1));
	name.pop_back();
	name = trim_string(name);
	auto initializer = name.find('=');
	if (initializer == std::string::npos || name.find(',') != std::string::npos) {
		return false;
	}

	symbol = trim_string(name.substr(0, initializer));
	auto array_begin = symbol.find('[');
	if (array_begin != std::string::npos) {
		symbol = symbol.substr(0, array_begin);
	}
	return symbol.starts_with("u_");
}

bool parse_uniform_decl(
        const std::string &line, UniformDecl &decl, std::string *skipped_symbol, std::string *skipped_reason)
{
	auto code = trim_string(strip_line_comment(line));
	if (!code.starts_with("uniform ")) {
		return false;
	}
	if (code.find('{') != std::string::npos) {
		return false;
	}
	if (code.empty() || code.back() != ';') {
		return false;
	}

	auto rest = trim_string(code.substr(7));
	auto type_end = rest.find_first_of(" \t");
	if (type_end == std::string::npos) {
		return false;
	}
	auto type = rest.substr(0, type_end);
	if (is_opaque_uniform_type(type)) {
		return false;
	}

	auto name = trim_string(rest.substr(type_end + 1));
	name.pop_back();
	name = trim_string(name);
	if (name.find('=') != std::string::npos) {
		return false;
	}
	if (name.find(',') != std::string::npos) {
		if (skipped_symbol != nullptr) {
			*skipped_symbol = name;
		}
		if (skipped_reason != nullptr) {
			*skipped_reason = "multiple declarations are not supported";
		}
		return false;
	}

	std::string array;
	auto array_begin = name.find('[');
	if (array_begin != std::string::npos) {
		array = name.substr(array_begin);
		name = name.substr(0, array_begin);
		if (array.empty() || array.back() != ']') {
			return false;
		}
	}

	if (!name.starts_with("u_")) {
		if (skipped_symbol != nullptr) {
			*skipped_symbol = name;
		}
		if (skipped_reason != nullptr) {
			*skipped_reason = "it does not have 'u_' prefix";
		}
		return false;
	}

	auto *info = find_uniform_type_info(type);
	aux_error(info == nullptr, "unsupported gathered uniform type '%s'\n", type.c_str());

	decl.type = type;
	decl.name = name;
	decl.member = name.substr(2);
	decl.array = array;
	decl.gl_type = info->gl_type;
	decl.nelem = parse_array_count(array);
	decl.elem_size = info->elem_size;
	decl.size = info->elem_size * decl.nelem;
	decl.align = decl.nelem > 1 ? info->array_align : info->align;
	decl.stride = decl.nelem > 1 ? align_up(info->block_size, info->array_align) : 0;
	decl.block_size = decl.nelem > 1 ? decl.stride * decl.nelem : info->block_size;
	return true;
}

void add_uniform_decl(std::vector<UniformDecl> &uniforms, const UniformDecl &decl)
{
	for (const auto &uniform: uniforms) {
		if (uniform.name == decl.name) {
			aux_error(
			        uniform.type != decl.type || uniform.array != decl.array,
			        "uniform '%s' declaration mismatch\n", decl.name.c_str());
			return;
		}
	}
	uniforms.push_back(decl);
}

uint32_t layout_uniforms(std::vector<UniformDecl> &uniforms)
{
	uint32_t offset = 0;
	for (auto &uniform: uniforms) {
		offset = align_up(offset, uniform.align);
		uniform.offset = offset;
		offset += uniform.block_size;
	}
	return align_up(offset, 16);
}

void set_gathered_uniform_result(
        spu::libspu::spu_shader::GatheredUniformSourceResult &result, const std::vector<UniformDecl> &uniforms,
        uint32_t block_size)
{
	result.block_symbol = "UB_GATHERED";
	result.block_size = block_size;
	result.uniforms.clear();
	result.uniforms.reserve(uniforms.size());
	for (const auto &uniform: uniforms) {
		result.uniforms.push_back({
		        uniform.name,
		        uniform.gl_type,
		        uniform.nelem,
		        uniform.offset,
		        uniform.size,
		        uniform.stride,
		});
	}
}

const char *source_name(const spu::Attrs &attrs, int32_t index)
{
	auto key = spu::string_printf("shader_source.%d.name", index);
	return attrs.get(key.c_str(), "<shader>");
}

std::string make_blank_line(const std::string &line)
{
	return !line.empty() && line.back() == '\n' ? "\n" : "";
}

std::string transform_uniform_decls(
        std::string text, std::vector<UniformDecl> &uniforms, const spu::Attrs &attrs)
{
	std::string out;
	SourceLoc loc;
	for (auto current = 0u; current < text.size();) {
		auto next = text.find('\n', current);
		if (next == std::string::npos) {
			next = text.size() - 1;
		}
		auto line = text.substr(current, next - current + 1);
		current = next + 1;

		if (parse_line_directive(line, loc)) {
			out += line;
			continue;
		}

		std::string initialized_symbol;
		if (parse_initialized_uniform(line, initialized_symbol)) {
			aux_printf(
			        "%s:%d: warning: initialized uniform '%s' is not gathered\n",
			        source_name(attrs, loc.index), loc.lineno, initialized_symbol.c_str());
		}

		UniformDecl decl;
		decl.loc = loc;
		std::string skipped_symbol;
		std::string skipped_reason;
		if (parse_uniform_decl(line, decl, &skipped_symbol, &skipped_reason)) {
			add_uniform_decl(uniforms, decl);
			out += make_blank_line(line);
		}
		else {
			if (!skipped_symbol.empty()) {
				aux_printf(
				        "%s:%d: warning: uniform '%s' is not gathered because %s\n",
				        source_name(attrs, loc.index), loc.lineno, skipped_symbol.c_str(),
				        skipped_reason.c_str());
			}
			out += line;
		}
		loc.lineno++;
	}
	return out;
}

std::map<std::string, std::string> make_uniform_replacements(const std::vector<UniformDecl> &uniforms)
{
	std::map<std::string, std::string> replacements;
	for (const auto &uniform: uniforms) {
		replacements[uniform.name] = std::string("ub_gathered.") + uniform.member;
	}
	return replacements;
}

void validate_gathered_uniform_declarations(
        const std::string &text, const std::map<std::string, std::string> &replacements,
        const spu::Attrs &attrs)
{
	SourceLoc loc;
	for (auto current = 0u; current < text.size();) {
		auto next = text.find('\n', current);
		if (next == std::string::npos) {
			next = text.size() - 1;
		}
		auto line = text.substr(current, next - current + 1);
		current = next + 1;

		if (parse_line_directive(line, loc)) {
			continue;
		}

		auto code = trim_string(strip_line_comment(line));
		if (!code.starts_with("uniform ") || code.find('{') != std::string::npos) {
			loc.lineno++;
			continue;
		}

		for (auto i = 0u; i < code.size();) {
			if (std::isalpha(uint8_t(code[i])) == 0 && code[i] != '_') {
				i++;
				continue;
			}

			auto begin = i;
			while (i < code.size() && is_identifier_char(code[i])) {
				i++;
			}
			auto name = code.substr(begin, i - begin);
			if (replacements.find(name) != replacements.end()) {
				aux_error(
				        true, "%s:%d: uniform declaration for gathered uniform '%s' remains\n",
				        source_name(attrs, loc.index), loc.lineno, name.c_str());
			}
		}
		loc.lineno++;
	}
}

std::string replace_uniform_refs(
        const std::string &text, const std::map<std::string, std::string> &replacements)
{
	std::string out;
	bool is_block_comment = false;
	for (auto i = 0u; i < text.size();) {
		if (text[i] == '#') {
			auto next = text.find('\n', i);
			if (next == std::string::npos) {
				out += text.substr(i);
				break;
			}
			out += text.substr(i, next - i + 1);
			i = next + 1;
			continue;
		}

		if (is_block_comment) {
			if (i + 1 < text.size() && text[i] == '*' && text[i + 1] == '/') {
				is_block_comment = false;
				out += text.substr(i, 2);
				i += 2;
			}
			else {
				out += text[i++];
			}
			continue;
		}

		if (i + 1 < text.size() && text[i] == '/' && text[i + 1] == '/') {
			auto next = text.find('\n', i);
			if (next == std::string::npos) {
				out += text.substr(i);
				break;
			}
			out += text.substr(i, next - i + 1);
			i = next + 1;
			continue;
		}
		if (i + 1 < text.size() && text[i] == '/' && text[i + 1] == '*') {
			is_block_comment = true;
			out += text.substr(i, 2);
			i += 2;
			continue;
		}

		if (std::isalpha(uint8_t(text[i])) != 0 || text[i] == '_') {
			auto begin = i;
			while (i < text.size() && is_identifier_char(text[i])) {
				i++;
			}
			auto name = text.substr(begin, i - begin);
			auto replacement = replacements.find(name);
			out += replacement != replacements.end() ? replacement->second : name;
			continue;
		}

		out += text[i++];
	}
	return out;
}

std::string insert_uniform_block(const std::string &text, const std::string &block)
{
	auto current = 0u;
	while (current < text.size() && text[current] == '#') {
		auto next = text.find('\n', current);
		if (next == std::string::npos) {
			break;
		}
		auto line = text.substr(current, next - current + 1);
		if (line.starts_with("#line")) {
			break;
		}
		current = next + 1;
	}
	return text.substr(0, current) + block + text.substr(current);
}

std::string make_uniform_block(const std::vector<UniformDecl> &uniforms)
{
	if (uniforms.empty()) {
		return "";
	}

	std::string block;
	const auto &first = uniforms.front();
	block += spu::string_printf("#line %d %d\n", first.loc.lineno, first.loc.index);
	block += "layout(std140) uniform UB_GATHERED {\n";
	for (const auto &uniform: uniforms) {
		block += spu::string_printf("#line %d %d\n", uniform.loc.lineno, uniform.loc.index);
		block += "\t" + uniform.type + " " + uniform.member + uniform.array + ";\n";
	}
	block += spu::string_printf("#line %d %d\n", first.loc.lineno, first.loc.index);
	block += "} ub_gathered;\n\n";
	return block;
}
}  // namespace

namespace spu::libspu::spu_shader {

void preprocessGatheredUniformSources(Attrs &attrs, GatheredUniformSourceResult &result)
{
	result = {};
	const char *stage_names[] = {"vert", "tesc", "tese", "geom", "frag", "comp"};

	std::map<std::string, std::string> sources;
	std::vector<UniformDecl> uniforms;
	for (auto stage_name: stage_names) {
		auto *src = attrs.get<const char *>(stage_name, nullptr);
		if (src && *src) {
			sources[stage_name] = transform_uniform_decls(src, uniforms, attrs);
		}
	}
	if (uniforms.empty()) {
		return;
	}

	auto block_size = layout_uniforms(uniforms);
	set_gathered_uniform_result(result, uniforms, block_size);

	auto block = make_uniform_block(uniforms);
	auto replacements = make_uniform_replacements(uniforms);
	for (auto &[stage_name, text]: sources) {
		validate_gathered_uniform_declarations(text, replacements, attrs);
		text = replace_uniform_refs(text, replacements);
		if (text.find("main") != std::string::npos) {
			text = insert_uniform_block(text, block);
		}
		attrs.replace(stage_name.c_str(), text);
	}
	attrs.preserve();
}

}  // namespace spu::libspu::spu_shader
