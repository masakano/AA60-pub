//
// Loader :
//
#include "shader_loader.h"
#include "spu_object.h"

#ifdef _WIN32
#define unlink _unlink
#define popen _popen
#define pclose _pclose
#define c_cpp std::string("cpp ")
#define c_delim std::string("\"")
#else
#include <unistd.h>  // unlink
#define c_cpp std::string("LANG=en cpp ")
#define c_delim std::string("'")
#endif

namespace {
constexpr const char *c_preface = "#version 460\n";
constexpr const char *c_cppopt = "-D__GLSL__ ";

void append_source_name_attrs(spu::Attrs &attrs, const std::vector<std::string> &names)
{
	attrs.emplace_back("shader_source.count", uint32_t(names.size()));
	for (auto i = 0u; i < names.size(); i++) {
		auto key = spu::string_printf("shader_source.%d.name", i);
		attrs.emplace_back(spu::g_strheap.preserve(key.c_str()), names[i]);
	}
}
}  // namespace

namespace spu {
template<> inline size_t serialize(uint8_t *heap, bool is_dry, const libspu::shader::Loader &o)
{
	auto *hp = heap;
	hp += serialize(hp, is_dry, o.m_signature);
	hp += serialize(hp, is_dry, o.m_preface);
	hp += serialize(hp, is_dry, o.m_args);
	hp += serialize(hp, is_dry, o.m_names);
	hp += serialize(hp, is_dry, o.m_attrs);
	return hp - heap;
}

template<> inline size_t deserialize(const uint8_t *heap, libspu::shader::Loader &o)
{
	auto *hp = heap;
	hp += deserialize(hp, o.m_signature);
	hp += deserialize(hp, o.m_preface);
	hp += deserialize(hp, o.m_args);
	hp += deserialize(hp, o.m_names);
	hp += deserialize(hp, o.m_attrs);  // heap must be non-volatile
	return hp - heap;
}
}  // namespace spu

