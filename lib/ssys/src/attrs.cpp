//
// Attrs :
//
#include <algorithm>
#include <condition_variable>
#include <ssys/attrs.h>
#include <cstring>
#include <memory>
#include <string>
#include <typeinfo>
#include "levenstein_distance.h"

namespace spu {

class Attrs::Trace {
public:
	static constexpr const char *c_delim = "|";

	struct Log {
		Attr::attr_type_t type;
		std::string str;
	};

	Trace *share()
	{
		m_useCount++;
		return this;
	}

	const std::string &name() const { return m_name; }
	const std::map<std::string, Log> &logs() { return m_logs; }
	void addToLog(const std::string subkey, const Attr &attr)
	{
		auto key = std::string(attr.key().c_str());
		if (!subkey.empty() && key.rfind(subkey, 0) != 0) {
			key = subkey + key;
		}
		m_logs[key] = {attr.type(), attr.toString()};
	}

	void report(const char *filter, uint32_t max_count)
	{
		aux_printf("defined:\n");
		report_defined();

		aux_printf("\ninput:\n");
		report_input();

		aux_printf("\napplied:\n");
		report_applied(filter, max_count);
	}

	bool check(uint32_t max_count)
	{
		for (const auto &attr: m_attrs) {
			auto key_str = attr.key().c_str();
			if (m_logs.find(key_str) == std::end(m_logs)) {
				aux_printf("unused option found:\n");
				report_input();
				aux_printf("candidate for '-%s':\n", key_str);
				report_applied(key_str, max_count);
				aux_printf("try: '-[key]?' for specific options. '-?' for all details\n");
				return false;
			}
		}
		return true;
	}

	static Trace *create(const std::string &key, const Attrs &attrs, bool is_report_at_dispose)
	{
		auto *trace = new Trace(key, attrs, is_report_at_dispose);
		ms_traces().push_back(trace);
		return trace;
	}

	static void dispose(const Attrs &attrs)
	{
		auto *trace = attrs.m_trace;
		attrs.m_trace = nullptr;
		if (trace && --trace->m_useCount <= 0) {
			auto it = vector_find(ms_traces(), trace);
			if (it == std::end(ms_traces())) {
				printf("double-free %p\n", (void *)trace);
				aux_message(0, "double-free tracer. (maybe your trace() is overrapped)\n");
			}
			else {
				ms_traces().erase(it);
				delete trace;
			}
		}
	}
	static void disposeAll()
	{
		for (auto &trace: ms_traces()) {
			aux_message(0, "cleanup trace: %p\n", trace);
			delete trace;
		}
		ms_traces().clear();
		ms_traces().shrink_to_fit();
	}

	Trace(const Trace &) = delete;
	Trace &operator=(const Trace &) = delete;

private:
	std::map<std::string, Log> m_logs;
	std::string m_name;
	Attrs m_attrs;
	int32_t m_useCount = 1;
	bool m_isReportAtDispose = true;

	explicit Trace(const std::string &name, const Attrs &attrs, bool is_report_at_dispose)
	        : m_name(name), m_attrs(attrs), m_isReportAtDispose(is_report_at_dispose)
	{
	}

	~Trace()
	{
		if (m_isReportAtDispose) {
			auto line = std::string(m_name.length() + 1, '-');
			aux_printf("%s\n", line.c_str());
			aux_printf("%s:\n", m_name.c_str());
			aux_printf("%s\n", line.c_str());
			report(nullptr, 0);
		}
	}

	void report_defined()
	{
		for (auto &location_string: m_attrs.m_locationStrings) {
			aux_printf("    %s\n", location_string.c_str());
		}
	}

