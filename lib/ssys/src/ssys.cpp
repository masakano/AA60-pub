//
// File :
//
#include "default_output.h"
#include <ssys/ssys.h>

#ifdef _WIN32
#define quick_exit(x) exit(x)
#else
#include <cxxabi.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#endif

#ifndef _MSC_VER
#include <cxxabi.h>  // demangle
#endif

#include <iconv.h>
#include <chrono>
#include <numeric>
#include <random>
#include <regex>
#include <sys/stat.h>

namespace spu {
namespace {
std::chrono::high_resolution_clock::time_point s_offset_time;
}  // namespace

int32_t get_term_column()
{
#ifndef _WIN32
	struct winsize ws;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
		return ws.ws_col;
	}
#endif
	return Message::c_default_column_size;
}

uint64_t get_microsec()
{
	using namespace std::chrono;
	const auto delta = high_resolution_clock::now() - s_offset_time;
	return duration_cast<microseconds>(delta).count();
}

void set_microsec(uint64_t usec)
{
	using namespace std::chrono;
	s_offset_time = high_resolution_clock::now() - microseconds(usec);
}

void sleep_microsec(uint64_t usec, uint64_t msec, uint64_t sec)
{
	using namespace std::chrono;

	if (usec > 0 || msec > 0 || sec > 0) {
		// std::this_thread::sleep_for(microseconds(usec) + microseconds(msec) + seconds(sec));
		std::this_thread::sleep_for(microseconds(usec) + milliseconds(msec) + seconds(sec));
	}
}

#ifdef _MSC_VER
std::string demangle_string(const std::string &name) { return name; }
#else
std::string demangle_string(const std::string &name)
{
	// clang-format off
	constexpr struct {
		const char *src;
		const char *dst;
	} c_blacklists[] = {
	        {
			"std::.*::basic_string<char, std::char_traits<char>, std::allocator<char> >",
			"std::string"
		},
	        {
			"std::filesystem::.*::path", "std::filesystem::path"
		},
	        {
			nullptr, nullptr
		},
	};
	// clang-format on

	int32_t status;
	char *str = abi::__cxa_demangle(name.c_str(), nullptr, nullptr, &status);
	if (str) {
		std::string dname = str;
		for (auto i = 0; c_blacklists[i].src; i++) {
			dname = std::regex_replace(dname, std::regex(c_blacklists[i].src), c_blacklists[i].dst);
		}
		free(str);
		return dname;
	}
	return name;
}
#endif

std::string string_printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	va_list args_copy;
	va_copy(args_copy, args);
	int size = std::vsnprintf(nullptr, 0, fmt, args_copy);
	va_end(args_copy);

	if (size < 0) {
		va_end(args);
		return {};
	}

	std::string result(size, '\0');
	std::vsnprintf(result.data(), result.size() + 1, fmt, args);
	va_end(args);

	return result;
}

