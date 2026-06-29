//
//$<<Header>>$
//
#pragma once
#include <gsys/canvas/copy.h>

namespace spu::gs_cuda {

class Viewer {
public:
	void init(const char* path, uint32_t width, uint32_t height)
	{
		m_width = width;
		m_height = height;

		Attrs texture_attrs = {
		        {"target",  GL_TEXTURE_BUFFER                  },
		        {"iformat", GL_RGBA32F                         },
		        {"size",    m_width * m_height * sizeof(float4)},
		};
		m_outputbuf.init(texture_attrs);
		m_graphicsResource.initFromTexture(m_outputbuf.id(), cudaGraphicsMapFlagsWriteDiscard);

		auto viewport0 = Rectf(0, 0, m_width, m_height);
		Attrs canvas_attrs = {
		        {"viewport0",  viewport0},
		        {"path",       path     },
		        {"def_width",  m_width  },
		        {"def_height", m_height },
		};
		m_canvas.init(canvas_attrs);
		m_canvas.u_color0 = m_outputbuf.id();
	}

	void view(CudaBuffer<float4>& accum_buffer, cudaStream_t stream)
	{
		m_graphicsResource.map(stream);
		cuda_ck(cudaMemcpyAsync(
		        m_graphicsResource.cudaptr(), accum_buffer.data(), m_width * m_height * sizeof(float4),
		        cudaMemcpyDefault, stream));
		m_graphicsResource.unmap();
		m_canvas.render();
	}

private:
	int32_t m_width = 0;
	int32_t m_height = 0;
	GsCanvas m_canvas;
	CudaGraphicsResource<float4> m_graphicsResource;
	spu::SpuTexture m_outputbuf;
};
}  // namespace spu::gs_cuda
