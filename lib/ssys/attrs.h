//
// Attr :
//
#pragma once
#include "ssys.h"
#include "simd.h"
#include "serializer.h"
#include <source_location>

namespace spu {

struct Attr {  // no member prefix

	using callback_t = void (*)(void *);
	template<class T>
	using enable_ptr_last_resort_t = std::enable_if_t<!std::is_same_v<std::remove_cv_t<T>, double>>;

	enum attr_type_t : uint32_t {
		e_undef = 0,     ///< undefined
		e_hash,          ///< hash32_t
		e_vec4f,         ///< vec4f_t
		e_vec4i,         ///< vec4i_t
		e_float,         ///< float
		e_int,           ///< int32_t
		e_uint,          ///< uint32_t
		e_float_ptr,     ///< float*
		e_int_ptr,       ///< int32_t*
		e_uint_ptr,      ///< uint32_t*
		e_char_ptr,      ///< const char*
		e_char_ptr_ptr,  ///< const char**
		e_void_ptr_ptr,  ///< void**
		e_void_ptr,      ///< void*
		e_callback,      ///< callback_t*
		e_nullptr,       ///< nullptr
		e_attr_type_max,
	};

	union attr_value_t {
		float f;
		int64_t si;
		uint64_t ui;
		const void *cp;
		void *vp;
		callback_t cb;
		vec4f_t fv;
		vec4i_t iv;
		hash32_t h;
	};

	Attr() = default;

	constexpr Attr(const hash32_t &key, const Attr &attr) noexcept
	        : m_key(key), m_type(attr.m_type), m_value(attr.m_value)
	{
	}
	template<class T> constexpr Attr(const hash32_t &key, const T **value) noexcept : Attr(value)
	{
		m_key = key;
	}
	template<class T> constexpr Attr(const hash32_t &key, const T *value) noexcept : Attr(value)
	{
		m_key = key;
	}
	template<class T> constexpr Attr(const hash32_t &key, const T &value) noexcept : Attr(value)
	{
		m_key = key;
	}
	explicit constexpr Attr(std::nullptr_t) noexcept : m_type(e_nullptr) { m_value.cp = nullptr; }
	explicit constexpr Attr(vec4f_t value) noexcept : m_type(e_vec4f) { m_value.fv = value; }
	explicit constexpr Attr(vec3f_t value) noexcept : m_type(e_vec4f) { m_value.fv = value; }
	explicit constexpr Attr(vec2f_t value) noexcept : m_type(e_vec4f) { m_value.fv = value; }
	explicit constexpr Attr(vec4i_t value) noexcept : m_type(e_vec4i) { m_value.iv = value; }
	explicit constexpr Attr(vec3i_t value) noexcept : m_type(e_vec4i) { m_value.iv = value; }
	explicit constexpr Attr(vec2i_t value) noexcept : m_type(e_vec4i) { m_value.iv = value; }
	explicit constexpr Attr(callback_t value) noexcept : m_type(e_callback) { m_value.cb = value; }
	explicit constexpr Attr(float value) noexcept : m_type(e_float) { m_value.f = value; }
	explicit constexpr Attr(double value) noexcept : m_type(e_float) { m_value.f = value; }
	explicit constexpr Attr(uint64_t value) noexcept : m_type(e_uint) { m_value.ui = value; }
	explicit constexpr Attr(uint32_t value) noexcept : m_type(e_uint) { m_value.ui = value; }
	explicit constexpr Attr(uint16_t value) noexcept : m_type(e_uint) { m_value.ui = value; }
	explicit constexpr Attr(uint8_t value) noexcept : m_type(e_uint) { m_value.ui = value; }
	explicit constexpr Attr(int64_t value) noexcept : m_type(e_int) { m_value.si = value; }
	explicit constexpr Attr(int32_t value) noexcept : m_type(e_int) { m_value.si = value; }
	explicit constexpr Attr(int16_t value) noexcept : m_type(e_int) { m_value.si = value; }
	explicit constexpr Attr(int8_t value) noexcept : m_type(e_int) { m_value.si = value; }
	explicit constexpr Attr(const hash32_t &value) noexcept : m_type(e_hash) { m_value.h = value; }

	explicit Attr(std::string &&value) noexcept : m_type(e_char_ptr)
	{
		m_value.cp = g_strheap.preserve(value.c_str());
	}
	explicit Attr(const std::string &value) noexcept : m_type(e_char_ptr)
	{
		m_value.cp = g_strheap.preserve(value.c_str());
	}

