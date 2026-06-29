//
// ResourceTexture :
//
#include "resource_texture.h"
#include "image_loader.h"

namespace spu::libspu::resource {

ResourceTexture::ResourceTexture(const char *path, const Attrs &attrs) : ResourceObject(path, attrs)
{
	m_attrs.emplace_back("cube_target", image::Loader::c_cubeTarget);
	m_attrs.preserve();
	m_id = spu_texture_new(m_attrs);
}

void ResourceTexture::doBackground() { m_loader.loadImage(m_path, m_attrs, false); }

void ResourceTexture::doEpilogue()
{
	auto layer = m_attrs.get("layer", 0);
	auto channel = m_attrs.get("channel", -1);
	auto iformat = m_attrs.get("iformat", 0);
	auto pformat = m_loader.format();
	auto width = m_loader.width();
	auto height = m_loader.height();

	Attrs texture_attrs = {
	        {"width",  width },
	        {"height", height},
	};

	if (iformat == 0) {
		iformat = pformat;
		texture_attrs.emplace_back("iformat", iformat);
	}
	if (channel == -1 && (iformat == GL_RED || pformat == GL_R8)) {  // turn RED and R8 into gray scale
		Attrs swizzle_attrs = {
		        {"swizzle_r", GL_RED},
		        {"swizzle_g", GL_RED},
		        {"swizzle_b", GL_RED},
		        {"swizzle_a", GL_RED},
		};
		texture_attrs += swizzle_attrs;
	}

	int32_t loc[] = {0, 0, layer, 0};
	uint32_t size[] = {0, 0, 1, 0};

	auto org_width = 0;
	spu_texture_get(m_id, "width", &org_width);
	if (org_width == 0) {
		spu_texture_set(m_id, texture_attrs.uniq());
	}
	if (channel == -1) {
		spu_texture_send(m_id, m_loader.pixels(), pformat, loc, size);
	}
	else {
		aux_error(iformat != GL_RGBA8, "multi-channel for '%s' not supported\n", opengl_const(iformat));
		std::vector<uint32_t> rgba8(width * height, 0xffffffff);
		if (org_width != 0) {
			spu_texture_recv(m_id, rgba8.data(), GL_RGBA8, loc, size);
		}
		m_loader.embedR8(rgba8.data(), channel);
		spu_texture_send(m_id, rgba8.data(), GL_RGBA8, loc, size);
	}
}
}  // namespace spu::libspu::resource