std::vector<std::string> extract_from_string(
        const std::string &line0, const std::string &delims, const std::string &cutoffs, bool is_strip_quote)
{
	enum {
		e_delim,
		e_dquote,
		e_squote,
		e_body,
		e_eol,
	};

	std::vector<std::string> items;
	std::string line = line0 + '#';  // sentinel

	auto sp = 0;
	auto state = e_delim;
	auto is_escape = false;

	const auto append = [&](std::string::size_type sp, std::string::size_type ep) {
		const auto spaces = std::string(" \t\n\r");

		sp = std::min(ep, line.find_first_not_of(spaces, sp));
		ep = line.find_last_not_of(spaces, std::max(sp, ep));

		if (line[sp] != '#') {
			std::string item;
			if (sp != std::string::npos && ep != std::string::npos) {
				if (is_strip_quote) {
					auto t = line[sp];
					auto b = line[ep];
					if ((t == '\'' && b == '\'') || (t == '\"' && b == '\"')) {
						sp++;
						ep--;
					}
				}
				for (auto i = sp; i <= ep; i++) {  // use signed
					item.push_back(line[i]);
				}
			}
			items.push_back(item);
		}
	};

	for (auto i = 0u; state != e_eol && i < line.size(); i++) {
		const auto is_delim = delims.find(line[i]) != std::string::npos;
		const auto is_cutoff = cutoffs.find(line[i]) != std::string::npos;

		if (is_escape) {
			is_escape = false;
		}
		else if (is_cutoff) {  // eg '{' '}'

			switch (state) {
			case e_delim: items.push_back({line[i]}); break;
			case e_dquote:
			case e_squote: break;
			case e_body:
				state = e_delim;
				append(sp, i - 1);
				items.push_back({line[i]});
				break;
			default: assert(0);
			}
		}
		else if (is_delim) {
			switch (state) {
			case e_delim:
			case e_dquote:
			case e_squote: break;
			case e_body:
				state = e_delim;
				if (is_cutoff) {
					items.push_back({line[i]});
				}
				append(sp, i - 1);
				break;
			default: assert(0);
			}
		}
		else if (line[i] == '\\') {  // escape
			is_escape = true;
		}
		else if (line[i] == '#') {  // eol
			switch (state) {
			case e_delim: state = e_eol; break;
			case e_dquote:
			case e_squote: break;
			case e_body:
				state = e_eol;
				append(sp, i - 1);
				break;
			default: assert(0);
			}
		}
		else if (line[i] == '"') {  // double quatation
			switch (state) {
			case e_delim:
				state = e_dquote;
				sp = i;
				break;
			case e_dquote: state = e_body; break;
			case e_squote: break;
			case e_body: state = e_dquote; break;
			default: assert(0);
			}
		}
		else if (line[i] == '\'') {  // single quatation
			switch (state) {
			case e_delim:
				state = e_squote;
				sp = i;
				break;
			case e_dquote: break;
			case e_squote: state = e_body; break;
			case e_body: state = e_squote; break;
			default: assert(0);
			}
		}
		else {  // body
			switch (state) {
			case e_delim:
				state = e_body;
				sp = i;
				break;
			case e_dquote:
			case e_squote:
			case e_body: break;
			default: assert(0);
			}
		}
	}
	if (state == e_dquote || state == e_squote) {
		append(sp, line.size() - 2);
	}
	return items;
}

std::filesystem::path make_cache_path(
        const std::filesystem::path &path, const std::filesystem::path &base_dir, const std::string &suffix)
{
	auto absolute_path = std::filesystem::absolute(path);
	auto relative_path = absolute_path.relative_path();
	auto cache_dir = base_dir / relative_path.parent_path();
	std::filesystem::create_directories(cache_dir);
	auto cache_path = base_dir / relative_path;
	return cache_path += suffix;
}

void sha256(const void *text, size_t len, uint8_t buf[32]);
void sha256(const void **textv, size_t *lenv, unsigned char buf[32]);

void hash256_t::set_from_buffer(const void *text, size_t len) { sha256(text, len, m_data); }

void hash256_t::set_from_parts(const void **textv, size_t *lenv) { sha256(textv, lenv, m_data); }

void hash256_t::randomize()
{
	std::mt19937 mt;
	std::random_device rd;

	mt.seed(rd());
	uint32_t data32[8];

	for (auto &i: data32) {
		i = mt();
	}
	memcpy(m_data, data32, sizeof(m_data));
}

void hash256_t::report(const char *str) const
{
	if (str && *str) aux_printf("%s: ", str);

	for (const auto &i: m_data) {
		aux_printf("%02x", i);
	}
	aux_printf("\n");
}

std::string peeloff_string(const std::string &str)
{
	const char *s0 = str.c_str();
	const char *s1 = s0 + strlen(s0) - 1;

	while (s0 != s1) {
		if (!isspace(*s0)) {
			break;
		}
		s0++;
	}

	while (s1 != s0) {
		if (!isspace(*s1)) {
			break;
		}
		s1--;
	}
	auto p0 = s0 - str.c_str();
	auto p1 = s1 - str.c_str() + 1;

	return p0 < p1 ? str.substr(p0, p1 - p0) : str;
}

