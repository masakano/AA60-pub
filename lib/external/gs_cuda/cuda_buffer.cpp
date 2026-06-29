//
// @file cuda_buffer.cpp
//
#include "cuda_buffer.h"
#include <cuda.h>  // CU_DEVICE_ATTRIBUTE_TEXTURE_ALIGNMENT

#ifdef MAINTENANCE
#define REPORT() report()
#else
#define REPORT()
#endif

#define ROUND_UP(x, n) (((x) + ((n) - 1)) & (~((n) - 1)))
#define PIXEL_BYTE (16)

namespace spu::gs_cuda {
namespace {
int32_t get_gpu_alignment()
{
	int32_t array[] = {0};
	cuDeviceGetAttribute(array, CU_DEVICE_ATTRIBUTE_TEXTURE_ALIGNMENT, 0);
	int32_t value = array[0];
	return value;
}

uint32_t get_byte_per_pixel(uint32_t format)
{
	switch (format) {
	case GL_R8: return 1;
	case GL_RGB8: return 3;
	case GL_RGBA8: return 4;
	case GL_SRGB8_ALPHA8: return 4;
	case GL_R32F: return 4;
	case GL_RG32F: return 8;
	case GL_RGB32F: return 12;
	case GL_RGBA32F: return 16;
	default: assert(0);
	}
}

cudaChannelFormatDesc get_cuda_channel_format_desc(uint32_t format)
{
	switch (format) {
	case GL_R8: return cudaCreateChannelDesc(8, 0, 0, 0, cudaChannelFormatKindUnsigned);
	case GL_RGB8: return cudaCreateChannelDesc(8, 8, 8, 0, cudaChannelFormatKindUnsigned);
	case GL_RGBA8: return cudaCreateChannelDesc(8, 8, 8, 8, cudaChannelFormatKindUnsigned);
	case GL_SRGB8_ALPHA8: return cudaCreateChannelDesc(8, 8, 8, 8, cudaChannelFormatKindUnsigned);
	case GL_R32F: return cudaCreateChannelDesc(32, 0, 0, 0, cudaChannelFormatKindFloat);
	case GL_RG32F: return cudaCreateChannelDesc(32, 32, 0, 0, cudaChannelFormatKindFloat);
	case GL_RGB32F: return cudaCreateChannelDesc(32, 32, 32, 0, cudaChannelFormatKindFloat);
	case GL_RGBA32F: return cudaCreateChannelDesc(32, 32, 32, 32, cudaChannelFormatKindFloat);
	default: assert(0);
	}
}

cudaTextureReadMode get_cuda_texture_read_mode(uint32_t format)
{
	switch (format) {
	case GL_R8:
	case GL_RGB8:
	case GL_RGBA8:
	case GL_SRGB8_ALPHA8: return cudaReadModeNormalizedFloat;
	case GL_R32F:
	case GL_RG32F:
	case GL_RGB32F:
	case GL_RGBA32F: return cudaReadModeElementType;
	default: assert(0);
	}
}

}  // namespace

//
// ScopedDevice
//
void CudaScopedDevice::setDevice(int32_t device) { cuda_ck(cudaSetDevice(device)); }

int32_t CudaScopedDevice::getDevice()
{
	int32_t device = 0;
	cuda_ck(cudaGetDevice(&device));
	return device;
}

int32_t CudaScopedDevice::deviceCount()
{
	int32_t count = 1;
	cuda_ck(cudaGetDeviceCount(&count));
	return count;
}

CudaScopedDevice::CudaScopedDevice() { m_prevDevice = getDevice(); }

CudaScopedDevice::CudaScopedDevice(int32_t device)
{
	m_prevDevice = getDevice();
	setDevice(device);
}

CudaScopedDevice::~CudaScopedDevice() { setDevice(m_prevDevice); }

//
// CudaBufferBody
//
CudaBufferBody::CudaBufferBody()
{
	int32_t gpu_count = CudaScopedDevice::deviceCount();
	m_cudaTextureObjects.resize(gpu_count, 0);
	m_datas.resize(gpu_count, nullptr);
	m_arrays.resize(gpu_count, nullptr);

	ms_shelf().insert(this);
}

CudaBufferBody::~CudaBufferBody()
{
	ms_shelf().erase(this);

	CudaScopedDevice dev;  // auto push-pop
	for (auto i = 0u; i < m_datas.size(); i++) {
		CudaScopedDevice::setDevice(i);
		if (m_datas[i]) {
			cuda_ck(cudaFree(m_datas[i]));
		}
		if (m_arrays[i]) {
			cuda_ck(cudaFreeArray(m_arrays[i]));
		}
		if (m_cudaTextureObjects[i]) {
			cuda_ck(cudaDestroyTextureObject(m_cudaTextureObjects[i]));
		}
	}
}

void CudaBufferBody::dispose(CudaBufferBody *body)
{
	if (body && --body->m_useCount <= 0) {
		delete body;
	}
}

void CudaBufferBody::shutdown()
{
	auto nonvolatile_shelf = ms_shelf();
	for (auto &shelf: nonvolatile_shelf) {
		delete shelf;
	}
}

void CudaBufferBody::report()
{
	printf("cudaBuffer:\n");
	printf("\t%10s %6s %10s %4s %5s %s\n", "width", "height", "pitch", "size", "count", "address");
	for (auto &shelf: ms_shelf()) {
		printf("\t%10d %6d %10d %4d %5d ", shelf->m_width, shelf->m_height, shelf->m_pitch,
		       shelf->m_byteStride, shelf->m_useCount);
		for (auto &buffer: shelf->m_datas) {
			printf("%-12p:", buffer);
		}
		printf(" %s\n", shelf->m_name.c_str());
	}
}

void CudaBufferBody::init(
        const std::string &name, uint32_t alloc_nelem, uint32_t byte_per_pixel,
        const std::vector<cudaStream_t> &streams)
{
	if (m_name.empty()) m_name = name;
	m_width = alloc_nelem;
	m_height = 1;
	m_depth = 1;
	m_pitch = alloc_nelem;
	m_byteStride = byte_per_pixel;

	auto alloc_byte_size = alloc_nelem * m_byteStride;

	if (alloc_byte_size > m_byteSize) {
		CudaScopedDevice dev;  // auto push-pop
		for (auto i = 0u; i < m_datas.size(); i++) {
			CudaScopedDevice::setDevice(i);
			auto stream = i < streams.size() ? streams[i] : 0;
			if (m_datas[i]) {
				cuda_ck(cudaFreeAsync(m_datas[i], stream));
			}
			cuda_ck(cudaMallocAsync(&m_datas[i], alloc_byte_size, stream));
		}
		m_byteSize = alloc_byte_size;
		REPORT();
	}
}

void CudaBufferBody::initArray(
        const std::string &name, uint32_t width, uint32_t height, uint32_t depth, uint32_t format,
        bool is_cubemap)
{
	if (is_cubemap) assert(m_width == m_height && m_depth % 6 == 0);

	m_name = name;
	m_format = format;
	m_width = width;
	m_height = height;
	m_depth = depth;
	m_pitch = m_height > 1 ? ROUND_UP(width, get_gpu_alignment() / PIXEL_BYTE) : m_width;
	m_byteStride = get_byte_per_pixel(format);

	auto byte_size = m_pitch * m_height * m_byteStride;
	if (byte_size != m_byteSize) {
		CudaScopedDevice dev;  // auto push-pop
		auto desc = get_cuda_channel_format_desc(m_format);
		for (auto i = 0u; i < m_datas.size(); i++) {
			CudaScopedDevice::setDevice(i);
			if (m_arrays[i]) {
				cuda_ck(cudaFreeArray(m_arrays[i]));
			}
			if (m_cudaTextureObjects[i]) {
				cuda_ck(cudaDestroyTextureObject(m_cudaTextureObjects[i]));
				m_cudaTextureObjects[i] = 0;
			}
			auto cuda_depth = m_depth == 1 ? 0 : m_depth;
			cudaExtent extent = {
			        .width = m_pitch,
			        .height = m_height,
			        .depth = cuda_depth,
			};
			auto flags = 0u;
			if (cuda_depth) flags |= cudaArrayLayered;
			if (is_cubemap) flags |= cudaArrayCubemap;
			cuda_ck(cudaMalloc3DArray(&m_arrays[i], &desc, extent, flags));
		}
		m_byteSize = byte_size;
		bindToCudaTexture(is_cubemap);
		REPORT();
	}
}

void CudaBufferBody::sendEach(
        uint32_t gpuid, const void *hostptr, uint32_t first, uint32_t count, cudaStream_t stream)
{
	if (hostptr == nullptr) {
		return;
	}
	else if (m_height == 1) {
		assert(m_datas[gpuid]);
		auto byte_first = first * m_byteStride;
		auto byte_count = count == 0 ? m_byteSize - byte_first : count * m_byteStride;
		cuda_ck(cudaMemcpyAsync(
		        (uint8_t *)m_datas[gpuid] + byte_first, (uint8_t *)hostptr + byte_first, byte_count,
		        cudaMemcpyDefault, stream));
	}
	else if (m_depth == 1) {
		assert(m_arrays[gpuid]);
		cuda_ck(cudaMemcpy2DToArrayAsync(
		        m_arrays[gpuid], 0, 0, hostptr, m_width * m_byteStride, m_width * m_byteStride,
		        m_height, cudaMemcpyDefault, stream));
	}
	else {
		if (count == 0) count = m_depth;

		cudaMemcpy3DParms parms;
		memset(&parms, 0, sizeof(parms));
		parms.srcPtr = make_cudaPitchedPtr((void *)hostptr, m_width * m_byteStride, m_width, m_height),
		parms.dstArray = m_arrays[gpuid];
		parms.dstPos = make_cudaPos(0, 0, first);
		parms.extent = make_cudaExtent(m_pitch, m_height, count);
		parms.kind = cudaMemcpyDefault;
		cuda_ck(cudaMemcpy3DAsync(&parms, stream));
	}
}

void CudaBufferBody::send(
        const void *hostptr, uint32_t first, uint32_t count, const std::vector<cudaStream_t> &streams)
{
	CudaScopedDevice dev;
	for (auto i = 0u; i < m_datas.size(); i++) {
		CudaScopedDevice::setDevice(i);
		auto stream = i < streams.size() ? streams[i] : 0;
		sendEach(i, hostptr, first, count, stream);
	}
}

void CudaBufferBody::recvEach(
        uint32_t gpuid, void *hostptr, uint32_t first, uint32_t count, cudaStream_t stream)
{
	auto byte_first = first * m_byteStride;
	auto byte_count = count == 0 ? m_byteSize - byte_first : count * m_byteStride;
	auto *gpu_ptr = static_cast<uint8_t *>(m_datas[gpuid]);

	assert(m_width == m_pitch);  // not implemented yet

	cuda_ck(cudaMemcpyAsync(
	        (uint8_t *)hostptr + byte_first, gpu_ptr + byte_first, byte_count, cudaMemcpyDefault, stream));
}

void CudaBufferBody::copy(const CudaBufferBody &src, const std::vector<cudaStream_t> &streams)
{
	for (auto i = 0u; i < m_datas.size(); i++) {
		auto &src_buffer = src.m_datas[i];
		auto stream = i < streams.size() ? streams[i] : 0;
		cuda_ck(cudaMemcpyAsync(
		        m_datas[i], src_buffer, m_byteSize, cudaMemcpyKind::cudaMemcpyDefault, stream));
	}
}

void CudaBufferBody::bindToCudaTexture(bool is_cubemap)
{
#ifdef MAINTENANCE
	printf("bindToCudaTexture:\n");
	printf("    format : %s\n", opengl_const(m_format));
	printf("    pitch  : %d\n", m_pitch);
	printf("    width  : %d\n", m_width);
	printf("    height : %d\n", m_height);
	printf("    depth  : %d\n", m_depth);
	printf("    cubemap: %d\n", is_cubemap);
#endif

	if (m_arrays[0] && m_cudaTextureObjects[0] == 0) {
		CudaScopedDevice dev;
		for (auto i = 0u; i < m_datas.size(); i++) {
			CudaScopedDevice::setDevice(i);
			cudaResourceDesc resdesc;
			memset(&resdesc, 0, sizeof(resdesc));

			resdesc.resType = cudaResourceTypeArray;
			resdesc.res.array.array = m_arrays[i];

			const auto read_mode = get_cuda_texture_read_mode(m_format);
			const auto wrap_mode = cudaAddressModeWrap;
			const auto is_srgb = m_format == GL_SRGB8_ALPHA8 ? 1 : 0;

			cudaTextureDesc texdesc = {
			        {wrap_mode, wrap_mode, wrap_mode}, // addressMode
			        cudaFilterModeLinear, // filterMode
			        read_mode, // readMode
			        is_srgb, // sRGB
			        {0.0f, 0.0f, 0.0f, 0.0f}, // borderColor
			        1, // normalizedCoords
			        16, // maxAnisotropy
			        cudaFilterModePoint, // mipmapFilterMode
			        0.0f, // mipmapLevelBias,
			        0.0f, // minMipmapLevelClamp
			        0.0f, // maxMipmapLevelClamp
			        0, // disableTrilinearOptimization
			        is_cubemap, // seamlessCubemap (11.6)
			};
			cuda_ck(cudaCreateTextureObject(&m_cudaTextureObjects[i], &resdesc, &texdesc, NULL))
		}
	}
}
}  // namespace spu::gs_cuda
