//
//$<<Header>>$
//

#pragma once

#include <images/image.hpp>

namespace spu::oglplus::images {

inline Image LoadTexture(const std::string& name, bool y_is_up = true, bool x_is_right = true)
{
	const auto dir = std::filesystem::path("assets/textures");
	const auto path = dir / (name + ".png");
	const auto full_path = File::searchPath(path);

	std::string flip_str;

	if (!x_is_right) {
		flip_str += "x";
	}
	if (!y_is_up) {
		flip_str += "y";
	}

	Attrs attrs = {
	        {"flip", flip_str},
	};

	uint32_t format = 0;
	uint32_t width = 0;
	uint32_t height = 0;
	const void* pix = nullptr;

	uint32_t inventory_id = spu_inventory_new("image", full_path.string().c_str(), attrs);
	spu_inventory_sync(inventory_id, 0);  // sync
	// spu_inventory_get(inventory_id, "format", &format);
	spu_inventory_get(inventory_id, "iformat", &format);
	spu_inventory_get(inventory_id, "width", &width);
	spu_inventory_get(inventory_id, "height", &height);
	spu_inventory_get(inventory_id, "data", &pix);

	// support RGBA PNG only
	assert(format == GL_RGBA8);
	return Image(width, height, 1, 4, static_cast<const uint8_t*>(pix), format, format);
}
}  // namespace spu::oglplus::images