	explicit constexpr Attr(const float *value) noexcept : m_type(e_float_ptr) { m_value.cp = value; }
	explicit constexpr Attr(const int32_t *value) noexcept : m_type(e_int_ptr) { m_value.cp = value; }
	explicit constexpr Attr(const uint32_t *value) noexcept : m_type(e_uint_ptr) { m_value.cp = value; }
	explicit constexpr Attr(const char *value) noexcept : m_type(e_char_ptr) { m_value.cp = value; }
	explicit constexpr Attr(const char **value) noexcept : m_type(e_char_ptr_ptr) { m_value.cp = value; }

	template<class T, class = enable_ptr_last_resort_t<T>>
	explicit constexpr Attr(T *value) noexcept : m_type(e_void_ptr)  // last resort, no const
	{
		m_value.vp = value;
	}
	template<class T, class = enable_ptr_last_resort_t<T>>
	explicit constexpr Attr(const T *value) noexcept : m_type(e_void_ptr)  // last resort, const
	{
		m_value.cp = value;
	}
	explicit constexpr Attr(void **value) noexcept : m_type(e_void_ptr_ptr) { m_value.vp = value; }
	explicit constexpr Attr(void *const *value) noexcept : m_type(e_void_ptr_ptr) { m_value.cp = value; }
	template<
	        class T, class = std::enable_if_t<
	                         !std::is_same_v<std::remove_cv_t<T>, void>
	                         && std::is_same_v<enable_ptr_last_resort_t<T>, void>>>
	explicit constexpr Attr(T **value) noexcept : m_type(e_void_ptr_ptr)  // last resort, no const
	{
		m_value.vp = value;
	}
	explicit constexpr Attr(const std::filesystem::path &value) noexcept = delete;  // black list

	operator callback_t() const
	{
		castCheck(m_type, e_callback);
		return m_value.cb;
	}
	operator const char **() const
	{
		castCheck(m_type, e_char_ptr_ptr);
		return (const char **)m_value.vp;
	}
	template<class T> operator T *() const
	{
		castCheck(m_type, e_void_ptr);
		return (T *)m_value.vp;
	}
	template<class T> operator T **() const
	{
		castCheck(m_type, e_void_ptr_ptr);
		return (T **)m_value.vp;
	}

	explicit operator float() const { return castToFloat(); }
	explicit operator uint32_t() const { return castToUint(); }
	explicit operator int32_t() const { return castToInt(); }
	explicit operator uint64_t() const { return castToUint(); }
	explicit operator int64_t() const { return castToInt(); }
	explicit operator bool() const { return castToBool(); }

	operator hash32_t() const { return castToHash(); }
	operator vec4f_t() const { return castToVec4f(); }
	operator vec3f_t() const { return vec3f_t(castToVec4f()); }
	operator vec2f_t() const { return vec2f_t(castToVec4f()); }

	operator vec4i_t() const { return castToVec4i(); }
	operator vec3i_t() const { return vec3i_t(castToVec4i()); }
	operator vec2i_t() const { return vec2i_t(castToVec4i()); }

	operator const int32_t *() const { return castToIntPtr(); }
	operator const uint32_t *() const { return castToUintPtr(); }
	operator const float *() const { return castToFloatPtr(); }
	operator const char *() const { return castToCharPtr(); }

	bool isPtr() const;
	std::string toString(int32_t max_len = 64) const;
	attr_value_t fromString(attr_type_t type) const;

	bool test(const hash32_t &key, int32_t len = 0) const;
	void preserve();
	const hash32_t &key() const noexcept { return m_key; }
	const attr_type_t &type() const noexcept { return m_type; }
	const attr_value_t &value() const noexcept { return m_value; }

	static const char *typeName(attr_type_t t_type);
	static bool isFatalCast(attr_type_t from, attr_type_t to);

private:
	hash32_t m_key = "(undef)";
	attr_type_t m_type = e_undef;
	attr_value_t m_value = {0};
	void castCheck(attr_type_t s_type, attr_type_t t_type) const;

	float castToFloat() const;
	uint32_t castToUint() const;
	int32_t castToInt() const;
	bool castToBool() const;
	hash32_t castToHash() const;
	vec4f_t castToVec4f() const;
	vec4i_t castToVec4i() const;
	const float *castToFloatPtr() const;
	const int32_t *castToIntPtr() const;
	const uint32_t *castToUintPtr() const;
	const char *castToCharPtr() const;

