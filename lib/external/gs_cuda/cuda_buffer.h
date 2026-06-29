//
//$<<Header>>$
//
#pragma once
#include <spu/spu.h>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>

#define cuda_ck(call)                                                                      \
	{                                                                                  \
		auto e = (call);                                                           \
		aux_error(e, "cuda error %d:%s\n", e, cudaGetErrorString(cudaError_t(e))); \
	}

namespace spu::gs_cuda {

class CudaScopedDevice {
public:
	CudaScopedDevice();
	CudaScopedDevice(int32_t device);
	~CudaScopedDevice();

	static void setDevice(int32_t device);
	static int32_t getDevice();
	static int32_t deviceCount();

private:
	int32_t m_prevDevice = 0;
};

class CudaBufferBody {
public:
	void init(
	        const std::string &name, uint32_t alloc_count, uint32_t byte_per_pixel,
	        const std::vector<cudaStream_t> &streams);

	void initArray(
	        const std::string &name, uint32_t width, uint32_t height, uint32_t depth, uint32_t format,
	        bool is_cubemap);
	void sendEach(uint32_t gpuid, const void *hostptr, uint32_t first, uint32_t count, cudaStream_t stream);
	void send(
	        const void *hostptr, uint32_t first, uint32_t count, const std::vector<cudaStream_t> &streams);
	void recvEach(uint32_t gpuid, void *hostptr, uint32_t first, uint32_t count, cudaStream_t stream);
	void copy(const CudaBufferBody &src, const std::vector<cudaStream_t> &streams);

	uint32_t byteSize() const { return m_byteSize; }
	uint32_t byteStride() const { return m_byteStride; }
	uint32_t size() const { return m_byteSize / m_byteStride; }

	const std::vector<void *> &datas() const { return m_datas; }
	const std::vector<cudaArray_t> &arrays() const { return m_arrays; }
	const std::vector<cudaTextureObject_t> &cudaTextureObjects() const { return m_cudaTextureObjects; }

	void share() { m_useCount++; }

	static void dispose(CudaBufferBody *body);
	static CudaBufferBody *create() { return new CudaBufferBody(); }
	static void shutdown();
	static void report();

private:
	std::string m_name;
	std::vector<void *> m_datas;
	std::vector<cudaArray_t> m_arrays;
	std::vector<cudaTextureObject_t> m_cudaTextureObjects;

	uint32_t m_format = 0;
	uint32_t m_pitch = 0;
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	uint32_t m_depth = 0;
	uint32_t m_byteStride = 0;
	uint32_t m_byteSize = 0;
	uint32_t m_useCount = 1;

	CudaBufferBody();
	~CudaBufferBody();
	static std::unordered_set<CudaBufferBody *> &ms_shelf()
	{
		static std::unordered_set<CudaBufferBody *> v;
		return v;
	}

