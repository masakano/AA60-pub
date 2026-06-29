//
// TextureBody :
//
#include <spu++/spu_texture.h>
#include "spu/spu.h"

namespace spu::spu_object {

TextureBody::TextureBody(uint32_t handle, bool is_owner) : SpuBody(handle, is_owner) {}

TextureBody::TextureBody(const Attrs &attrs) : TextureBody(0, true)
{
	auto sender_attrs = attrs.select("sender.");
	auto receiver_attrs = attrs.select("receiver.");
	auto texture_attrs = attrs.unselect({"sender.", "receiver."});

	auto *restarget = texture_attrs.pick("restarget", "texture");
	auto *file_path = texture_attrs.pick("path", "");

	if (*file_path) {
		handle = spu_inventory_new(restarget, file_path, texture_attrs);
	}
	else {
		handle = spu_texture_new(texture_attrs);
	}

	if (!receiver_attrs.empty()) {
		auto *callback = receiver_attrs.get<void (*)(void *)>("callback", default_receiver_callback);
		Attrs def_receiver_attrs = {
		        {"callback",   callback},
		        {"texture_id", handle  },
		};
		aux_error(
		        !spu_video_dec_new(def_receiver_attrs + receiver_attrs),
		        "video decoder already used\n");
		occupy_decoder = true;
	}
	if (!sender_attrs.empty()) {
		auto *callback = sender_attrs.get<void (*)(void *)>("callback", default_sender_callback);
		auto width = attrs.get("width", 0);
		auto height = attrs.get("height", 0);

		Attrs def_sender_attrs = {
		        {"callback",   callback},
		        {"texture_id", handle  },
		        {"width",      width   },
		        {"height",     height  },
		};
		aux_error(!spu_video_enc_new(def_sender_attrs + sender_attrs), "video encoder already used\n");
		occupy_encoder = true;
	}
	spu_graphics_get("pad", &ms_pad);
}

TextureBody::~TextureBody()
{
	assert(use_count == 0);
	if (is_owner) {
		if (handle) {
			spu_inventory_delete(handle);
		}
		if (occupy_encoder) {
			spu_video_enc_delete();
		}
		if (occupy_decoder) {
			spu_video_dec_delete();
		}
	}
}

void TextureBody::default_receiver_callback(void *arg)
{
	auto *ack = static_cast<DefaultAck *>(arg);
	if (ms_swapCount != ms_pad->swap_count) {
		ack->size = sizeof(ack->pad);
		ack->pad = *ms_pad;
		ms_swapCount = ms_pad->swap_count;
	}
	else {
		ack->size = 0;
	}
}

void TextureBody::default_sender_callback(void *arg)
{
	auto *ack = static_cast<const DefaultAck *>(arg);
	if (ack->size == sizeof(ack->pad)) {
		*ms_pad = ack->pad;
	}
}
}  // namespace spu::spu_object

namespace spu {
void SpuTexture::set(const Attrs &attrs)
{
	if (restarget() == e_texture) {
		spu_texture_set(id(), attrs);
	}
}


int32_t SpuTexture::get(const hash32_t &key, void *value) const
{
	static constexpr hash32_t e_hash = "hash";
	static constexpr hash32_t e_signature = "signature";
	static constexpr hash32_t e_count = "count";

	int32_t ret = 0;
	if ((ret = spu_inventory_get(id(), key, value))) {
		return ret;
	}
	if (restarget() == e_texture && key != e_hash && key != e_signature && key != e_count) {
		return spu_texture_get(id(), key, value);
	}
	return 0;
}

void SpuTexture::report(const char *str) const
{
	if (str && *str) aux_printf("%s:\n", str);
	if (restarget() == e_texture) {
		spu_texture_report(id());
	}
}

bool SpuTexture::encode(const uint32_t *embed_value)
{
	auto *body = get_body();
	if (body && body->occupy_encoder) {
		return spu_video_enc(embed_value);
	}
	return false;
}
bool SpuTexture::decode(uint32_t *embed_value)
{
	auto *body = get_body();
	if (body && body->occupy_decoder) {
		return spu_video_dec(embed_value);
	}
	return false;
}

}  // namespace spu