std::string pretty_string(const std::string &str, int32_t max_len)
{
	if (max_len <= 0) {
		max_len = std::max(0, get_term_column() + max_len);
	}
	std::string result;
	auto *s = str.c_str();
	auto is_quote = false;
	auto length = 0;

	while (length < max_len - 4 && (*s != 0)) {
		if (*s == '"') {
			is_quote = !is_quote;
		}
		if (is_quote) {
			switch (*s) {
			case '\n': result += "\\n"; break;
			case '\t': result += "\\t"; break;
			case '\v': result += "\\v"; break;
			default: result += *s; break;
			}
			length++;
		}
		else {
			result += *s;
			length++;
		}
		s++;
	}
	if (length >= max_len - 4) {
		result += "...";
	}
	if (str.back() == '\n' && result.back() != '\n') result += "\n";
	return result;
}

ColumnAligner::ColumnAligner(const std::string &prefix, const std::string &delim)
        : m_prefix(prefix), m_delim(delim)
{
}

void ColumnAligner::puts(const std::vector<std::string> &lists)
{
	std::vector<std::string> trim_list;
	for (auto &item: lists) {
		auto sp = item.find_first_not_of(' ');
		if (sp == std::string::npos) {
			trim_list.push_back("-");
		}
		else {
			trim_list.emplace_back(std::string(item, sp));
		}
	}
	m_lists.emplace_back(trim_list);
}

std::vector<std::string> ColumnAligner::flush()
{
	std::vector<std::string> outputs;
	size_t col_count = 0;

	for (auto &list: m_lists) {
		col_count = std::max(col_count, list.size());
	}
	for (auto &list: m_lists) {
		while (list.size() < col_count) {
			list.emplace_back("-");
		}
	}
	std::vector<size_t> widths(col_count, 0);
	for (auto &list: m_lists) {
		for (auto i = 0u; i < col_count; i++) {
			widths[i] = std::max(widths[i], list[i].length());
		}
	}

	for (auto &list: m_lists) {
		std::string line = m_prefix;
		for (auto i = 0u; i < col_count; i++) {
			auto &item = list[i];
			line += item;
			line += std::string(widths[i] - item.length(), ' ');
			if (i != col_count - 1) {
				line += m_delim;
			}
		}
		line += '\n';
		outputs.push_back(line);
	}
	m_lists.clear();
	return outputs;
}

//
// streheap
//
Strheap::Strheap() { clear(); }

void Strheap::clear()
{
	for (auto &heap: m_heap) {
		delete heap.second;
	}
	m_heap.clear();
}

const char *Strheap::preserve(const char *str)
{
	if (str == nullptr) return nullptr;

	auto hash = hash32_t(str);
	auto it = m_heap.find(hash.value());
	if (it != std::end(m_heap)) {
		return it->second;
	}
	auto new_str = strdup(str);
	m_heap.insert({hash.value(), new_str});
	return new_str;
}
Strheap g_strheap;

//
// Message
//
Message::Message() { output = default_output; }

void Message::abort(const char *msg) { throw std::runtime_error(msg); }

void Message::putline(int32_t level, const char *symbol, const char *format, ...)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	enum { e_label_size = 128 };
	using label_t = std::array<char, e_label_size>;

	Message::message_t body = {};
	auto thread_id = std::this_thread::get_id();
	if (m_texts.find(thread_id) == std::end(m_texts)) {
		m_texts[thread_id] = {0};
	}

	auto &text = m_texts[thread_id];

	va_list arg;
	va_start(arg, format);
	vsnprintf(body.data(), body.size(), format, arg);
	va_end(arg);

	// time
	label_t time_text = {0};
	{
		const auto t = time(nullptr);
		const auto *lt = localtime(&t);
		sprintf(time_text.data(), "%02d:%02d:%02d", lt->tm_hour, lt->tm_min, lt->tm_sec);
	}

	// symbol
	label_t label_text = {0};
	if (symbol) {
		auto labels = extract_from_string(symbol, "::", "()");
		if (labels.size() == 1) {
			strcpy(label_text.data(), symbol);
		}
		else {
			for (auto i = 0u; i < labels.size(); i++) {
				if (labels[i] == "(") {
					if (i > 2) {
						strcpy(label_text.data(),
						       (labels[i - 2] + "::" + labels[i - 1]).c_str());
					}
					else if (i > 1) {
						strcpy(label_text.data(), labels[i - 1].c_str());
					}
					break;
				}
			}
		}
	}

	// text
	auto body_len = strlen(body.data());
	strncat(text.data(), body.data(), text.size() - 1);

	if (strlen(text.data()) == text.size() - 1) {
		strcpy(text.data() + text.size() - 5, "...\n");
	}
	if (body_len >= body.size() - 1 || strchr(body.data(), '\n') != nullptr) {
		if (output) {
			output(level, time_text.data(), label_text.data(), text.data());
		}
		if (is_query) {
			fprintf(stderr, "want core dump? [y/n](n) ");
			if (getchar() == 'y') {
				abort();  // quick core dump
			}
			fprintf(stderr, "\n");
			fflush(stderr);
		}
		m_texts.erase(m_texts.find(thread_id));
	}
}
Message g_message;

