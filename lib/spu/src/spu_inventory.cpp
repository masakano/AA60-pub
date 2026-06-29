//
// Inventory :
//
#include "resource_image.h"
#include "resource_shader.h"
#include "resource_texture.h"
#include <ssys/ssys.h>
#include <cstdint>

using namespace spu::libspu::resource;

namespace spu {
namespace libspu {
namespace {

constexpr hash32_t e_shader = "shader";
constexpr hash32_t e_image = "image";
constexpr hash32_t e_texture = "texture";

hash32_t restarget(uint32_t inventory_id)
{
	switch (inventory_id >> 16) {
	case 0: return e_shader;
	case GL_TEXTURE_HOST_IMAGE: return e_image;
	default: return e_texture;
	}
}

struct Location {
	uint32_t id = 0;
	uint32_t hash = 0;
	int32_t use_count = 0;
	bool is_owner = 1;
	ResourceObject *resource_object = nullptr;
	std::string signature;

	Location() = default;
	Location(
	        uint32_t id, uint32_t hash, int32_t use_count, bool is_owner, ResourceObject *resource_object,
	        const std::string &signature)
	        : id(id), hash(hash), use_count(use_count), is_owner(is_owner),
	          resource_object(resource_object), signature(signature)
	{
	}
};

class Inventory {
public:
	Inventory(const hash32_t &name, bool dispose_at_sync)
	        : mc_name(name), mc_isDisposeAtSync(dispose_at_sync)
	{
	}

	virtual ~Inventory() = default;
	void dispose();
	uint32_t create(const char *path, const Attrs &attrs);
	uint32_t reuse(const char *signature);
	void append(uint32_t id, const char *signature);
	uint32_t get(Location *loc, const hash32_t &key, void *value) const;
	bool remove(Location *loc, bool is_force = false);
	void remove(uint32_t id, bool is_force = false);
	void remove_not(std::vector<uint32_t>::const_iterator sp, std::vector<uint32_t>::const_iterator ep);
	void report() const;

	int32_t syncAll();
	bool sync(uint32_t id, bool is_nonblock);

