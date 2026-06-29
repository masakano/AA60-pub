//
//
//
#pragma once
#include "cuda_buffer.h"

namespace spu::gs_cuda {
class ReadRGBA8 {
public:
	void read(const char* path)
	{
		Attrs attrs = {
		        {"iformat", GL_RGBA8},
		};
		m_inventoryId = spu_inventory_new("image", path, attrs);
		spu_inventory_sync(m_inventoryId, 0);
		spu_inventory_get(m_inventoryId, "width", &m_width);
		spu_inventory_get(m_inventoryId, "height", &m_height);
		spu_inventory_get(m_inventoryId, "iformat", &m_format);
		spu_inventory_get(m_inventoryId, "data", &m_pix);
		assert(m_format == GL_RGBA8);
		m_name = path;
	}

	void send(CudaBuffer<uint32_t>& buffer, cudaStream_t stream)
	{
		buffer.initArray(m_name, m_pix, m_width, m_height, 1, GL_RGBA8, false, {stream});
	}

	~ReadRGBA8()
	{
		if (m_inventoryId) {
			spu_inventory_delete(m_inventoryId);
		}
	}

private:
	std::string m_name;
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	uint32_t m_format = 0;
	uint8_t* m_pix = nullptr;
	uint32_t m_inventoryId = 0;
};
}  // namespace spu::gs_cuda
