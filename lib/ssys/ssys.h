//
// collection of system class
//
#pragma once

#ifdef __clang__
#pragma clang diagnostic ignored "-Wpragma-once-outside-header"
#endif

#include <cassert>
#include <cstring>
#include <cstdarg>
#include <cstddef>
#include <cmath>
#include <functional>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <unordered_set>
#include <filesystem>
#include <thread>
#include <mutex>
#include <algorithm>
#include <type_traits>
#include <compare>

#include "murmur3.h"  // murmur3 hash
#include "util_vector.h"

#ifdef _WIN32
#define __PRETTY_FUNCTION__ __func__
#endif

#define aux_printf(...) spu::g_message.putline(0, 0, __VA_ARGS__)
#define aux_abort(msg) spu::g_message.abort(msg);
#define aux_message(msg_level, ...)                                                  \
	if (spu::g_message.level >= msg_level) {                                     \
		spu::g_message.putline(msg_level, __PRETTY_FUNCTION__, __VA_ARGS__); \
	}

#define aux_error(cond, ...)                 \
	if (cond) {                          \
		aux_message(0, __VA_ARGS__); \
		aux_abort("aux_error");      \
	}

namespace spu {

template<class T> inline T align_up(T x, T a) { return x % a == 0 ? x : (x + a) / a * a; }

template<class T, class U> inline T bit_cast(U x)
{
	union {
		U u;
		T t;
	} u;
	u.u = x;
	return u.t;
}

template<class target_t, class base_t> std::vector<target_t> select_objects(const std::vector<base_t> &objects)
{
	std::vector<target_t> results;
	for (auto &object: objects) {
		auto ret = dynamic_cast<target_t>(object);
		if (ret) results.push_back(ret);
	}
	return results;
}

uint32_t opengl_const(const char *str);
const char *opengl_const(int32_t code);

std::string string_printf(const char *format, ...);

std::vector<std::string> extract_from_string(
        const std::string &line, const std::string &delims = " \t\r\n", const std::string &cutoffs = "",
        bool is_strip_quote = false);

std::string peeloff_string(const std::string &str);
std::string pretty_string(const std::string &str, int32_t max_len = 0);
std::string demangle_string(const std::string &name);

int32_t get_term_column();

uint64_t get_microsec();
void set_microsec(uint64_t usec);
void sleep_microsec(uint64_t usec, uint64_t msec = 0, uint64_t sec = 0);

std::filesystem::path make_cache_path(
        const std::filesystem::path &path, const std::filesystem::path &base_dir = "cache",
        const std::string &suffix = ".cache");

template<class T> struct optional_t {
	bool hit;
	const T value;

	optional_t() = default;
	optional_t(bool hit, const T &value) noexcept : hit(hit), value(value) {}
	optional_t(const T &value) noexcept : hit(true), value(value) {}
};

struct hash32_t {
	inline constexpr hash32_t() : text(nullptr), data(0) {}
	inline constexpr hash32_t(uint32_t data) : text(nullptr), data(data) {}
	inline constexpr hash32_t(const void *text, std::size_t len)
	        : text(nullptr), data(murmur3::murmur3d((const char *)text, len))
	{
	}
	inline constexpr hash32_t(const char *text)
	        : text(text), data(text ? murmur3::murmur3d(text, murmur3::strlen(text)) : 0)
	{
	}
	inline constexpr void replace(const char *new_text) { text = new_text; }
	inline constexpr bool valid() const { return data != 0; }
	inline constexpr uint32_t value() const { return data; }
	inline constexpr const char *c_str() const { return text; }
	inline constexpr bool operator==(const hash32_t &h) const { return data == h.data; }
	inline constexpr std::strong_ordering operator<=>(const hash32_t &h) const { return data <=> h.data; }

private:
	const char *text;
	uint32_t data;
};
inline constexpr hash32_t operator""_h32(const char *str, std::size_t len) { return hash32_t(str, len); }

class hash256_t {
public:
	hash256_t() = default;

	explicit hash256_t(const void *text, size_t len) { set_from_buffer(text, len); }
	explicit hash256_t(const void **textv, size_t *lenv) { set_from_parts(textv, lenv); }

	template<class T> explicit hash256_t(const T &object)
	{
		static_assert(std::is_trivially_copyable<T>::value, "not trivially copyable");
		set_from_buffer(&object, sizeof(T));
	}

	template<class T> explicit hash256_t(const std::vector<T> &objects)
	{
		static_assert(std::is_trivially_copyable<T>::value, "not trivially copyable");
		set_from_buffer(objects.data(), objects.size() * sizeof(T));
	}

	void randomize();
	void report(const char *str) const;

	uint8_t *hash() { return m_data; }
	const uint8_t *hash() const { return m_data; }

	friend bool operator==(const hash256_t &d0, const hash256_t &d1) noexcept
	{
		return memcmp(&d0, &d1, sizeof(hash256_t)) == 0;
	}

	friend std::strong_ordering operator<=>(const hash256_t &d0, const hash256_t &d1) noexcept
	{
		const auto cmp = memcmp(&d0, &d1, sizeof(hash256_t));
		if (cmp < 0) {
			return std::strong_ordering::less;
		}
		if (cmp > 0) {
			return std::strong_ordering::greater;
		}
		return std::strong_ordering::equal;
	}

private:
	void set_from_buffer(const void *text, size_t len);
	void set_from_parts(const void **textv, size_t *lenv);

