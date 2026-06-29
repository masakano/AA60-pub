//
// GsObject :
//
#pragma once

#include <spu++/spu++.h>
#include <ssys/cpu_prof.h>
#include <cstdint>

namespace spu {

class GsNode;
class GsObject {
public:
	static constexpr int32_t e_class_depth = 0;
	static constexpr hash32_t e_render = "render";
	static constexpr hash32_t e_debug_render = "debug_render";
	static constexpr hash32_t e_lazy = "lazy";
	static constexpr hash32_t e_alive = "alive";

	template<class T> struct ClassCreator {
		ClassCreator() { startups().push_back({T::startup, T::shutdown, T::e_class_depth}); }
	};

	GsObject(const GsObject &) = delete;
	GsObject &operator=(const GsObject &) = delete;

	virtual ~GsObject();
	virtual void dispose();
	virtual void init(const Attrs &attrs);
	virtual void update() { m_lastUpdateCount = getSeconds().count(); }
	virtual uint32_t lastUpdateCount() const { return m_lastUpdateCount; }
	virtual void report(const char *) const;

	virtual void syncCallback(const std::function<void(void *)> &callback, void *arg = nullptr);
	virtual void startInspector() {}
	virtual void stopInspector()
	{
		delete m_inspector;
		m_inspector = nullptr;
	}

	virtual void set(const Attrs &attrs);
	virtual void setName(const std::string &name);
	virtual std::string typeName() const;
	virtual std::string prettyName() const;

	virtual const std::string name() const { return m_name; }
	virtual hash32_t hash() const { return m_hash; }

	virtual void setRelatedNodes(const std::vector<GsNode *> &nodes) { m_relatedNodes = nodes; }
	virtual const std::vector<GsNode *> &relatedNodes() const { return m_relatedNodes; }

	virtual void setProperty(const hash32_t &key, int32_t value) { m_properties[key] = value; }
	virtual int32_t getProperty(const hash32_t &key) const { return m_properties.at(key); }

	virtual bool sync(bool is_nonblock) final;  // use 'doSync()' to override

	static std::vector<GsObject *> &aliveObjects() { return ms_state->objects; }
	static Attrs &getAttrs() { return ms_state->attrs; }
	static CpuProf &getCpuProf() { return staticState().prof; }
	static Seconds &getSeconds() { return staticState().seconds; }

	static SpuTexture &defaultWhiteTexture() { return ms_state->white_texture; }
	static SpuTexture &defaultBlackTexture() { return ms_state->black_texture; }
	static SpuTexture &defaultLightTexture() { return ms_state->light_texture; }
	static SpuTexture &defaultBrdfTexture() { return ms_state->brdf_texture; }

	static bool isDefaultTexture(uint32_t texture_id);
	static void startup(const Attrs &attrs);
	static void pruneObjects(const std::vector<GsObject *> &alives);
	static void shutdown();

	template<class T> void set(const hash32_t &key, const T &value) { set({Attr(key, value)}); }

protected:
	GsObject *m_inspector = nullptr;
	GsObject(const char *name);

	virtual bool doSync([[maybe_unused]] bool is_nonblock) { return false; }
	virtual void doAddInspector() {}

private:
	struct Startup {
		void (*startup)(const Attrs &);
		void (*shutdown)();
		int32_t level;
	};
	struct StaticState {
		std::vector<Startup> startups;
		CpuProf prof;
		Seconds seconds;
	};
	struct State {
		std::vector<GsObject *> objects;
		Attrs attrs;
		SpuTexture white_texture;
		SpuTexture black_texture;
		SpuTexture light_texture;
		SpuTexture brdf_texture;
	};
	struct Callback {
		std::function<void(void *)> func;
		void *arg;
	};

	hash32_t m_hash;
	std::string m_name;
	std::vector<GsNode *> m_relatedNodes;
	std::map<hash32_t, int32_t> m_properties;
	std::vector<Callback> m_callbacks;
	bool m_isInCallback = false;
	uint32_t m_lastUpdateCount = 0;

	static StaticState &staticState()
	{
		static StaticState s;
		return s;
	}
	static std::vector<Startup> &startups() { return staticState().startups; }
	inline static State *ms_state = nullptr;
};
}  // namespace spu
