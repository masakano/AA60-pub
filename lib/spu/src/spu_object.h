//
// SpuMessage :
//
#pragma once

#include <spu/spu.h>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#endif

#define spu_message(msg_level, ...)                                            \
	if (g_message.level >= msg_level) {                                    \
		g_message.putline(msg_level, SpuMessage::name(), __VA_ARGS__); \
	}

#define F(func, ...)                                                                 \
	func(__VA_ARGS__);                                                           \
	if (!SpuMessage::isEnableDebugOutput()) {                                    \
		auto err = glGetError();                                             \
		aux_error(err, "%s: [%s: 0x%04x]\n", #func, opengl_const(err), err); \
	}                                                                            \
	if (g_message.level >= 4) {                                                  \
		g_message.putline(4, "ogl", "\t%s\n", #func);                        \
	}

#define MSG() SpuMessage libspu_message(SpuMessage::e_static_name, __func__);

#define SET()  \
	MSG(); \
	aux_error(!SpuMessage::isMainThread(), "called fromm different thread.\n");

#define GET_CHK(id, key) \
	SET();           \
	if (key == hash32_t()) return s_objects.isValid(id);

namespace spu::libspu {

class SpuMessage {
public:
	enum StaticNameTag {
		e_static_name,
	};

	SpuMessage(StaticNameTag, const char *name) : m_prevName(ms_name)
	{
		if (isMainThread()) ms_name = name;

		if (g_message.level >= 3) {
			SpuPad *pad = nullptr;
			spu_graphics_get("pad", &pad);
			if (pad) {
				g_message.putline(3, "spu", "%d: %s\n", pad->swap_count, ms_name);
			}
		}
	}
	SpuMessage(const char *) = delete;
	SpuMessage(const std::string &) = delete;

	~SpuMessage()
	{
		if (isMainThread()) ms_name = m_prevName;
	}

	static void enableDebugOutput()
	{
		auto is_enable = g_message.level < 3;
		if (ms_isEnableDebugOutput != is_enable) {
			if (is_enable) {
				auto callback = reinterpret_cast<GLDEBUGPROC>(gl_on_error);
				F(glDebugMessageCallback, callback, nullptr);
				F(glEnable, GL_DEBUG_OUTPUT);
			}
			else {
				F(glDisable, GL_DEBUG_OUTPUT);
			}
			ms_isEnableDebugOutput = is_enable;
		}
	}
	static const char *name() { return isMainThread() ? ms_name : "[background]"; }
	static bool isMainThread() { return ms_mainThreadId == std::this_thread::get_id(); }
	static bool isEnableDebugOutput() { return ms_isEnableDebugOutput; }
	static void setMainThreadId() { ms_mainThreadId = std::this_thread::get_id(); }

private:
	static void gl_on_error(
	        GLenum, GLenum, GLuint, GLenum severity, GLsizei, const GLchar *message, const void *)
	{
		const std::map<int32_t, int32_t> msg_level = {
		        {GL_DEBUG_SEVERITY_HIGH,         0},
		        {GL_DEBUG_SEVERITY_MEDIUM,       1},
		        {GL_DEBUG_SEVERITY_LOW,          1},
		        {GL_DEBUG_SEVERITY_NOTIFICATION, 2},
		};

		if (msg_level.at(severity) <= g_message.level) {
			g_message.putline(msg_level.at(severity), SpuMessage::name(), "%s\n", message);
			if (g_message.level > 0 && severity == GL_DEBUG_SEVERITY_HIGH) {
				aux_abort();
			}
		}
		glGetError();  // flush
	}

	inline static std::thread::id ms_mainThreadId;
	inline static const char *ms_name = "spu";
	inline static bool ms_isEnableDebugOutput = false;
	const char *m_prevName = "spu";
};

template<class T>
int32_t getvalue(const hash32_t &dst_name, void *dst_value, const hash32_t &src_name, const T &src_value)
{
	if (dst_name == src_name) {
		if (dst_value) {
			*static_cast<T *>(dst_value) = src_value;
		}
		return sizeof(T);
	}
	return 0;
}

class LocalPool {
public:
	uint32_t alloc()
	{
		m_count++;
		for (auto &used: m_used) {
			if (used == 0) {
				used = 1;
				return &used - &m_used[0];
			}
		}
		m_used.push_back(1);
		return m_used.size() - 1;
	}

	uint32_t count() const noexcept { return m_count; }

	void free(int32_t id)
	{
		m_count--;
		m_used[id] = 0;
	}

private:
	std::vector<uint32_t> m_used;
	int32_t m_count = 0;
};

union Handle {
	uint32_t ui;
	int32_t i;
	struct {
		uint16_t id;
		uint16_t target;
	};

	Handle(uint32_t ui = 0) noexcept : ui(ui) {}
	Handle(uint16_t id, uint16_t target) noexcept : id(id), target(target) {}
};

class Object {
public:
	Object() = default;
	Object(const Object &) = delete;
	Object &operator=(const Object &) = delete;
	virtual ~Object() = default;
	virtual Handle handle() const noexcept { return m_handle; }

protected:
	Handle m_handle = 0;
};

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

class ManagerObject {
public:
	ManagerObject(int32_t level) : m_level(level) { ms_managers().push_back(this); }

	ManagerObject(const ManagerObject &) = delete;
	ManagerObject &operator=(const ManagerObject &) = delete;
	virtual ~ManagerObject() = default;

	static void startupAll()
	{
		auto compar = [](const ManagerObject *a, const ManagerObject *b) {
			return a->m_level < b->m_level;
		};

		auto &managers = ms_managers();
		std::sort(managers.begin(), managers.end(), compar);
		for (auto &manager: managers) {
			spu_message(1, "startup: %d %s\n", manager->m_level, manager->name());
			manager->startup();
		}
		SpuMessage::enableDebugOutput();
	}

	static void shutdownAll()
	{
		auto &managers = ms_managers();
		std::reverse(managers.begin(), managers.end());
		for (auto &manager: managers) {
			manager->shutdown();
		}
	}

	static std::vector<std::vector<uint32_t>> getAlivesList()
	{
		std::vector<std::vector<uint32_t>> alives_list;
		for (auto &manager: ms_managers()) {
			alives_list.push_back(manager->getAlives());
		}
		return alives_list;
	}

	static void prune(const std::vector<std::vector<uint32_t>> &alives_list)
	{
		aux_message(1, "prune:\n");
		aux_message(1, "    %-12s %-8s %8s %s\n", "object", "handle", "", "index/links");
		aux_message(1, "    ---------------------------------------------\n");

		auto &managers = ms_managers();
		assert(alives_list.size() == managers.size());
		for (auto i = 0u; i < alives_list.size(); i++) {
			managers.at(i)->prune(alives_list[i]);
		}
	}

	static void reportAll()
	{
		aux_printf("    %-12s %-8s %8s %s\n", "object", "handle", "", "index/links");
		aux_printf("    ---------------------------------------------\n");
		for (auto &manager: ms_managers()) {
			manager->report();
		}
	}

	virtual const char *name() const = 0;
	virtual bool isValid(uint32_t handle) const = 0;
	virtual uint32_t handleToIndex(const Handle &handle) const = 0;

protected:
	virtual std::vector<uint32_t> getAlives() const = 0;
	virtual void startup() {}
	virtual void prune(const std::vector<uint32_t> &alives) = 0;
	virtual void shutdown() = 0;
	virtual void report() = 0;

private:
	int32_t m_level;
	static std::vector<ManagerObject *> &ms_managers()
	{
		static std::vector<ManagerObject *> v;
		return v;
	}
};

template<class object_t> class Manager : public ManagerObject {
public:
	Manager(const char *name, int32_t level) : ManagerObject(level), m_name(name) {}

	template<class derived_object_t = object_t> Handle add(const Attrs &attrs)
	{
		auto prefix_id = attrs.getf<uint32_t>((std::string(name()) + "_id").c_str());
		if (prefix_id.hit) {
			auto handle = Handle(prefix_id.value);
			auto index = handleToIndex(handle);
			if (index < m_objects.size() && m_objects[index]) {
				spu_message(
				        2, "%s: reuse: handle=0x%08x index=%ld\n", name(), handle.ui, index);
				return handle;
			}
		}

		auto *object = new derived_object_t(attrs);
		auto handle = object->handle();
		auto index = handleToIndex(handle);

		if (index >= m_objects.size()) {
			m_objects.resize(index + 1);
		}

		if (m_objects[index]) {
			spu_message(0, "%s: index(%d) already used (overwritten)\n", name(), index);
			delete m_objects[index];
		}
		m_objects[index] = object;
		return handle;
	}

	void remove(uint32_t handle, void (*dispose)(object_t *) = nullptr)
	{
		auto index = handleToIndex(handle);
		if (index < m_objects.size()) {
			if (dispose) {
				(*dispose)(m_objects[index]);
			}
			else {
				delete m_objects[index];
			}
			m_objects[index] = nullptr;
		}
	}

	const std::vector<object_t *> &objects() const { return m_objects; }

	virtual object_t *at(uint32_t handle)
	{
		auto index = handleToIndex(handle);
		if (index >= m_objects.size() || m_objects[index] == nullptr) {
			report();
			aux_printf("    ------------------------------\n");
			aux_printf("    %-12s %08x %8d\n\n", "*this", handle, index);
			aux_error(true, "invalid handle\n", handle);
		}
		return m_objects[index];
	}

	bool isValid(uint32_t handle) const override
	{
		auto index = handleToIndex(handle);
		return index < m_objects.size() && m_objects[index];
	}

	std::vector<uint32_t> getAlives() const override
	{
		std::vector<uint32_t> alives;
		for (auto &object: m_objects) {
			if (object) {
				auto handle = object->handle();
				alives.push_back(handle.ui);
			}
		}
		return alives;
	}

	void prune(const std::vector<uint32_t> &alives) override
	{
		for (auto &object: m_objects) {
			if (object) {
				auto index = &object - &m_objects[0];
				auto handle = object->handle();
				if (std::find(begin(alives), end(alives), handle.ui) == std::end(alives)) {
					aux_message(
					        1, "    %-12s %08x %8d %p\n", name(), handle.ui, index, object);
					delete object;
					object = nullptr;
				}
			}
		}
	}

	void shutdown() override
	{
		prune({});
		m_objects.clear();
	}

	void report() override
	{
		for (auto &object: m_objects) {
			if (object) {
				auto index = &object - &m_objects[0];
				auto handle = object->handle();
				aux_printf("    %-12s %08x %8d %p\n", name(), handle.ui, index, object);
			}
		}
	}

	const char *name() const override { return m_name; }
	uint32_t handleToIndex(const Handle &handle) const override { return handle.ui; }

private:
	std::vector<object_t *> m_objects;
	const char *m_name = nullptr;
};
}  // namespace spu::libspu
