//
// Manager :
//
#include "spu_texture_object.h"
#include "spu_texture_null.h"
#include "spu_texture_renderbuffer.h"
#include "spu_texture_sampler.h"
#include "spu_texture_texbuffer.h"
#include "spu_texture_trad.h"

namespace spu {
namespace libspu::spu_texture {

class TextureManager : public Manager<TextureObject> {
public:
	enum Category {
		e_trad = 0,
		e_sampler,
		e_renderbuffer,
		e_texbuffer,
	};

	explicit TextureManager(const char *name) : Manager<TextureObject>(name, 0) {}

	void startup() override
	{
		Attrs null_texture_attrs = {
		        {"texture_id", 0x00000000},
		};
		Attrs null_sampler_attrs = {
		        {"texture_id", 0xffff0000},
		};
		Manager::add<TextureNull>(null_texture_attrs);
		Manager::add<TextureNull>(null_sampler_attrs);
	}

	void shutdown() override { F(glBindSampler, 0, 0); }

	uint32_t handleToIndex(const Handle &handle) const override
	{
		auto category = getCategory(handle.target);
		aux_error(
		        handle.id == 0 && (category != e_trad && category != e_sampler),
		        "bad null texture target (%s)\n", opengl_const(handle.target));
		return handle.id * 8 + category;
	}

	Handle add(const Attrs &attrs)  // not virutal
	{
		auto target = attrs.get("target", GL_TEXTURE_2D);

		switch (getCategory(target)) {
		case e_trad: return Manager::add<TextureTrad>(attrs);
		case e_sampler: return Manager::add<TextureSampler>(attrs);
		case e_renderbuffer: return Manager::add<TextureRenderbuffer>(attrs);
		case e_texbuffer: return Manager::add<TextureTexbuffer>(attrs);
		default: aux_error(true, "invalid target %s:%08x\n", opengl_const(target), target);
		}
	}

private:
	static Category getCategory(uint32_t target)
	{
		switch (target) {
		case 0:
		case GL_TEXTURE_1D:
		case GL_TEXTURE_1D_ARRAY:
		case GL_TEXTURE_RECTANGLE:
		case GL_TEXTURE_2D:
		case GL_TEXTURE_2D_ARRAY:
		case GL_TEXTURE_3D:
		case GL_TEXTURE_2D_MULTISAMPLE:
		case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
		case GL_TEXTURE_CUBE_MAP:
		case GL_TEXTURE_CUBE_MAP_ARRAY: return e_trad;
		case GL_TEXTURE_SAMPLER: return e_sampler;
		case GL_RENDERBUFFER: return e_renderbuffer;
		case GL_TEXTURE_BUFFER: return e_texbuffer;
		default: aux_error(true, "%s: supported target\n", opengl_const(target));
		}
	}
};

TextureManager s_objects("texture");

}  // namespace libspu::spu_texture

using namespace libspu;
using namespace libspu::spu_texture;

uint32_t spu_texture_new(const Attrs &attrs)
{
	SET();
	return s_objects.add(attrs).ui;
}

void spu_texture_delete(uint32_t texture_id)
{
	SET();
	if (texture_id) s_objects.remove(texture_id);
}

void spu_texture_copy(
        uint32_t dst_texture_id, uint32_t src_texture_id, const int32_t dst_loc[4], const int32_t src_loc[4],
        const uint32_t size[4])
{
	SET();
	s_objects.at(dst_texture_id)->copy(s_objects.at(src_texture_id), dst_loc, src_loc, size);
}

void spu_texture_send(
        uint32_t texture_id, const void *pix, uint32_t pformat, const int32_t loc[4], const uint32_t size[4],
        bool is_clear, uint32_t raw_pformat, uint32_t raw_ptype)
{
	SET();
	s_objects.at(texture_id)->send(pix, pformat, loc, nullptr, size, is_clear, raw_pformat, raw_ptype);
}

void spu_texture_recv(
        uint32_t texture_id, void *pix, uint32_t pformat, const int32_t loc[4], const uint32_t size[4],
        uint32_t raw_pformat, uint32_t raw_ptype)
{
	SET();
	s_objects.at(texture_id)->recv(pix, pformat, nullptr, loc, size, raw_pformat, raw_ptype);
}

void spu_texture_report(uint32_t texture_id)
{
	SET();
	s_objects.at(texture_id)->report();
}

bool spu_texture_use(uint32_t texture_id, int32_t slot)
{
	SET();
	return s_objects.at(texture_id)->use(slot);
}

void spu_texture_update(uint32_t texture_id)
{
	SET();
	s_objects.at(texture_id)->update();
}

void spu_texture_set(uint32_t texture_id, const Attrs &attrs)
{
	SET();
	s_objects.at(texture_id)->set(attrs);
}

int32_t spu_texture_get(uint32_t texture_id, const hash32_t &key, void *value)
{
	GET_CHK(texture_id, key);
	if (key == "signature") {
		spu_message(0, "cannot use 'signature' key. (use 'spu_inventory_get()' instead)\n");
	}
	return s_objects.at(texture_id)->get(key, value);
}

//
// move to spu_texture_trad since this works on traditional texture only
//
uint32_t spu_texture_alias(
        uint32_t texture_id, uint32_t alias_target, uint32_t alias_format, uint32_t depth, uint32_t offset)
{
	SET();

	auto *texture = s_objects.at(texture_id);
	auto alias_id = 0u;

	if (depth == 0) {
		depth = texture->m_depth - offset;
	}

	if (alias_format == 0) {
		alias_format = texture->m_iformat;
	}

	Attrs attrs = {
	        {"target",  alias_target},
	        {"iformat", alias_format},
	        {"alias",   1           },
	};
	alias_id = s_objects.add(attrs).ui;

	F(glTextureView, alias_id & 0xffff, alias_target, texture_id & 0xffff, alias_format,
	  texture->m_baseLevel, texture->m_maxLevel + 1, offset, depth);

	auto *alias_texture = s_objects.at(alias_id);
	alias_texture->set(texture);  // implies get
	return alias_id;
}
}  // namespace spu
