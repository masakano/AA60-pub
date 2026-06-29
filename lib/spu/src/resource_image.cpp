//
// ResourceImage :
//
#include "resource_image.h"
#include "image_loader.h"

namespace spu::libspu::resource {

ResourceImage::ResourceImage(const char *path, const Attrs &attrs) : ResourceObject(path, attrs)
{
	m_attrs.peek("use_cache", "deprecated");
	m_attrs.peek("pformat", "use 'iformat' instead");
	m_attrs.peek("format", "use 'iformat' instead");
	m_attrs.emplace_back("cube_target", image::Loader::c_cubeTarget);
	m_attrs.preserve();

	auto image_id = m_attrs.get("texture_id", -1);
	m_id = image_id == -1 ? ++ms_id : image_id;
}

void ResourceImage::doBackground()
{
	m_loader.loadImage(m_path, m_attrs, true);  // full_convert
}

void ResourceImage::doEpilogue() {}

int32_t ResourceImage::get(const hash32_t &key, void *value) const
{
	auto ret = 0;

	if (isBgRun()) {
		return -1;
	}
	if (key == "format"_h32) {
		aux_message(0, "'format' deprecated. use 'iformat' instead\n");
		return ret;
	}
	if ((ret = getvalue(key, value, "iformat"_h32, m_loader.format()))) {
		return ret;
	}
	if ((ret = getvalue(key, value, "width"_h32, m_loader.width()))) {
		return ret;
	}
	if ((ret = getvalue(key, value, "height"_h32, m_loader.height()))) {
		return ret;
	}
	if ((ret = getvalue(key, value, "depth"_h32, m_loader.depth()))) {
		return ret;
	}
	if ((ret = getvalue(key, value, "data"_h32, m_loader.pixels()))) {
		return ret;
	}
	return ret;
}
}  // namespace spu::libspu::resource