//
// File
//
File::File(const std::filesystem::path &path) : m_name(path) { openEmbedded(path); }

File::File(const std::filesystem::path &path, const std::string &mode, bool is_abort) : File(path)
{
	open(path, mode, is_abort);
}

File::~File() { close(); }

void File::pushBaseEach(const std::vector<std::filesystem::path> &paths)
{
	std::vector<std::filesystem::path> full_paths;
	for (const auto &path: paths) {
		auto rel_path = path.lexically_normal();

		if (path.is_absolute()) {
			full_paths.push_back(rel_path);
		}
		else {
			auto current = std::filesystem::current_path();
			while (true) {
				auto full_path = current / rel_path;
				if (std::filesystem::exists(full_path)) {
					full_paths.push_back(full_path);
				}
				auto parent = current.parent_path();
				if (parent == current) {
					break;
				}
				current = parent;
			}
		}
	}
	auto &bases = ms_bases();
	bases.insert(begin(bases), full_paths);  // "." is the highest
	for (auto &base: bases[0]) {
		aux_message(2, "push '%s'\n", base.string().c_str());
	}
}

void File::pushBase(const std::vector<std::filesystem::path> &paths)
{
	auto &bases = ms_bases();
	if (bases.empty()) {
		pushBaseEach({"."});
	}
	pushBaseEach(paths);
}

void File::pushBase(std::string path_list)
{
	auto str_paths = extract_from_string(path_list, ":");
	std::vector<std::filesystem::path> paths(str_paths.size());
	std::copy(begin(str_paths), end(str_paths), begin(paths));
	pushBase(paths);
}

void File::popBase()
{
	auto &bases = ms_bases();
	assert(bases.size() > 1);
	for (auto &base: bases[0]) {
		aux_message(2, "pop '%s'\n", base.c_str());
	}
	bases.erase(begin(bases));
}

std::vector<std::filesystem::path> File::getBases()
{
	std::vector<std::filesystem::path> all_bases;
	for (auto &bases: ms_bases()) {
		for (auto &base: bases) {
			if (find(begin(all_bases), end(all_bases), base) == end(all_bases)) {
				all_bases.push_back(base);
			}
		}
	}
	return all_bases;
}

std::filesystem::path File::searchPath(const std::filesystem::path &path, bool is_abort)
{
	aux_message(2, "search \"%s\"\n", path.string().c_str());

	if (path == "") {
		aux_error(is_abort, "no input path\n");
		return std::filesystem::path();
	}

	auto &embeds = ms_embeds();
	if (embeds.find(path) != end(embeds)) {
		aux_message(1, "found in embeded \"%s\"\n", path.string().c_str());
		return path;
	}

	auto rel_path = path.lexically_normal();
	if (path.is_absolute()) {
		return rel_path;
	}

	auto &bases = ms_bases();
	if (bases.empty()) {
		File::pushBase(std::vector<std::filesystem::path>{});
	}

	for (auto &base_set: bases) {
		for (auto &base: base_set) {
			auto full_path = (base / std::filesystem::path(rel_path)).lexically_normal();

			if (full_path == std::filesystem::path("C")) {
				assert(0);
			}
			aux_message(2, "search '%s'\n", full_path.string().c_str());

			if (std::filesystem::exists(full_path)) {
				aux_message(2, "found '%s'\n", full_path.string().c_str());
				return full_path;
			}
		}
	}
	aux_error(is_abort, "\"%s\": file not found. (try '-message.level 1')\n", rel_path.string().c_str());
	return std::filesystem::path();
}