	SPU_SERIALIZER_FRIENDS
};

class Attrs : public std::vector<Attr> {
public:
	using base_t = std::vector<Attrs>;

	Attrs() = default;
	Attrs(const char **argv, const std::source_location &location = std::source_location::current());
	Attrs(std::initializer_list<Attr> list,
	      const std::source_location &location = std::source_location::current());

	Attrs(const Attrs &attrs);
	Attrs(Attrs &&attrs) noexcept;
	~Attrs();

	Attrs &operator=(const Attrs &attrs);

	template<class T> optional_t<T> getf(const hash32_t &key, const T def = T()) const
	{
		try {
			auto attr = Attr(key, def);
			auto is_found = false;

			for (auto &this_attr: *this) {
				if (this_attr.test(key)) {
					attr = this_attr;
					is_found = true;
				}
			}

			addToLog(attr);
			return {is_found, T(attr)};
		}
		catch (std::invalid_argument &e) {
			report("attrs");
			aux_error(true, "invalid argument [%s]\n", e.what());
		}
	}
	template<class T> T get(const hash32_t &key, const T def) const { return getf<T>(key, def).value; }
	template<class T> void apply(const hash32_t &key, T &value) const { value = get<T>(key, value); }

	template<class T> T pick(const hash32_t &key, const T def)
	{
		T ret = get(key, def);
		erase(key);
		return ret;
	}
	template<class T> T pick(const hash32_t &key, int32_t index, const T def)
	{
		T ret = get(key, index, def);
		erase(key);
		return ret;
	}
	template<class T> bool replace(const hash32_t &key, const T &value)
	{
		return replace(Attr(key, value));
	}
	template<class T> Attrs &append(const hash32_t &key, const T &value)
	{
		return append({Attr(key, value)});
	}
	template<class T> Attrs &prepend(const hash32_t &key, const T &value)
	{
		return prepend({Attr(key, value)});
	}

	bool replace(const Attr &attr);
	bool peek(const hash32_t &name, const char *message = nullptr) const;
	bool peek(const std::vector<hash32_t> &names, const char *message = nullptr) const;
	bool subpeek(const std::vector<const char *> &prefixes, const char *message = nullptr) const;

	Attrs select(const char *prefix, bool is_keep_prefix = false) const;
	Attrs unselect(const std::vector<const char *> &prefixes) const;

	Attrs rewind(const hash32_t &ewind_key = "rewind") const;
	Attrs uniq() const;

	void report(const char *str) const;
	void trace(const char *str, bool is_report_at_dispose = true) const;
	void traceReport(const char *filter, uint32_t max_count) const;
	bool traceCheck(uint32_t max_count) const;

	void dispose();
	void preserve();
	std::string signature() const;

	Attrs &erase(const hash32_t &key);
	Attrs &prepend(const Attrs &attrs);
	Attrs &append(const Attrs &attrs);

	const Attrs &operator+=(const Attrs &attrs)
	{
		append(attrs);
		return *this;
	}

	friend Attrs operator+(const Attrs &a0, const Attrs &a1)
	{
		auto result = a0;
		result.append(a1);
		return result;
	}

	bool testAndLog(const Attr &attr, const Attr &test_attr, int32_t len = 0) const;
	void addToLog(const Attr &attr) const;
	void load(File &file, const char *tag);

	const std::string &subkey() const { return m_subkey; }
	void setSubkey(const std::string &subkey) { m_subkey = subkey; }

	static void clearAllTraces();
	static const std::vector<std::string> inspect(File &file);

private:
	class Trace;  // pimpl
	mutable Trace *m_trace = nullptr;
	mutable std::string m_subkey;
	mutable std::vector<std::string> m_locationStrings;

	void addLocation(const std::source_location &location);
	void addLocationString(const std::string &str);
	void shareTrace(const Attrs &attrs);
	SPU_SERIALIZER_FRIENDS
};

template<> size_t serialize(uint8_t *heap, bool is_dry, const Attr &object);
template<> size_t deserialize(const uint8_t *heap, Attr &object);

template<> size_t serialize(uint8_t *heap, bool is_dry, const Attrs &objects);
template<> size_t deserialize(const uint8_t *heap, Attrs &objects);

template<> const char *Attrs::get(const hash32_t &key, const char *def) const;
template<> double Attrs::get(const hash32_t &key, double def) const = delete;
template<> optional_t<double> Attrs::getf(const hash32_t &key, const double def) const = delete;

}  // namespace spu