namespace spu::libspu::shader {

bool Loader::loadFromCache(const std::filesystem::path &path)
{
	auto cache_path = make_cache_path(path);

	if (std::filesystem::exists(cache_path)) {
		spu_message(1, "%s: read from cache\n", cache_path.string().c_str());
		m_heap = read_from_file<std::vector<uint8_t>>(cache_path);
		deserialize(m_heap.data(), *this);
		return true;
	}
	return false;
}

void Loader::saveToCache(const std::filesystem::path &path)
{
	auto size = serialize(nullptr, true, *this);
	std::vector<uint8_t> heap(size);
	serialize(heap.data(), false, *this);
	auto cache_path = make_cache_path(path);
	spu_message(1, "%s: save to cache\n", cache_path.string().c_str());

	printf("saveToCache: %s\n", cache_path.c_str());
	File file(cache_path, "wb");
	file.write(heap.data(), heap.size());
}

void Loader::load(const std::filesystem::path &path, Attrs &attrs)
{
	auto is_use_cache = attrs.pick<bool>("use_cache", 0);
	auto full_path = File::searchPath(path);

	if (is_use_cache) {
		if (loadFromCache(full_path)) {
			return;
		}
	}

	attrs.peek("use_cpp", "deprecated");
	m_preface = attrs.pick("preface", c_preface);
	m_signature = path.string() + ":";

	for (auto &c: m_preface) {
		m_signature += c == '\n' ? ' ' : c;
	}

	std::vector<std::filesystem::path> incdirs;
	incdirs.push_back(full_path.parent_path());  // add current explisitly
	for (auto &dir: File::getBases()) {
		incdirs.push_back(dir);
	}

	std::string cppopt = c_cppopt;
	for (auto &incdir: incdirs) {
		if (incdir != "") {
			m_args.push_back(std::string("-I") + incdir.string());
			cppopt += m_args.back() + " ";
		}
	}

	std::map<const char *, const char *> defs;

	for (auto &attr: attrs) {
		if (attr.test("def_", 4)) {
			attrs.addToLog(attr);
			defs[attr.key().c_str()] = static_cast<const char *>(attr);
		}
	}

	for (auto &def: defs) {
		auto strtailcmp = [](const char *s0, const char *s1) {
			return strcmp(s0 + strlen(s0) - strlen(s1), s1);
		};
		if (def.second == nullptr) {
			attrs.report("attrs");
			aux_error(true, "'%s': def macro is not set (nullptr)\n", def.first);
		}

		if (strtailcmp(def.first, "_path") == 0 && (def.second[0] != '<' && def.second[0] != '\"')) {
			attrs.report("attrs");
			aux_error(
			        true, "'%s': suspicious def, you mean <%s> or \"%s\"?\n", def.first,
			        def.second);
		}

		m_signature += std::string(":") + def.first + "=" + def.second;
		m_args.push_back(std::string("-D") + def.first + "=" + c_delim + def.second + c_delim);
		cppopt += m_args.back() + " ";
	}
	auto cmdbuf = c_cpp + full_path.string() + " " + cppopt;

	m_names.push_back(full_path.string());

	readSource(cmdbuf);
	parseSources();
	append_source_name_attrs(m_attrs, m_names);
	postprocessSources(full_path);
	if (is_use_cache) {
		saveToCache(full_path);
	}
}

void Loader::readSource(const std::string &cmdbuf)
{
	spu_message(2, "%s\n", cmdbuf.c_str());
	FILE *fp = popen(cmdbuf.c_str(), "r");
	aux_error(fp == nullptr, "%s : cannot open\n", m_names.back().c_str());

	int32_t c;
	while ((c = fgetc(fp)) != EOF) {
		m_text.push_back(c);
	}
	pclose(fp);
}

void Loader::parseSources()
{
	std::string line;
	int32_t type;
	while ((type = getline(line)) != shader::Loader::e_eof) {
		bool is_directive = type & e_directive;
		type &= ~e_directive;

		for (auto &src: m_sources) {
			auto mask = m_masks.at(src.first);
			if ((type & mask) != 0) {
				auto &text = src.second;
				text = is_directive ? line + text : text + line;
			}
		}
	}
}

void Loader::postprocessSources(const std::filesystem::path &full_path)
{
	for (auto &src: m_sources) {
		const auto &symbol = src.first;
		auto &text = src.second;

		if (text.find("main") == std::string::npos) {
			text = "";
		}
		else {
			text = m_preface + text;
		}
		if (g_message.level >= 3) {
			if (!text.empty()) {
				auto save_path = make_cache_path(full_path).replace_extension(symbol);
				File file(save_path, "w");
				file.write(text.c_str(), text.length());
			}
		}
		if (!text.empty()) {  // necessary for serizier bug (need fix)
			m_attrs.emplace_back(symbol.c_str(), text.c_str());
		}
	}
}

int32_t Loader::getline(std::string &line)
{
	if (m_current >= m_text.size()) {
		return Loader::e_eof;
	}

	line.clear();
	while (m_current < m_text.size()) {
		char c = m_text[m_current++];
		line += c;
		if (c == '\n') {
			break;
		}
	}

	// #line
	if (line[0] == '#') {
		// glsl directive
		if (line[1] == '#') {
			line = std::string(spu::string_printf("%s", line.c_str() + 1));
			return m_type | e_directive;
		}
		char buf[256];
		sscanf(line.c_str(), "# %d \"%s\"", &m_lineno, buf);
		buf[strlen(buf) - 1] = 0;

		auto p = find(begin(m_names), end(m_names), buf);
		if (p == end(m_names)) {
			m_names.emplace_back(buf);
			m_index = m_names.size() - 1;
		}
		else {
			m_index = p - begin(m_names);
		}
		line = spu::string_printf("#line %d %d\n", m_lineno, m_index);
		return m_type;
	}

	std::vector<std::string> list = extract_from_string(line);
	if ((isalpha(line[0]) != 0) && !list.empty()) {
		if (*(end(list[0]) - 1) == ':') {
			m_type = 0;
			for (auto &key: extract_from_string(list[0], "|:")) {
				int32_t mask = m_masks.at(key);
				aux_error(
				        mask == 0, "%s:%d: \"%s\": unknown label\n", m_names[m_index].c_str(),
				        m_lineno, list[0].c_str());
				m_type |= mask;
			}
			line = spu::string_printf("#line %d %d\n", m_lineno, m_index);
		}
	}
	m_lineno++;
	return m_type;
}
}  // namespace spu::libspu::shader