bool File::open(const std::filesystem::path &path, const std::string &mode, bool is_abort)
{
	aux_error(m_fp, "%s: duplicate open\n", m_name.c_str());

	m_base = nullptr;
	m_curr = 0;
	m_size = 0;

	if (openEmbedded(path)) {
		return true;
	}
	if (path == "-") {
		m_fp = (mode[0] == 'r') ? stdin : stdout;
		m_name = "-";
		return true;
	}
	if (mode[0] == 'w' || mode[0] == 'a') {
		m_name = path;
		if ((m_fp = fopen(m_name.string().c_str(), mode.c_str()))) {
			return true;
		}
	}
	else if (mode[0] == 'r') {
		m_name = searchPath(path, is_abort);
		if ((m_fp = fopen(m_name.string().c_str(), mode.c_str()))) {
			return true;
		}
	}
	aux_error(is_abort, "%s: cannot open\n", m_name.string().c_str());
	return false;
}

void File::flush() { fflush(m_fp); }

void File::close()
{
	if (m_fp) {
		if (m_fp != stdin && m_fp != stdout) {
			fclose(m_fp);
		}
		m_fp = nullptr;
	}
	m_base = nullptr;
	m_curr = 0;
	m_size = 0;
}

int64_t File::mtime() const
{
	if (m_base) {
		return 0;
	}
	if (std::filesystem::exists(m_name)) {
		const auto tp = std::filesystem::last_write_time(m_name);
		const auto sec = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch());
		return sec.count();
	}
	return -1;
}

// bool File::exists() const { return m_size > 0 ? true : std::filesystem::exists(m_name); }

size_t File::size() const
{
	if (m_size) {
		return m_size;
	}
	if (std::filesystem::exists(m_name)) {
		return std::filesystem::file_size(m_name);  // not work in msys?
	}
	return 0;
}

void File::rewind()
{
	if (m_base) {
		// do nothing
	}
	else {
		aux_error(!m_fp, "%s: cannnot rewind (forget calling open()?) \n", m_name.string().c_str());
		::rewind(m_fp);
	}
	m_curr = 0;
}

size_t File::tell()
{
	if (m_base) {
		return m_curr;
	}
	aux_error(!m_fp, "%s: cannnot tell (forget calling open()?)\n", m_name.string().c_str());
	return m_curr = ::ftell(m_fp);
}

bool File::getchar(uint8_t &c)
{
	if (m_base) {
		if (m_curr < m_size) {
			c = m_base[m_curr++];
			return true;
		}
		return false;
	}
	else {
		auto len = fread(&c, 1, 1, m_fp);
		return len == 1;
	}
}

size_t File::read(void *buf, size_t count, bool is_abort)
{
	size_t read_count = 0;
	if (m_base) {
		read_count = m_size > m_curr ? std::min(m_size - m_curr, count) : 0;
		memcpy(buf, m_base + m_curr, read_count);
	}
	else if (!m_fp) {
		read_count = 0;
	}
	else if (buf == nullptr) {
		read_count = fseek(m_fp, count, SEEK_CUR) == 0 ? count : 0;
	}
	else {
		read_count = fread(buf, 1, count, m_fp);
	}
	m_curr += read_count;

	aux_error(
	        is_abort && read_count != count, "%s: read error in %ld/%ld bytes (forget 'rb'?)\n",
	        m_name.string().c_str(), read_count, count);

	return read_count;
}

size_t File::write(const void *buf, size_t count, bool is_abort)
{
	int32_t ret = 0;
	if (!m_fp) {
		ret = 0;
	}
	else if (buf == nullptr) {
		ret = fseek(m_fp, count, SEEK_CUR);
		m_curr += count;
	}
	else {
		ret = fwrite(buf, 1, count, m_fp);
		m_curr += count;
	}
	aux_error(
	        is_abort && ret != int32_t(count), "%s: write error in %ld/%ld bytes\n",
	        m_name.string().c_str(), ret, count);
	return ret;
}