	void report_input()
	{
		ColumnAligner input_aligner("    ", " : ");
		for (const auto &attr: m_attrs) {
			auto is_found = m_logs.find(attr.key().c_str()) != std::end(m_logs);
			auto mark = is_found ? std::string("+ ") : std::string("- ");

			std::vector<std::string> list = {
			        mark + attr.key().c_str(),
			        Attr::typeName(attr.type()),
			        attr.toString(),
			};
			input_aligner.puts(list);
		}
		for (auto &output: input_aligner.flush()) {
			aux_printf("%s", output.c_str());
		}
	}
	void report_applied(const char *filter, uint32_t max_count)
	{
		ColumnAligner output_aligner("    ", " : ");

		struct Line {
			int32_t distance;
			std::pair<std::string, Log> log;
		};
		std::vector<Line> lines;
		if (filter == nullptr) {
			for (auto &log: m_logs) {
				lines.emplace_back(0, log);
			}
			max_count = 0;
		}
		else if (filter[strlen(filter) - 1] == '?') {
			auto len = strlen(filter) - 1;
			for (auto &log: m_logs) {
				if (strncmp(filter, log.first.c_str(), len) == 0) {
					lines.emplace_back(0, log);
				}
			}
			max_count = 0;
		}
		else {
			LevensteinDistance ld;
			for (auto &log: m_logs) {
				auto distance = ld.compare(filter, log.first);
				lines.emplace_back(distance, log);
			}
			auto gt = [&](const Line &l0, const Line &l1) { return l0.distance < l1.distance; };
			vector_sort(lines, gt);
		}

		for (auto &line: lines) {
			auto count = &line - &lines[0];
			if (max_count > 0 && count >= max_count) {
				break;
			}
			auto &log = line.log;
			std::vector<std::string> list = {
			        log.first,
			        Attr::typeName(log.second.type),
			        log.second.str,
			};
			output_aligner.puts(list);
		}
		auto outputs = output_aligner.flush();
		for (auto &output: outputs) {
			aux_printf("%s", output.c_str());
		}
	}