	void bindToCudaTexture(bool is_cubemap);
};

class CudaBufferView;
template<class T> class CudaBuffer {
public:
	CudaBuffer() = default;

	CudaBuffer(const CudaBuffer &buffer)
	{
		auto prev_body = m_body;
		m_body = buffer.m_body;
		if (m_body) m_body->share();
		CudaBufferBody::dispose(prev_body);  // dispose later
	}

	CudaBuffer(CudaBuffer &&buffer)
	{
		CudaBufferBody::dispose(m_body);
		m_body = buffer.m_body;
		buffer.m_body = nullptr;
	}

	virtual ~CudaBuffer() { CudaBufferBody::dispose(m_body); }

	CudaBuffer &operator=(const CudaBuffer &buffer)
	{
		auto prev_body = m_body;
		m_body = buffer.m_body;
		if (m_body) m_body->share();
		CudaBufferBody::dispose(prev_body);
		return *this;
	}

	void dispose()
	{
		CudaBufferBody::dispose(m_body);
		m_body = nullptr;
	}

	void init(const std::string &name, uint32_t alloc_nelem, const std::vector<cudaStream_t> &streams = {})
	{
		create();
		m_body->init(name, alloc_nelem, sizeof(T), streams);
	}

	void initArray(
	        const std::string &name, uint32_t width, uint32_t height, uint32_t depth, uint32_t format,
	        bool is_cubemap)
	{
		create();
		m_body->initArray(name, width, height, depth, format, is_cubemap);
	}

	void sendEach(
	        uint32_t gpuid, const void *hostptr, uint32_t first, uint32_t count, cudaStream_t stream = 0)
	{
		if (m_body) m_body->sendEach(gpuid, hostptr, first, count, stream);
	}

	void recvEach(uint32_t gpuid, void *hostptr, uint32_t first, uint32_t count, cudaStream_t stream = 0)
	{
		if (m_body) m_body->recvEach(gpuid, hostptr, first, count, stream);
	}

	void send(
	        const void *hostptr, uint32_t first, uint32_t count,
	        const std::vector<cudaStream_t> &streams = {})
	{
		if (m_body) m_body->send(hostptr, first, count, streams);
	}

	void copy(const CudaBuffer &src, const std::vector<cudaStream_t> &streams = {})
	{
		if (m_body && src.m_body) m_body->copy(*src.m_body, streams);
	}

	cudaTextureObject_t cudaTextureObject(uint32_t gpuid) const
	{
		return m_body ? m_body->cudaTextureObjects().at(gpuid) : 0;
	}

	T *data(uint32_t gpuid = 0) const
	{
		return m_body ? static_cast<T *>(m_body->datas().at(gpuid)) : nullptr;
	}
	bool empty() const { return !m_body; }
	cudaArray_t array(uint32_t gpuid = 0) const { return m_body ? m_body->arrays().at(gpuid) : nullptr; }
	uint32_t byteSize() const { return m_body ? m_body->byteSize() : 0; }
	uint32_t byteStride() const { return m_body ? m_body->byteStride() : 0; }
	uint32_t size() const { return m_body ? m_body->size() : 0; }

	void init(
	        const std::string &name, const void *hostptr, uint32_t alloc_nelem,
	        const std::vector<cudaStream_t> &streams = {})
	{
		init(name, alloc_nelem, streams);
		send(hostptr, 0, alloc_nelem, streams);
	}

	void initArray(
	        const std::string &name, const void *hostptr, uint32_t width, uint32_t height, uint32_t depth,
	        uint32_t format, bool is_cubemap, const std::vector<cudaStream_t> &streams = {})
	{
		initArray(name, width, height, depth, format, is_cubemap);
		send(hostptr, 0, 0, streams);
	}

	template<class T2>
	void init(
	        const std::string &name, const std::vector<T2> &vec,
	        const std::vector<cudaStream_t> &streams = {})
	{
		static_assert(sizeof(T) == sizeof(T2), "byte_per_pixel mismatch");
		init(name, vec.data(), vec.size(), streams);
	}

	template<class T2>
	void sendEach(uint32_t gpuid, const std::vector<T2> &vec, const std::vector<cudaStream_t> &streams = {})
	{
		static_assert(sizeof(T) == sizeof(T2), "byte_per_pixel mismatch");
		sendEach(gpuid, vec.data(), vec.size(), streams);
	}

	template<class T2> void send(const std::vector<T2> &vec, const std::vector<cudaStream_t> &streams = {})
	{
		static_assert(sizeof(T) == sizeof(T2), "byte_per_pixel mismatch");
		send(vec.data(), 0, vec.size(), streams);
	}
	static void shutdown() { CudaBufferBody::shutdown(); }

private:
	friend class CudaBufferView;
	CudaBufferBody *m_body = nullptr;  // never release
	void create()
	{
		if (m_body == nullptr) m_body = CudaBufferBody::create();
	}
};

template<class T> class CudaGraphicsResource {
public:
	CudaGraphicsResource() = default;

	CudaGraphicsResource(CudaGraphicsResource &&src)
	{
		*this = src;
		src.m_res = nullptr;
		src.m_cudaptr = nullptr;
	}

	~CudaGraphicsResource()
	{
		unmap();
		if (m_res) {
			cuda_ck(cudaGraphicsUnregisterResource(m_res));
		}
	}

	void initFromTexture(uint32_t texture_id, uint32_t mode)
	{
		if ((m_textureId = texture_id)) {
			spu_texture_get(m_textureId, "buffer_id", &m_bufferId);
			spu_texture_get(m_textureId, "size", &m_byteSize);
			cuda_ck(cudaGraphicsGLRegisterBuffer(&m_res, m_bufferId, mode));
		}
	}

	void initFromBuffer(uint32_t buffer_id, uint32_t nelem, uint32_t mode)
	{
		m_bufferId = buffer_id;
		m_byteSize = nelem * sizeof(T);
		cuda_ck(cudaGraphicsGLRegisterBuffer(&m_res, m_bufferId, mode));
	}

	void map(cudaStream_t stream)
	{
		if (m_res && !m_cudaptr) {
			size_t size;
			m_stream = stream;
			cuda_ck(cudaGraphicsMapResources(1, &m_res, m_stream));
			cuda_ck(cudaGraphicsResourceGetMappedPointer(&m_cudaptr, &size, m_res));
		}
	}

	void unmap()
	{
		if (m_res && m_cudaptr && m_stream) {
			cuda_ck(cudaGraphicsUnmapResources(1, &m_res, m_stream));
			m_cudaptr = nullptr;
			m_stream = 0;
		}
	}

	T *cudaptr() const { return static_cast<T *>(m_cudaptr); }
	uint32_t textureId() const { return m_textureId; }
	uint32_t bufferId() const { return m_bufferId; }
	uint32_t byteSize() const { return m_byteSize; }

private:
	cudaGraphicsResource_t m_res = nullptr;
	cudaStream_t m_stream = 0;
	uint32_t m_byteSize = 0;
	uint32_t m_textureId = 0;
	uint32_t m_bufferId = 0;
	void *m_cudaptr = nullptr;

	CudaGraphicsResource(const CudaGraphicsResource &) = default;
	CudaGraphicsResource &operator=(const CudaGraphicsResource &) = default;
};

class CudaBufferView {
public:
	CudaBufferView() = default;

	template<class T>
	CudaBufferView(const CudaBuffer<T> &buffer, uint32_t byte_offset = 0, uint32_t gpuid = 0)
	{
		auto *body = buffer.m_body;
		if (body) {
			m_data = body->datas().at(gpuid);
			m_byteStride = body->byteStride();
			m_byteSize = body->byteSize();
			if (byte_offset) {
				m_data = (uint8_t *)m_data + byte_offset;
			}
		}
	}

	void *data() const { return m_data; }
	uint32_t byteSize() const { return m_byteSize; }
	uint32_t byteStride() const { return m_byteStride; }
	uint32_t size() const { return m_byteSize / m_byteStride; }

private:
	void *m_data = nullptr;
	uint32_t m_byteStride = 0;
	uint32_t m_byteSize = 0;
};
}  // namespace spu::gs_cuda