void File::printf(const char *format, ...)
{
	if (m_fp) {
		va_list arg;
		va_start(arg, format);
		vfprintf(m_fp, format, arg);
		va_end(arg);
	}
}

int32_t File::scanf(const char *format, ...)
{
	aux_error(!m_fp, "cannnot scan\n");
	int32_t ret;
	va_list arg;
	va_start(arg, format);
	ret = vfscanf(m_fp, format, arg);
	va_end(arg);
	return ret;
}

bool File::getline(std::string &line)
{
	aux_error(!m_fp, "cannnot getline\n");
	Message::line_t linebuf;

	line.clear();
	while (true) {
		if (fgets(linebuf.data(), linebuf.size(), m_fp) == nullptr) {
			return false;
		}
		auto len = strlen(linebuf.data());
		if (len > 1 && linebuf[len - 2] == '\\') {
			linebuf[len - 2] = 0;
			line += linebuf.data();
		}
		else {
			line += linebuf.data();
			return true;
		}
	}
}

bool File::getlist(
        std::vector<std::string> &list, const std::string &delim, const std::string &cutoff,
        bool is_strip_quote)
{
	bool ret;
	do {
		std::string line;
		if ((ret = getline(line))) {
			list = extract_from_string(line, delim, cutoff, is_strip_quote);
		}
	} while (ret && list.empty());
	return ret;
}

//
// FIleEmbed
//
bool File::openEmbedded(const std::filesystem::path &path)
{
	auto &embeds = ms_embeds();
	auto it = embeds.find(path);
	if (it != end(embeds)) {
		auto &e = it->second;
		m_base = e.data;
		m_size = e.size;
		m_curr = 0;
		return true;
	}
	return false;
}

void File::embed(const std::filesystem::path &path, const void *data, size_t size)
{
	auto &embeds = ms_embeds();
	if (embeds.find(path) != end(embeds)) {
		return;  // skip
	}
	aux_message(1, "add to embed '%s'\n", path.string().c_str());
	embeds[path] = {(const uint8_t *)data, size};
}

void File::unembed(const std::filesystem::path &path)
{
	ms_embeds().erase(path);
	aux_error(true, "\"%s\": not found in embed files\n", path.string().c_str());
}

// CharsetConverter
namespace {
std::vector<char> conv_raw(void *cd, const char *srcbuf, size_t srcbyte, uint32_t stride)
{
	assert(cd);
	std::vector<char> dstbuf(srcbyte * 16, 0);  // conservetive

	char *src = const_cast<char *>(srcbuf);
	char *dst = dstbuf.data();
	auto dstbyte = dstbuf.size();

	if (iconv(cd, &src, &srcbyte, &dst, &dstbyte) != 0) {
		aux_message(1, "cannot convert. do nothing.\n");
		iconv(cd, nullptr, nullptr, nullptr, nullptr);
		memcpy(dst, src, srcbyte);
	}
	static char s_zero[16] = {0};
	for (auto n = 0u; n < dstbuf.size(); n += stride) {
		if (memcmp(&dstbuf[n], s_zero, stride) == 0) {
			dstbuf.resize(n);
			return dstbuf;
		}
	}
	assert(0);
	return {};
}
}  // namespace

void CharsetConverter::init(const char *dst_type, const char *src_type)
{
	aux_error(
	        (m_cd = iconv_open(dst_type, src_type)) == iconv_t(-1),
	        "cannot open '%s' to '%s'. (confirm type by 'iconv --list')\n", src_type, dst_type);
}
std::vector<char> CharsetConverter::get(const char *srcbuf, size_t srcbyte, uint32_t stride)
{
	assert(m_cd);
	return conv_raw(m_cd, srcbuf, srcbyte, stride);
}

CharsetConverter::~CharsetConverter()
{
	if (m_cd) iconv_close(m_cd);
}
}  // namespace spu
