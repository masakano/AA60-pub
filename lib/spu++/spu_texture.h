//
// TextureBody :
//
#pragma once

#include "spu_object.h"
#include <spu/spu.h>  // SpuPad

namespace spu {

namespace spu_object {

struct TextureBody : public SpuBody {
public:
	TextureBody(uint32_t handle, bool is_owner);
	TextureBody(const Attrs &attrs);
	~TextureBody();

	bool occupy_encoder = false;
	bool occupy_decoder = false;

	static void default_receiver_callback(void *arg);
	static void default_sender_callback(void *arg);

private:
	struct DefaultAck {
		uint32_t size;
		SpuPad pad;
	};
	inline static SpuPad *ms_pad;
	inline static uint32_t ms_swapCount = 0;

};
}  // namespace spu_object

class SpuTexture : public spu_object::SpuObject<spu_object::TextureBody> {
public:
	static constexpr hash32_t e_texture = "texture";
	static constexpr hash32_t e_image = "image";
	static constexpr uint32_t c_bitrate = 32 * 1000 * 1000;
	static constexpr uint32_t c_target_fps = 60;

	using base_t = spu_object::SpuObject<spu_object::TextureBody>;

	SpuTexture() = default;

	explicit SpuTexture(uint32_t handle, bool is_owner = true) { reset(handle, is_owner); }
	explicit SpuTexture(const Attrs &attrs) { init(attrs); }
	explicit SpuTexture(const char *path, const Attrs &attrs = Attrs()) { init(path, attrs); }
	using spu_object::SpuObject<spu_object::TextureBody>::set;

	void init(const Attrs &attrs) override
	{
		auto init_attrs = attrs;
		auto *path = init_attrs.pick<const char *>("path", nullptr);
		if (path && *path) {
			init(path, init_attrs);
		}
		else {
			base_t::init(init_attrs);
		}
	}
	void init(const char *path, const Attrs &attrs = Attrs())
	{
		auto inventory_attrs = attrs;
		auto *restarget = inventory_attrs.pick("restarget", "texture");
		auto handle = spu_inventory_new(restarget, path, inventory_attrs);
		if (get_body() == nullptr || get_body()->handle != handle) {
			reset(handle);  // for additional init
		}
	}
	bool reuse(const char *signature) 
	{ 
		uint32_t id;
		if ((id = spu_inventory_reuse(e_texture, signature)) != ~0u) {
			reset(id);
			return true;
		}
		if ((id = spu_inventory_reuse(e_image, signature)) != ~0u) {
			reset(id);
			return true;
		}
		return false;
	}
	void append(const char *signature) const
	{ 
		spu_inventory_append(id(), signature); 
	}
	void send(
	        const void *pix, uint32_t format, const int32_t loc[4] = nullptr,
	        const uint32_t size[4] = nullptr, bool is_clear = false, uint32_t pformat = 0,
	        uint32_t ptype = 0)
	{
		assert(restarget() == e_texture);
		spu_texture_send(id(), pix, format, loc, size, is_clear, pformat, ptype);
	}
	void recv(void *pix, uint32_t format, const int32_t loc[4] = nullptr, const uint32_t size[4] = nullptr)
	        const
	{
		assert(restarget() == e_texture);
		spu_texture_recv(id(), pix, format, loc, size);
	}
	void copy(
	        uint32_t src_texture_id, const int32_t dst_loc[4] = nullptr, const int32_t src_loc[4] = nullptr,
	        const uint32_t size[4] = nullptr)
	{
		assert(restarget() == e_texture);
		spu_texture_copy(id(), src_texture_id, dst_loc, src_loc, size);
	}
	void save(const char *path, const hash32_t &target, uint32_t level = 0) const
	{
		assert(restarget() == e_texture);
		spu_texture_save(id(), path, target, level);
	}
	void update()
	{
		assert(restarget() == e_texture);
		spu_texture_update(id());
	}
	int32_t sync(bool is_nonblock) { return spu_inventory_sync(id(), is_nonblock); }


	hash32_t restarget() const { return (id() >> 16) == GL_TEXTURE_HOST_IMAGE ? e_image : e_texture; }

	bool encode(const uint32_t *embed_value = nullptr);
	bool decode(uint32_t *embed_value = nullptr);
	void set(const Attrs &attrs) override;
	int32_t get(const hash32_t &key, void *value) const override;
	void report(const char *str) const override;
};
}  // namespace spu