	const hash32_t &name() const { return mc_name; }
	const std::map<uint32_t, Location *> &hashmap() const { return m_hashmap; }

protected:
	const hash32_t mc_name;
	const bool mc_isDisposeAtSync;
	virtual ResourceObject *doCreate(const char *path, const Attrs &attrs) = 0;
	virtual void doDispose(uint32_t id) = 0;

private:
	std::map<uint32_t, Location *> m_hashmap;
	bool monitor(Location *loc, bool is_nonblock);
};

void Inventory::dispose()
{
	const auto c_sleep_usec = 16000;
	while (syncAll() != 0) {
		sleep_microsec(c_sleep_usec);
	}
	for (auto &pair: m_hashmap) {
		auto *loc = pair.second;
		if (loc->is_owner) {
			doDispose(loc->id);
		}
	}
	m_hashmap.clear();
}

uint32_t Inventory::create(const char *path, const Attrs &attrs)
{
	auto signature = std::string(path) + attrs.signature();
	auto hash = hash32_t(signature.c_str()).value();
#if 1
	uint32_t reuse_id = reuse(signature.c_str());
	if (reuse_id != ~0u) return reuse_id;

#else
	auto it = m_hashmap.find(hash);
	if (it != std::end(m_hashmap)) {
		auto *loc = it->second;
		loc->use_count++;
		spu_message(2, "reuse %s #%02x for '%s'\n", mc_name.c_str(), loc->id, path);
		return loc->id;
	}
#endif
	auto *resource_object = doCreate(path, attrs);
	if (!resource_object->isSuccess()) {
		auto is_abort = attrs.get("abort", 1);
		aux_error(is_abort, "'%s' : create failed\n", path);
		doDispose(resource_object->id());
		ResourceObject::dispose(resource_object);
		return 0;
	}
	resource_object->start();

	auto id = resource_object->id();
	m_hashmap[hash] = new Location(id, hash, 1, true, resource_object, signature);
	spu_message(1, "create %s #%02x for '%s'\n", mc_name.c_str(), id, signature.c_str());
	return id;
}

uint32_t Inventory::reuse(const char *signature)
{
	auto hash = hash32_t(signature).value();
	auto it = m_hashmap.find(hash);
	if (it != end(m_hashmap)) {
		auto *loc = it->second;
		loc->use_count++;
		spu_message(2, "reuse %s #%02x for '%s'\n", mc_name.c_str(), loc->id, signature);
		return loc->id;
	}
	return ~0u;
}

void Inventory::append(uint32_t id, const char *signature)
{
	auto hash = hash32_t(signature).value();
	auto it = m_hashmap.find(hash);

	aux_error(
		it != end(m_hashmap), "%s #%02x '%s' already exists\n", mc_name.c_str(), id,
		signature);

	auto *loc = new Location(id, hash, 1, false, nullptr, signature);
	m_hashmap[hash] = loc;
}

bool Inventory::remove(Location *loc, bool is_force)
{
	if (--loc->use_count == 0 || is_force) {
		if (loc->resource_object) {
			spu_message(
			        1, "remove %s #%02x '%s'", mc_name.c_str(), loc->id, loc->signature.c_str());
			ResourceObject::dispose(loc->resource_object);
			loc->resource_object = nullptr;
		}
		if (loc->is_owner) {
			doDispose(loc->id);
		}
		delete loc;
		return true;
	}
	return false;
}

void Inventory::remove(uint32_t id, bool is_force)
{
	for (auto it = begin(m_hashmap); it != end(m_hashmap); ++it) {
		auto *loc = it->second;
		if (loc->id == id) {
			if (remove(loc, is_force)) {
				m_hashmap.erase(it);
			}
			return;
		}
	}
}

void Inventory::remove_not(std::vector<uint32_t>::const_iterator sp, std::vector<uint32_t>::const_iterator ep)
{
	auto it = begin(m_hashmap);
	while (it != end(m_hashmap)) {
		auto *loc = it->second;
		if (std::find(sp, ep, loc->id) == ep) {  // not alive
			if (remove(loc, true)) {
				it = m_hashmap.erase(it);
				continue;
			}
		}
		++it;
	}
}

void Inventory::report() const
{
	for (auto &pair: m_hashmap) {
		auto *loc = pair.second;
		auto msg = string_printf(
		        "    %-12s %08x %8d %s", mc_name.c_str(), loc->id, loc->use_count,
		        loc->signature.c_str());
		aux_printf("%s\n", msg.c_str());
	}
}

uint32_t Inventory::get(Location *loc, const hash32_t &key, void *value) const
{
	auto *resource_object = loc->resource_object;
	if (resource_object) {
		return resource_object->get(key, value);
	}
	return 0;
}

int32_t Inventory::syncAll()
{
	auto run_count = 0;
	for (auto &pair: m_hashmap) {
		auto *loc = pair.second;
		if (monitor(loc, true)) {
			run_count++;
		}
	}
	return run_count;
}

bool Inventory::sync(uint32_t id, bool is_nonblock)
{
	for (auto &pair: m_hashmap) {
		auto *loc = pair.second;
		if (loc->id == id) {
			return monitor(loc, is_nonblock);
		}
	}
	return false;
}

bool Inventory::monitor(Location *loc, bool is_nonblock)
{
	const auto c_sleep_usec = 1000;
	const auto c_timeout_usec = 60000000;

	for (auto usec = 0; usec < c_timeout_usec; usec += c_sleep_usec) {
		if (loc->resource_object == nullptr) {
			return false;
		}
		if (!loc->resource_object->sync()) {
			if (mc_isDisposeAtSync) {
				ResourceObject::dispose(loc->resource_object);
				loc->resource_object = nullptr;
			}
			return false;
		}
		if (is_nonblock) {
			return true;
		}
		sleep_microsec(c_sleep_usec);
	}
	aux_error(true, "timeout\n");
}

class ImageInventory : public Inventory {
public:
	ImageInventory() : Inventory(e_image, false) {}
	ResourceObject *doCreate(const char *path, const Attrs &attrs) override
	{
		return new ResourceImage(path, attrs);
	}
	void doDispose(uint32_t /*id*/) override {}
};

class TextureInventory : public Inventory {
public:
	TextureInventory() : Inventory(e_texture, true) {}
	ResourceObject *doCreate(const char *path, const Attrs &attrs) override
	{
		// printf("doCreate: %s\n", path);
		return new ResourceTexture(path, attrs);
	}
	void doDispose(uint32_t id) override { spu_texture_delete(id); }
};

class ShaderInventory : public Inventory {
public:
	ShaderInventory() : Inventory(e_shader, true) {}

	ResourceObject *doCreate(const char *path, const Attrs &attrs) override
	{
		return new ResourceShader(path, attrs);
	}

	void doDispose(uint32_t id) override { spu_shader_delete(id); }
};

void vsync_callback(void *);  // forward decl

class InventoryManager : public Manager<Object> {
public:
	InventoryManager(const char *name, int32_t level) : Manager(name, level) {}

	bool isAlive() const
	{
		if (!m_isAlive) {
			spu_message(0, "already shutdown or not startup yet\n");
		}
		return m_isAlive;
	}

	void syncAll()
	{
		if (isAlive()) {
			for (auto &inventory: m_inventories) {
				inventory->syncAll();
			}
		}
	}

	Inventory *select(const hash32_t &target)
	{
		if (isAlive()) {
			for (auto &inventory: m_inventories) {
				if (inventory->name() == target) {
					return inventory;
				}
			}
			aux_error(true, "%s: unknown inventory\n", target.c_str());
		}
		return nullptr;
	}

	void startup() override
	{
		Attrs graphics_attrs = {
		        {"add_cb_vsync", vsync_callback},
		};
		assert(m_isAlive == false);
		m_isAlive = true;

		spu_graphics_set(graphics_attrs);
		m_inventories[0] = new ImageInventory();
		m_inventories[1] = new TextureInventory();
		m_inventories[2] = new ShaderInventory();
	}

