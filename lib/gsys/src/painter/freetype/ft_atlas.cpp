//
// FTAtlas :
//
#include "ft_atlas.h"

namespace spu::gs_painter::freetype {

void FTAtlas::upload()
{
	auto iformat = m_size.z == 1 ? GL_R8 : m_size.z == 3 ? GL_RGB8 : GL_RGBA8;
	auto width = 0, height = 0;

	if (m_texture.id()) {
		m_texture.get("width", &width);
		m_texture.get("height", &height);
	}
	if (m_size.x != width || m_size.y != height) {
		m_texture.dispose();
		Attrs attrs = {
		        {"target",     GL_TEXTURE_2D   },
                        {"iformat",    iformat         },
                        {"wrap_s",     GL_CLAMP_TO_EDGE},
		        {"wrap_t",     GL_CLAMP_TO_EDGE},
                        {"mag_filter", GL_LINEAR       },
                        {"min_filter", GL_LINEAR       },
		        {"width",      m_size.x        },
                        {"height",     m_size.y        },
		};
		m_texture.init(attrs);
	}
	m_texture.send(m_data.data(), iformat);
}

void FTAtlas::init(const Vec4i &size)
{
	assert(size.x > 0);
	assert(size.y > 0);
	assert((size.z == 1) || (size.z == 3) || (size.z == 4));

	m_used = 0;
	m_size = size;
	m_isModified = true;
	m_nodes = {Vec3f(1, 1, float(size.x) - 2)};
	m_data.resize(size.x * size.y * size.z);
	upload();
}

void FTAtlas::setRegion(const Recti &region, const uint8_t *data, const uint32_t stride)
{
	const auto x = uint32_t(region.ox);
	const auto y = uint32_t(region.oy);
	const auto width = uint32_t(region.sx);
	const auto height = uint32_t(region.sy);

	assert(x > 0);
	assert(y > 0);
	assert(x < uint32_t(m_size.x - 1));
	assert((x + width) <= uint32_t(m_size.x - 1));
	assert(y < uint32_t(m_size.y - 1));
	assert((y + height) <= uint32_t(m_size.y - 1));

	assert(height == 0 || (data != NULL && width > 0));

	auto depth = m_size.z;
	auto charsize = sizeof(char);
	for (auto i = 0u; i < height; ++i) {
		memcpy(m_data.data() + ((y + i) * m_size.x + x) * charsize * depth,
		       data + (i * stride) * charsize, width * charsize * depth);
	}
	m_isModified = true;
}

int FTAtlas::fit(const uint32_t index, const uint32_t width, const uint32_t height)
{
	auto &node = m_nodes.at(index);
	auto x = node.x;
	auto y = node.y;
	auto width_left = int32_t(width);
	auto i = index;

	if ((x + width) > (m_size.x - 1)) {
		return -1;
	}
	y = node.y;
	while (width_left > 0) {
		auto &node = m_nodes.at(i);
		if (node.y > y) {
			y = node.y;
		}
		if ((y + height) > (m_size.y - 1)) {
			return -1;
		}
		width_left -= node.z;
		++i;
	}
	return y;
}

void FTAtlas::merge()
{
	auto it = std::begin(m_nodes);
	while (it != std::end(m_nodes) - 1) {
		auto curr = it;
		auto next = it + 1;
		if (curr->y == next->y) {
			curr->z += next->z;
			it = m_nodes.erase(next) - 1;
		}
		else {
			++it;
		}
	}
}

Recti FTAtlas::getRegion(const uint32_t width, const uint32_t height)
{
	auto region = Recti(0, 0, int32_t(width), int32_t(height));
	auto best_height = UINT_MAX;
	auto best_index = -1;
	auto best_width = UINT_MAX;

	for (auto i = 0u; i < m_nodes.size(); ++i) {
		auto y = fit(i, width, height);
		if (y >= 0) {
			auto &node = m_nodes.at(i);
			if (((y + height) < best_height)
			    || (((y + height) == best_height)
			        && (node.z > 0 && (uint32_t)node.z < best_width))) {
				best_height = y + height;
				best_index = i;
				best_width = node.z;
				region.ox = node.x;
				region.oy = y;
			}
		}
	}

	if (best_index == -1) {
		region.ox = -1;
		region.oy = -1;
		region.sx = 0;
		region.sy = 0;
		return region;
	}

	Vec3f node;
	node.x = region.ox;
	node.y = region.oy + height;
	node.z = width;

	m_nodes.insert(std::begin(m_nodes) + best_index, node);

	auto it = std::begin(m_nodes) + best_index + 1;
	while (it != std::end(m_nodes)) {
		auto curr = it;
		auto prev = it - 1;

		if (curr->x < (prev->x + prev->z)) {
			int shrink = prev->x + prev->z - curr->x;
			curr->x += shrink;
			curr->z -= shrink;
			if (curr->z <= 0) {
				it = m_nodes.erase(curr);
			}
			else {
				break;
			}
		}
		else {
			break;
		}
	}
	merge();
	m_used += width * height;
	m_isModified = true;
	return region;
}

void FTAtlas::clear()
{
	Vec3f node = {1, 1, 1};

	m_used = 0;
	node.z = m_size.x - 2;

	m_nodes.clear();
	m_nodes.push_back(node);
	std::fill(begin(m_data), end(m_data), 0);
}

void FTAtlas::enlargeTexture(uint32_t width, uint32_t height)
{
	assert(width >= uint32_t(m_size.x));
	assert(height >= uint32_t(m_size.y));
	assert(width + height > uint32_t(m_size.x + m_size.y));

	auto width_old = m_size.x;
	auto height_old = m_size.y;

	// allocate new buffer
	auto data_old = m_data;
	m_data.resize(width * height * m_size.z);

	// update atlas size
	m_size.x = width;
	m_size.y = height;

	// add node reflecting the gained space on the right
	if (width > uint32_t(width_old)) {
		Vec3f node;
		node.x = width_old - 1;
		node.y = 1;
		node.z = width - width_old;
		m_nodes.push_back(node);
	}
	// copy over data from the old buffer, skipping first row and column because of the margin
	auto pixel_size = sizeof(char) * m_size.z;
	auto old_row_size = width_old * pixel_size;
	setRegion(
	        Recti(1, 1, width_old - 2, height_old - 2), data_old.data() + old_row_size + pixel_size,
	        old_row_size);
}
}  // namespace spu::gs_painter::freetype