	uint8_t m_data[32];
};

class Strheap {
public:
	Strheap();
	void clear();
	const char *preserve(const char *str);
	const std::map<uint32_t, const char *> get() const { return m_heap; }

private:
	std::map<uint32_t, const char *> m_heap;
};
extern Strheap g_strheap;

struct Message {
	static constexpr int32_t c_line_size = 1024;
	static constexpr int32_t c_message_size = 4096;
	static constexpr int32_t c_default_column_size = 120;

	using line_t = std::array<char, c_line_size>;
	using message_t = std::array<char, c_message_size>;
	using output_t = std::function<void(int level, const char *time, const char *label, const char *msg)>;

	output_t output = nullptr;
	bool is_query = false;
	int32_t level = 0;

	Message();

	void putline(int32_t level, const char *symbol, const char *format, ...);
	[[noreturn]] void abort(const char *msg = "(abort)");

private:
	std::mutex m_mutex;
	std::map<std::thread::id, message_t> m_texts;
};
extern Message g_message;  // for aux_printf

class File {
public:
	explicit File(const std::filesystem::path &path = "");
	File(const std::filesystem::path &path, const std::string &mode, bool is_abort = true);
	virtual ~File();

	bool open(const std::filesystem::path &path, const std::string &mode, bool is_abort = true);

	void flush();
	void close();
	void rewind();
	size_t size() const;
	int64_t mtime() const;
	size_t tell();
	size_t read(void *buf, size_t count, bool is_abort = true);
	size_t write(const void *buf, size_t count, bool is_abort = true);
	int32_t scanf(const char *format, ...);
	void printf(const char *format, ...);
	bool getchar(uint8_t &c);
	bool getline(std::string &line);

	bool getlist(
	        std::vector<std::string> &list, const std::string &delim = " \t\r\n",
	        const std::string &cutoff = "", bool is_strip_quoate = false);

	const std::filesystem::path &name() const noexcept { return m_name; }
	FILE *fp() const noexcept { return m_fp; }

	static void pushBase(const std::vector<std::filesystem::path> &paths);
	static void pushBase(std::string path_list);
	static void popBase();
	static void embed(const std::filesystem::path &path, const void *data, size_t size);
	static void unembed(const std::filesystem::path &path);
	static std::vector<std::filesystem::path> getBases();
	static std::filesystem::path searchPath(const std::filesystem::path &path, bool is_abort = true);

private:
	struct Embed {
		const uint8_t *data;
		size_t size;
	};
	std::filesystem::path m_name;

	FILE *m_fp = nullptr;
	const uint8_t *m_base = nullptr;
	size_t m_curr = 0;
	size_t m_size = 0;

	static std::vector<std::vector<std::filesystem::path>> &ms_bases()
	{
		static std::vector<std::vector<std::filesystem::path>> v;
		return v;
	}

	static std::map<std::filesystem::path, Embed> &ms_embeds()
	{
		static std::map<std::filesystem::path, Embed> v;
		return v;
	}

	bool openEmbedded(const std::filesystem::path &path);
	static void pushBaseEach(const std::vector<std::filesystem::path> &paths);
};

template<class vector_t = std::string, class file_t = File>
vector_t read_from_file(const std::filesystem::path &path, bool is_abort = true)
{
	vector_t data;
	file_t file;
	if (file.open(path, "rb", is_abort)) {
		auto len = file.size();
		auto elem_size = sizeof(data[0]);
		data.resize((len + elem_size - 1) / elem_size);
		file.read(&data[0], len);
	}
	return data;
}

class ColumnAligner {
public:
	ColumnAligner(const std::string &prefix = "", const std::string &delim = "");
	void puts(const std::vector<std::string> &lists);
	std::vector<std::string> flush();
	void clear() { m_lists.clear(); }

private:
	std::vector<std::vector<std::string>> m_lists;
	const std::string m_prefix;
	const std::string m_delim;
};

class CharsetConverter {
public:
	CharsetConverter() = default;
	CharsetConverter(const char *dst_type, const char *src_type) { init(dst_type, src_type); }
	~CharsetConverter();

	void init(const char *dst_type, const char *src_type);
	std::vector<char> get(const char *srcbuf, size_t srcbyte, uint32_t stride = 1);

	CharsetConverter(const CharsetConverter &) = delete;
	CharsetConverter &operator=(const CharsetConverter &) = delete;

private:
	void *m_cd = nullptr;
};

class Seconds {
public:
	void update()
	{
		auto absolute_current = double(get_microsec()) / 1000000;
		m_delta = (m_fixedDelta >= 0 ? m_fixedDelta : absolute_current - m_prev) * m_pace;
		m_prev = absolute_current;
		m_current += m_delta;
		m_count++;
	}
	void reset()
	{
		m_current = 0;
		m_count = 0;
		update();
	}

	void setPace(double pace) { m_pace = pace; }
	void setFixedDelta(double delta) { m_fixedDelta = delta; }

	double pace() const { return m_pace; }
	double fixedDelta() const { return m_fixedDelta; }
	double current() const { return m_current; }
	double delta() const { return m_delta; }
	int64_t count() const { return m_count; }

private:
	double m_prev = 0;
	double m_current = 0;
	double m_delta = 0;
	double m_pace = 1.0;
	double m_fixedDelta = -1;
	int64_t m_count = 0;
};
}  // namespace spu