	void shutdown() override
	{
		assert(m_isAlive == true);
		m_isAlive = false;

		for (auto &inventory: m_inventories) {
			inventory->dispose();
			delete inventory;
			inventory = nullptr;
		}
		Attrs graphics_attrs = {
		        {"del_cb_vsync", vsync_callback},
		};
		spu_graphics_set(graphics_attrs);
	}

	std::vector<uint32_t> getAlives() const override
	{
		std::vector<uint32_t> alives;
		if (isAlive()) {
			alives.push_back(m_inventories[0]->hashmap().size());
			alives.push_back(m_inventories[1]->hashmap().size());

			for (auto &pair: m_inventories[0]->hashmap()) {
				alives.push_back(pair.second->id);
			}
			for (auto &pair: m_inventories[1]->hashmap()) {
				alives.push_back(pair.second->id);
			}
			for (auto &pair: m_inventories[2]->hashmap()) {
				alives.push_back(pair.second->id);
			}
		}
		return alives;
	}

	void prune(const std::vector<uint32_t> &alives) override
	{
		if (isAlive() && alives.size() > 2) {
			auto image_size = alives.at(0);
			auto texture_size = alives.at(1);

			auto it0 = alives.begin() + 2;
			auto it1 = it0 + image_size;
			auto it2 = it1 + texture_size;
			auto it3 = alives.end();

			m_inventories[0]->remove_not(it0, it1);
			m_inventories[1]->remove_not(it1, it2);
			m_inventories[2]->remove_not(it2, it3);
		}
	}

	void report() override
	{
		if (!isAlive()) return;

		for (auto &inventory: m_inventories) {
			inventory->report();
		}
	}

private:
	Inventory *m_inventories[3] = {nullptr, nullptr, nullptr};
	bool m_isAlive = false;
};

InventoryManager s_inventories("inventory", 1);
void vsync_callback(void *) { s_inventories.syncAll(); }
}  // namespace
}  // namespace libspu

using namespace libspu;

uint32_t spu_inventory_new(const hash32_t &target, const char *path, const Attrs &attrs)
{
	MSG();
	auto inventory = s_inventories.select(target);
	aux_error(inventory == nullptr, "target (%s) not found\n", target.c_str());
	return inventory->create(path, attrs);
}

uint32_t spu_inventory_reuse(const hash32_t &target, const char *signature)
{
	MSG();
	aux_error(signature == nullptr, "no signature\n");
	auto inventory = s_inventories.select(target);
	return inventory->reuse(signature);
}


void spu_inventory_append(uint32_t inventory_id, const char *signature)
{
	MSG();
	aux_error(signature == nullptr, "no signature\n");
	auto inventory = s_inventories.select(restarget(inventory_id));
	inventory->append(inventory_id, signature);
}

void spu_inventory_delete(uint32_t inventory_id)
{
	MSG();
	if (inventory_id == 0) {
		spu_message(0, "deleteing null inventory_id\n");
	}

	auto inventory = s_inventories.select(restarget(inventory_id));
	if (inventory) {
		inventory->remove(inventory_id);
	}
}

int32_t spu_inventory_sync(uint32_t inventory_id, bool is_nonblock)
{
	MSG();
	auto inventory = s_inventories.select(restarget(inventory_id));
	return inventory ? inventory->sync(inventory_id, is_nonblock) : 0;
}

int32_t spu_inventory_get(uint32_t inventory_id, const hash32_t &key, void *value)
{
	MSG();
	aux_error(key == nullptr || value == nullptr, "invalid key=[%s] value=[%p]\n", key, value);
	aux_error(key == "digest", "'digest' deprecated\n");

	auto inventory = s_inventories.select(restarget(inventory_id));

	if (inventory) {
		for (auto &pair: inventory->hashmap()) {
			auto *loc = pair.second;
			if (loc->id == inventory_id) {
				static constexpr hash32_t e_hash = "hash";
				static constexpr hash32_t e_signature = "signature";
				static constexpr hash32_t e_count = "count";
				auto ret = 0;
				if ((ret = getvalue(key, value, e_hash, loc->hash))) {
					return ret;
				}
				if ((ret = getvalue(key, value, e_signature, loc->signature.c_str()))) {
					return ret;
				}
				if ((ret = getvalue(key, value, e_count, loc->use_count))) {
					return ret;
				}
				return inventory->get(loc, key, value);
			}
		}
	}
	return 0;
}

int32_t spu_inventory_sync_all(const hash32_t &target, bool is_nonblock)
{
	MSG();

	auto ret = 0;
	auto inventory = s_inventories.select(target);
	if (inventory) {
		const auto c_sleep_usec = 16000;
		while ((ret = inventory->syncAll()) != 0) {
			if (is_nonblock) break;
			sleep_microsec(c_sleep_usec);
		}
	}
	return ret;
}
}  // namespace spu