	static std::vector<Trace *> &ms_traces()
	{
		static std::vector<Trace *> v;
		return v;
	}
};

//
// Attr
//
static_assert(std::is_trivially_copyable<Attr>::value, "not copyable");

const char *Attr::typeName(attr_type_t t_type)
{
	static constexpr const char *s_type_names[] = {
	        "undef",  "hash32_t",     "vec4f_t",    "vec4i_t",     "float",       "int",
	        "uint",   "const float*", "const int*", "const uint*", "const char*", "const char**",
	        "void**", "void*",        "callback",   "nullptr",
	};
	return s_type_names[t_type];
}

bool Attr::isFatalCast(attr_type_t from, attr_type_t to)
{
	static constexpr auto T = true;
	static constexpr auto F = false;

	static constexpr bool s_is_fatal_casts[e_attr_type_max][e_attr_type_max] = {

	        {T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T}, // undef
	        {T, F, T, T, T, T, T, T, T, T, T, T, T, T, T, T}, // hash32_t
	        {T, T, F, T, T, T, T, T, T, T, T, T, T, T, T, T}, // vec4f_t
	        {T, T, T, F, T, T, T, T, T, T, T, T, T, T, T, T}, // vec4i_t
	        {T, T, F, T, F, T, T, T, T, T, T, T, T, T, T, T}, // float
	        {T, T, T, F, T, F, F, T, T, T, T, T, T, T, T, T}, // int
	        {T, T, T, T, T, F, F, T, T, T, T, T, T, T, T, T}, // uint
	        {T, T, T, T, T, T, T, F, T, T, T, T, T, F, T, T}, // float*
	        {T, T, T, T, T, T, T, T, F, F, T, T, T, F, T, T}, // int*
	        {T, T, T, T, T, T, T, T, F, F, T, T, T, F, T, T}, // uint*
	        {T, T, T, T, T, T, T, T, T, T, F, T, T, F, T, T}, // char*
	        {T, T, T, T, T, T, T, T, T, T, T, F, T, T, T, T}, // char**
	        {T, T, T, T, T, T, T, T, T, T, T, T, F, T, T, T}, // void**
	        {T, T, T, T, T, T, T, F, F, F, F, T, T, F, T, T}, // void*
	        {T, T, T, T, T, T, T, F, F, F, F, T, T, T, F, T}, // callback_t
	        {T, T, T, T, T, T, T, F, F, F, F, F, F, F, F, F}, // nullptr
	};
	return s_is_fatal_casts[from][to];
}

Attr::attr_value_t Attr::fromString(attr_type_t type) const
{
	attr_value_t result = {0};
	const char *str = (const char *)m_value.cp;
	try {
		switch (type) {
		case e_hash: {
			result.h = hash32_t(str);
			return result;
		}
		case e_vec4f: {
			auto list = extract_from_string(str, ", ");
			for (auto i = 0; i < 4; i++) {
				result.fv.f[i] = std::stof(size_t(i) < list.size() ? list[i] : list.back());
			}
			return result;
		}
		case e_vec4i: {
			auto list = extract_from_string(str, ", ");
			for (auto i = 0; i < 4; i++) {
				result.iv.i[i] = std::stoi(
				        size_t(i) < list.size() ? list[i] : list.back(), nullptr, 0);
			}
			return result;
		}
		case e_int:
		case e_uint: {
			if (strcmp(str, "true") == 0) {
				result.si = 1;
			}
			else if (strcmp(str, "false") == 0) {
				result.si = 0;
			}
			else {
				result.si = std::stoi(str, nullptr, 0);
			}
			return result;
		}
		case e_float: {
			result.f = std::stof(str);
			return result;
		}
		default:
			auto errmsg = std::string("cast error to '") + typeName(type) + "' from "
			            + typeName(m_type) + ' ' + toString();
			throw std::invalid_argument(errmsg);
		}
	}
	catch (const std::exception &e) {
		aux_error(true, "invalid argument [%s]\n", e.what());
	}
}

void Attr::castCheck(attr_type_t s_type, attr_type_t t_type) const
{
	if (s_type != t_type && isFatalCast(s_type, t_type)) {
		const char *format
		        = "%s:\n"
		          "    from %s\n"
		          "    to   %s\n";

		auto errmsg = string_printf(
		        format, m_key.c_str(), (std::string(typeName(m_type)) + " " + toString()).c_str(),
		        typeName(t_type));

		aux_printf("%s", errmsg.c_str());
		throw std::invalid_argument("cast error");
	}
}

float Attr::castToFloat() const
{
	if (m_type == e_char_ptr) {
		return fromString(e_float).f;
	}
	castCheck(m_type, e_float);
	return m_value.f;
}

uint32_t Attr::castToUint() const
{
	if (m_type == e_char_ptr) {
		return fromString(e_int).ui;
	}
	castCheck(m_type, e_int);
	return m_value.ui;
}

int32_t Attr::castToInt() const
{
	if (m_type == e_char_ptr) {
		return fromString(e_int).si;
	}
	castCheck(m_type, e_int);
	return m_value.si;
}

bool Attr::castToBool() const
{
	if (m_type == e_char_ptr) {
		return fromString(e_int).si;
	}
	castCheck(m_type, e_int);
	return m_value.si != 0;
}

hash32_t Attr::castToHash() const
{
	if (m_type == e_char_ptr) {
		return fromString(e_hash).h;
	}
	castCheck(m_type, e_hash);
	return m_value.h;
}

vec4f_t Attr::castToVec4f() const
{
	if (m_type == e_char_ptr) {
		return fromString(e_vec4f).fv;
	}
	if (m_type == e_float) {
		const auto f = m_value.f;
		return {f, f, f, f};
	}
	castCheck(m_type, e_vec4f);
	return m_value.fv;
}

vec4i_t Attr::castToVec4i() const
{
	if (m_type == e_char_ptr) {
		return fromString(e_vec4i).iv;
	}
	if (m_type == e_int) {
		const auto i = int32_t(m_value.si);
		return {i, i, i, i};
	}
	castCheck(m_type, e_vec4i);
	return m_value.iv;
}

const float *Attr::castToFloatPtr() const
{
	if (m_type == e_vec4f) {
		return m_value.fv.f;
	}
	castCheck(m_type, e_float_ptr);
	return (const float *)m_value.cp;
}

const int32_t *Attr::castToIntPtr() const
{
	if (m_type == e_vec4i) {
		return m_value.iv.i;
	}
	castCheck(m_type, e_int_ptr);
	return (const int32_t *)m_value.cp;
}

const uint32_t *Attr::castToUintPtr() const
{
	if (m_type == e_vec4i) {
		return m_value.iv.ui;
	}
	castCheck(m_type, e_uint_ptr);
	return (const uint32_t *)m_value.cp;
}

const char *Attr::castToCharPtr() const
{
	if (isPtr()) {
		return (const char *)m_value.cp;
	}
	else {
		auto data_str = toString();
		return g_strheap.preserve(data_str.c_str());
	}
}

bool Attr::isPtr() const
{
	/* clang-format off */
	return (m_type == e_float_ptr		||
		m_type == e_int_ptr		||
		m_type == e_uint_ptr		||
		m_type == e_char_ptr		||
		m_type == e_char_ptr_ptr	||
		m_type == e_void_ptr_ptr	||
		m_type == e_void_ptr		||
		m_type == e_nullptr);
	/* clang-format on */
}

std::string Attr::toString(int32_t max_len) const
{
	switch (m_type) {
	case e_undef: {
		return string_printf("(undef)");
	}
	case e_hash: {
		hash32_t h = m_value.h;
		return h.c_str();
	}
	case e_vec4f: {
		vec4f_t v = m_value.fv;
		return string_printf("{%g, %g, %g, %g}", v.x, v.y, v.z, v.w);
	}
	case e_vec4i: {
		vec4i_t v = m_value.iv;
		return string_printf("{%d, %d, %d, %d}", v.x, v.y, v.z, v.w);
	}
	case e_int: {
		return string_printf("%d", m_value.si);
	}
	case e_uint: {
		return string_printf("0x%x", m_value.ui);
	}
	case e_float: {
		return string_printf("%g", m_value.f);
	}
	case e_float_ptr: {
		const auto *fp = (const float *)m_value.cp;
		return fp ? string_printf("%g,%g,%g,%g,...", fp[0], fp[1], fp[2], fp[3]) : "(nil)";
	}
	case e_int_ptr: {
		const auto *ip = (const int32_t *)m_value.cp;
		return ip ? string_printf("%d,%d,%d,%d,...", ip[0], ip[1], ip[2], ip[3]) : "(nil)";
	}
	case e_uint_ptr: {
		const auto *ip = (const int32_t *)m_value.cp;
		return ip ? string_printf("0x%x,0x%x,0x%x,0x%x,...", ip[0], ip[1], ip[2], ip[3]) : "(nil)";
	}
	case e_char_ptr: {
		const std::string c_quat = "\"";
		const char *cp = (const char *)m_value.cp;
		return cp ? c_quat + pretty_string(cp, max_len) + c_quat : "(nil)";
	}
	case e_char_ptr_ptr: {
		const std::string c_quat = "\"";
		const char *const *cp = (const char *const *)m_value.cp;
		return cp ? c_quat + pretty_string(*cp, max_len) + c_quat : "(nil)";
	}
	case e_void_ptr_ptr: {
		return string_printf("%p", m_value.cp);
	}
	case e_void_ptr: {
		return string_printf("%p", m_value.cp);
	}
	case e_callback: {
		return string_printf("%p", m_value.cb);
	}
	case e_nullptr: {
		return string_printf("(nil) or (special)");
	}
	default:
		assert(0);
		auto errmsg = std::string("cast error: ") + typeName(m_type) + ' ' + toString();
		throw std::invalid_argument(errmsg);
	}
}

void Attr::preserve()
{
	m_key.replace(g_strheap.preserve(m_key.c_str()));
	if (m_type == Attr::e_char_ptr) {
		m_value.cp = g_strheap.preserve((const char *)m_value.cp);
	}
}

template<> size_t serialize(uint8_t *heap, bool is_dry, const Attr &object)
{
	assert(!object.isPtr() || object.m_type == Attr::e_char_ptr);

	auto *hp = heap;
	auto len = strlen(object.m_key.c_str());
	if (!is_dry) {
		memcpy(hp, object.m_key.c_str(), len + 1);
	}
	hp += len + 1;
	hp += serialize(hp, is_dry, object.m_type);
	hp += serialize(hp, is_dry, object.m_value);

	if (object.m_type == Attr::e_char_ptr) {
		auto len = strlen((const char *)object.m_value.cp);
		if (!is_dry) {
			memcpy(hp, object.m_value.cp, len + 1);
		}
		hp += len + 1;
	}
	return hp - heap;
}

template<> size_t deserialize(const uint8_t *heap, Attr &object)
{
	const auto *hp = heap;

	object.m_key = reinterpret_cast<const char *>(hp);
	hp += strlen(object.m_key.c_str()) + 1;
	hp += deserialize(hp, object.m_type);
	hp += deserialize(hp, object.m_value);

	if (object.m_type == Attr::e_char_ptr) {
		const auto *str = reinterpret_cast<const char *>(hp);
		object.m_value.cp = str;
		hp += strlen(str) + 1;
	}
	return hp - heap;
}

//
// Attrs
//

Attrs::Attrs(const char **argv, const std::source_location &location)
{
	addLocation(location);

	while (*argv) {
		aux_error(argv[0][0] != '-', "bad key format '%s' (must be '-' prefix)\n", argv[0]);
		aux_error(argv[0][1] == '\0', "bad key format '%s' (must be '-' prefix)\n", argv[0]);
		auto key = argv[0] + 1;
		if (argv[1] != nullptr && *argv[1] != '-') {
			push_back(Attr(key, argv[1]));
			argv += 2;
		}
		else {
			push_back(Attr(key, "true"));
			argv += 1;
		}
	}
	preserve();
}

Attrs::Attrs(std::initializer_list<Attr> list, const std::source_location &location)
{
	addLocation(location);
	for (const auto &attr: list) {
		push_back(attr);
	}
}

Attrs::Attrs(Attrs &&attrs) noexcept
        : std::vector<Attr>(std::move(attrs)), m_trace(std::move(attrs.m_trace)),
          m_subkey(std::move(attrs.m_subkey)), m_locationStrings(std::move(attrs.m_locationStrings))
{
	attrs.m_trace = nullptr;  // don't forget
}

Attrs::~Attrs() { dispose(); }

Attrs::Attrs(const Attrs &attrs)
        : std::vector<Attr>(attrs), m_subkey(attrs.m_subkey), m_locationStrings(attrs.m_locationStrings)
{
	shareTrace(attrs);
}

Attrs &Attrs::operator=(const Attrs &attrs)
{
	if (this == &attrs) {
		return *this;
	}
	Trace::dispose(*this);
	// m_trace = nullptr;
	assign(std::begin(attrs), std::end(attrs));
	m_subkey = attrs.m_subkey;
	m_locationStrings.clear();
	shareTrace(attrs);
	return *this;
}

void Attrs::addToLog(const Attr &attr) const
{
	if (m_trace) {
		m_trace->addToLog(m_subkey, attr);
	}
}

bool Attrs::testAndLog(const Attr &attr, const Attr &test_attr, int32_t len) const
{
	if (len == 0) {
		if (attr.key() == test_attr.key()) {
			addToLog(attr);
			return true;
		}
		addToLog(test_attr);
	}
	else {
		if (strncmp(attr.key().c_str(), test_attr.key().c_str(), len) == 0) {
			addToLog(attr);
			return true;
		}
	}
	return false;
}

bool Attr::test(const hash32_t &key, int32_t len) const
{
	return len == 0 ? m_key == key : strncmp(m_key.c_str(), key.c_str(), len) == 0;
}

bool Attrs::replace(const Attr &attr)
{
	auto is_set = false;
	for (auto &this_attr: *this) {
		if (attr.key() == this_attr.key()) {
			is_set = true;
			this_attr = attr;
		}
	}
	return is_set;
}

bool Attrs::peek(const hash32_t &name, const char *message) const
{
	for (const auto &attr: *this) {
		if (attr.key() == name) {
			if (message) {
				report("rejected attrs");
				aux_error(true, "'%s': %s\n", attr.key().c_str(), message);
			}
			return true;
		}
	}
	return false;
}

bool Attrs::peek(const std::vector<hash32_t> &names, const char *message) const
{
	for (auto &name: names) {
		if (peek(name, message)) {
			return true;
		}
	}
	return false;
}

bool Attrs::subpeek(const std::vector<const char *> &prefixes, const char *message) const
{
	for (auto &prefix: prefixes) {
		auto len = strlen(prefix);
		for (const auto &attr: *this) {
			if (strncmp(attr.key().c_str(), prefix, len) == 0) {
				if (message) {
					report("rejected attrs");
					aux_error(true, "'%s': %s\n", attr.key().c_str(), message);
				}
				return true;
			}
		}
	}
	return false;
}

Attrs Attrs::unselect(const std::vector<const char *> &prefixes) const
{
	Attrs attrs;
	for (auto &prefix: prefixes) {
		auto len = strlen(prefix);
		for (const auto &attr: *this) {
			if (strncmp(attr.key().c_str(), prefix, len) != 0) {
				attrs.push_back(attr);
			}
		}
	}
	attrs.m_subkey = m_subkey;
	attrs.shareTrace(*this);
	return attrs.uniq();  // don' forget uniq()
}

Attrs Attrs::select(const char *prefix, bool is_keep_prefix) const
{
	Attrs attrs;
	auto is_found = false;
	auto len = strlen(prefix);
	for (const auto &attr: *this) {
		if (strncmp(attr.key().c_str(), prefix, len) == 0) {  // not test()
			is_found = true;
			if (is_keep_prefix) {
				attrs.push_back(attr);
			}
			else {
				attrs.push_back({attr.key().c_str() + len, attr});
			}
		}
	}
	if (!is_keep_prefix) {
		attrs.m_subkey = m_subkey + prefix;
	}
	if (!is_found) {
		// addToLog(Attr(prefix, std::string(prefix) + "*"));
		addToLog(Attr(prefix, nullptr));
	}
	attrs.shareTrace(*this);
	return attrs;
}

Attrs Attrs::rewind(const hash32_t &rewind_key) const
{
	Attrs dst;
	for (const auto &attr: *this) {
		if (attr.key() == rewind_key) {
			dst.clear();
		}
		else {
			dst.push_back(attr);
		}
	}
	dst.shareTrace(*this);
	dst.m_subkey = m_subkey;
	return dst;
}

Attrs Attrs::uniq() const
{
	Attrs attrs;

	auto sp = std::begin(*this);
	auto ep = std::end(*this);
	for (auto ip = sp; ip != ep; ++ip) {
		auto det = [&ip](const Attr &attr) { return ip->key() == attr.key(); };
		if (find_if(ip + 1, ep, det) == ep) {
			attrs.push_back(*ip);
		}
	}

	attrs.shareTrace(*this);
	attrs.m_subkey = m_subkey;
	return attrs;
}

void Attrs::report(const char *str) const
{
	ColumnAligner aligner("    ", " : ");
	for (const auto &attr: *this) {
		std::vector<std::string> list = {
		        std::string(attr.key().c_str()),
		        Attr::typeName(attr.type()),
		        attr.toString(),
		};
		aligner.puts(list);
	}
	if (m_trace) {
		aux_printf("\n%s: (trace=%p '%s')\n", str, m_trace, m_trace->name().c_str());
	}
	else {
		aux_printf("\n%s:\n", str);
	}
	for (auto &output: aligner.flush()) {
		aux_printf("%s", output.c_str());
	}
	if (!m_locationStrings.empty()) {
		aux_printf("defined from:\n");
		for (auto &location_string: m_locationStrings) {
			aux_printf("    %s\n", location_string.c_str());
		}
	}
}

void Attrs::trace(const char *str, bool is_report_at_dispose) const
{
	Trace::dispose(*this);
	// m_trace = nullptr;
	m_subkey = "";  // clear subkey
	if (str) {
		m_trace = Trace::create(str, *this, is_report_at_dispose);
	}
}

Attrs &Attrs::erase(const hash32_t &key)
{
	auto op = [&key](const Attr &a) { return key == a.key(); };
	vector_remove_if(static_cast<std::vector<Attr> &>(*this), op);
	return *this;
}

Attrs &Attrs::prepend(const Attrs &attrs)
{
	insert(std::begin(*this), std::begin(attrs), std::end(attrs));
	if (m_trace == nullptr && attrs.m_trace) {
		m_subkey = attrs.m_subkey;
	}
	shareTrace(attrs);
	return *this;
}

Attrs &Attrs::append(const Attrs &attrs)
{
	insert(std::end(*this), std::begin(attrs), std::end(attrs));
	if (m_trace == nullptr && attrs.m_trace) {
		m_subkey = attrs.m_subkey;
	}
	shareTrace(attrs);
	return *this;
}

void Attrs::addLocationString(const std::string &str)
{
	for (auto &location_string: m_locationStrings) {
		if (str == location_string) {
			return;  // found
		}
	}
	m_locationStrings.push_back(str);
}

void Attrs::addLocation(const std::source_location &location)
{
	addLocationString(string_printf("'%s':%d", location.file_name(), location.line()));
}

void Attrs::shareTrace(const Attrs &attrs)
{
	for (auto &location_string: attrs.m_locationStrings) {
		addLocationString(location_string);
	}
	// m_locations.insert(m_locations.end(), attrs.m_locations.begin(), attrs.m_locations.end());

	if (m_trace == attrs.m_trace) {
		// do nothing
	}
	else if (m_trace == nullptr && attrs.m_trace) {
		m_trace = attrs.m_trace->share();
	}
	else if (m_trace && attrs.m_trace == nullptr) {
		attrs.m_trace = m_trace->share();
	}
	else if (m_trace && attrs.m_trace) {
		if (m_trace->name() != attrs.m_trace->name()) {
			aux_message(
			        0, "trace conflict %s:%s (skip shareing)\n", m_trace->name().c_str(),
			        attrs.m_trace->name().c_str());
		}
	}
}

void Attrs::dispose()
{
	Trace::dispose(*this);
	// m_trace = nullptr;
	m_subkey.shrink_to_fit();
}

void Attrs::preserve()
{
	for (auto &attr: *this) {
		attr.preserve();
	}
}

std::string Attrs::signature() const
{
	std::string str;
	for (const auto &attr: uniq()) {
		auto value_str = attr.toString();
		if (!value_str.empty()) {
			str += std::string(":") + attr.key().c_str() + "=" + value_str;
		}
	}
	return str;
}

void Attrs::clearAllTraces() { Trace::disposeAll(); }

void Attrs::load(File &file, const char *tag)
{
	std::vector<std::string> list;

	auto is_hit = false;
	auto is_hit_sticky = false;
	auto tags = extract_from_string(tag, ":");

	file.rewind();
	while (file.getlist(list, " \t", "", true)) {  // strip quote
		if (list[0].empty()) {
			/* do nothing */
		}
		else if (list[0][0] != '-') {
			is_hit = vector_is_find(tags, list[0]);
			is_hit_sticky |= is_hit;
		}
		else if (is_hit) {
			if (list.size() == 1) {
				aux_printf("%s.%s: (warning) null value\n", tag, list[0].c_str());
				list.emplace_back(" ");
			}
			aux_error(list[0][0] != '-', "%s: bad key format\n", list[0].c_str());

			auto key = list[0].substr(1);
			auto value = list[1];

			for (auto i = 2u; i < list.size(); i++) {
				value += " " + list[i];
			}
			push_back(Attr(g_strheap.preserve(key.c_str()), g_strheap.preserve(value.c_str())));
		}
	}
	if (!is_hit_sticky) {
		aux_printf("%s: tag not found in conf file\n", tag);
	}
}

const std::vector<std::string> Attrs::inspect(File &file)
{
	std::vector<std::string> tags;
	std::vector<std::string> list;

	file.rewind();
	while (file.getlist(list, " \t", "", true)) {
		if (!list[0].empty() && list[0][0] != '-') {
			tags.push_back(list[0]);
		}
	}
	return tags;
}

void Attrs::traceReport(const char *filter, uint32_t max_count) const
{
	if (m_trace) {
		m_trace->report(filter, max_count);
	}
}
bool Attrs::traceCheck(uint32_t max_count) const
{
	if (m_trace) {
		return m_trace->check(max_count);
	}
	return true;
}

template<> size_t serialize(uint8_t *heap, bool is_dry, const Attrs &object)
{
	auto *hp = heap;

	Attrs clean_attrs;
	for (const auto &attr: object) {
		if (!attr.isPtr() || attr.m_type == Attr::e_char_ptr) {
			clean_attrs.push_back(attr);
		}
	}

	size_t size = clean_attrs.size();
	hp += serialize(hp, is_dry, size);
	for (auto &attr: clean_attrs) {
		hp += serialize(hp, is_dry, attr);
	}
	hp += serialize(hp, is_dry, object.m_subkey);
	return hp - heap;
}

template<> size_t deserialize(const uint8_t *heap, Attrs &object)
{
	assert(object.m_trace == nullptr);
	const auto *hp = heap;
	size_t size = 0;
	hp += deserialize(hp, size);

	object.resize(size);
	for (auto &attr: object) {
		hp += deserialize(hp, attr);
	}
	hp += deserialize(hp, object.m_subkey);
	return hp - heap;
}

// experimental
template<> const char *Attrs::get(const hash32_t &key, const char *def) const
{
	if (def && def[0] == '?') {
		auto list = extract_from_string(def + 1, ":");
		auto result = g_strheap.preserve(getf<const char *>(key, list[0].c_str()).value);

		if (!vector_is_find(list, result)) {
			report("attrs");
			aux_error(true, "'%s' : '%s' not in candidate [%s]\n", key.c_str(), result, def);
		}
		return result;
	}
	else {
		return getf<const char *>(key, def).value;
	}
}
}  // namespace spu
