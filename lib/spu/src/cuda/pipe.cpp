//
// PipeEncoder :
//
#include "pipe.h"
#ifdef ck
#undef ck
#endif
#include "NV/Utils/pragmas.h"
#include "NV/NvDecoder/NvDecoder.h"
#include "NV/NvEncoder/NvEncoderCuda.h"
#include "color_space.h"
#ifdef ck
#undef ck
#endif
#define ck(e)                                                             \
	do {                                                              \
		auto err = (e);                                           \
		aux_error(err != 0, "General error code=%d\n", int(err)); \
	} while (0)

#include <algorithm>
#include <cstring>

namespace spu::libspu::video {

namespace {
static constexpr auto c_device_id = 0;
static constexpr auto c_decode_tries = 3u;
static constexpr auto c_extra_output_delay = 0u;
static constexpr auto c_buffer_format = NV_ENC_BUFFER_FORMAT_ABGR;

#ifndef _WIN32
inline bool operator==(const GUID &guid1, const GUID &guid2)
{
	return memcmp(&guid1, &guid2, sizeof(GUID)) == 0;
}
inline bool operator!=(const GUID &guid1, const GUID &guid2) { return !(guid1 == guid2); }
#endif

class ScopedDevice {
public:
	explicit ScopedDevice(int32_t device) { ck(cudaSetDevice(device)); }
	~ScopedDevice() = default;
};

GUID encode_guid(hash32_t codec) { return codec == "hevc" ? NV_ENC_CODEC_HEVC_GUID : NV_ENC_CODEC_H264_GUID; }

cudaVideoCodec decode_codec(hash32_t codec)
{
	return codec == "hevc" ? cudaVideoCodec_HEVC : cudaVideoCodec_H264;
}
void setup_encoder_params(
        NvEncoderCuda *encoder, NV_ENC_INITIALIZE_PARAMS *initialize_params, NV_ENC_CONFIG *encode_config,
        hash32_t codec, uint32_t bitrate, uint32_t target_fps)
{
	initialize_params->encodeConfig = encode_config;

	auto codec_guid = encode_guid(codec);
	auto preset_guid = NV_ENC_PRESET_P3_GUID;
	encoder->CreateDefaultEncoderParams(
	        initialize_params, codec_guid, preset_guid, NV_ENC_TUNING_INFO_LOW_LATENCY);

	encode_config->gopLength = NVENC_INFINITE_GOPLENGTH;
	encode_config->frameIntervalP = 1;

	if (codec_guid == NV_ENC_CODEC_H264_GUID) {
		encode_config->encodeCodecConfig.h264Config.idrPeriod = NVENC_INFINITE_GOPLENGTH;
	}
	else {
		encode_config->encodeCodecConfig.hevcConfig.idrPeriod = NVENC_INFINITE_GOPLENGTH;
	}

	encode_config->rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR;
	encode_config->rcParams.multiPass = NV_ENC_TWO_PASS_FULL_RESOLUTION;
	encode_config->rcParams.averageBitRate = bitrate;

	initialize_params->frameRateNum = target_fps;
	initialize_params->frameRateDen = 1;

	encode_config->rcParams.vbvBufferSize
	        = (encode_config->rcParams.averageBitRate * initialize_params->frameRateDen
	           / initialize_params->frameRateNum)
	        * 5;
	encode_config->rcParams.maxBitRate = encode_config->rcParams.averageBitRate;
	encode_config->rcParams.vbvInitialDelay = encode_config->rcParams.vbvBufferSize;
}
}  // namespace

Inventory::~Inventory()
{
	for (auto &r: m_resources) {
		ck(cudaGraphicsUnregisterResource(r.second));
	}
}

void Inventory::erase(uint32_t texture_id) { m_resources.erase(texture_id); }

cudaGraphicsResource_t Inventory::get(uint32_t texture_id, uint32_t flags)
{
	auto &resource = m_resources[texture_id];

	if (resource) {
		ck(cudaGraphicsUnregisterResource(resource));
	}

	int id = texture_id & 0xffff;
	int target = (texture_id >> 16) & 0xffff;
	ck(cudaGraphicsGLRegisterImage(&resource, id, target, flags));
	return resource;
}

Pipe::Pipe(const Attrs &attrs, uint32_t flag) : m_resourceFlag(flag)
{
	ScopedDevice dev(c_device_id);
	m_textureId = attrs.get("texture_id", 0);
	assert((m_textureId >> 16) == GL_TEXTURE_2D);

	uint32_t iformat = 0;
	spu_texture_get(m_textureId, "iformat", &iformat);
	assert(iformat == GL_RGBA8 || iformat == GL_SRGB8_ALPHA8);

	spu_texture_get(m_textureId, "width", &m_width);
	spu_texture_get(m_textureId, "height", &m_height);

	m_resource = m_inventory.get(m_textureId, m_resourceFlag);

	m_codec = attrs.get("codec", "?hevc:h264");
}
Pipe::~Pipe()
{
	unmapResource();
	m_resource = nullptr;
}

void Pipe::mapResource()
{
	if (!m_isMapped) {
		ck(cudaGraphicsMapResources(1, &m_resource));
		m_isMapped = true;
	}
}

void Pipe::unmapResource()
{
	if (m_isMapped) {
		ck(cudaGraphicsUnmapResources(1, &m_resource));
		m_isMapped = false;
	}
}

Pipe::ScopedMappedArray::ScopedMappedArray(Pipe &pipe) : m_pipe(pipe)
{
	if (!m_pipe.m_isMapped) {
		m_pipe.mapResource();
		m_isOwner = true;
	}
	ck(cudaGraphicsSubResourceGetMappedArray(&m_array, m_pipe.m_resource, 0, 0));
}

Pipe::ScopedMappedArray::~ScopedMappedArray()
{
	if (m_isOwner) {
		m_pipe.unmapResource();
	}
}

PipeEncoder::PipeEncoder(const Attrs &attrs, Sender *sender)
        : Pipe(attrs, cudaGraphicsRegisterFlagsReadOnly), m_sender(sender)
{
	m_bitrate = attrs.get("bitrate", c_bitrate);
	m_targetFPS = attrs.get("target_fps", c_target_fps);
	assert(m_bitrate);
	assert(m_targetFPS);

	mapResource();
	restart();
}

PipeEncoder::~PipeEncoder() { dispose(); }

bool PipeEncoder::send_encoded_frames(
        const std::vector<NvEncOutputFrame> &packets, const FrameMetadata &metadata)
{
	auto bitstream_size = size_t(0);
	for (auto &packet: packets) {
		bitstream_size += packet.frame.size();
	}

	if (bitstream_size > uint32_t(-1)) {
		aux_message(0, "encoded video frame is too large: %zu\n", bitstream_size);
		return false;
	}

	std::vector<uint8_t> bitstream;
	bitstream.reserve(bitstream_size);
	for (auto &packet: packets) {
		bitstream.insert(bitstream.end(), packet.frame.begin(), packet.frame.end());
	}

	if (m_sender->sendFrame(bitstream.data(), uint32_t(bitstream.size()), metadata) == false) {
		aux_message(0, "encode interrupt\n");
		return false;
	}
	return true;
}

bool PipeEncoder::encode_background()
{
	ScopedDevice dev(c_device_id);
	if (m_sender->waitControlClock() == false) {
		return false;
	}
	FrameMetadata metadata;
	metadata.control_id = m_sender->sampleControlId();
	metadata.input_serial = m_inputSerial;
	const NvEncInputFrame *f = m_encoder->GetNextInputFrame();

	ScopedMappedArray mapped_array(*this);
	ck(cudaMemcpy2DFromArray(
	        f->inputPtr, f->pitch, mapped_array.array(), 0, 0, m_width * 4, m_height,
	        cudaMemcpyDeviceToDevice));

	auto is_key_frame_requested = m_sender->consumeKeyFrameRequest();
	auto is_force_key_frame = m_isForceIFrame || is_key_frame_requested;
	if (is_force_key_frame) {
		metadata.flags |= FrameMetadata::e_key_frame;
	}
	if (is_force_key_frame) {
		aux_message(0, "video encoder force IDR: requested=%d\n", is_key_frame_requested);
	}

	std::vector<NvEncOutputFrame> packets;
	if (is_force_key_frame) {
		NV_ENC_PIC_PARAMS params = {};
		params.encodePicFlags = NV_ENC_PIC_FLAG_FORCEIDR | NV_ENC_PIC_FLAG_OUTPUT_SPSPPS;
		m_encoder->EncodeFrame(packets, &params);
	}
	else {
		m_encoder->EncodeFrame(packets);
	}

	return send_encoded_frames(packets, metadata);
}

bool PipeEncoder::encode(const uint32_t *embed_value)
{
	auto result = true;
	if (m_future.valid()) {
		result = m_future.get();
	}
	if (result == false) {
		restart();
	}
	m_inputSerial = embed_value ? *embed_value : 0;
	m_future = std::async(std::launch::async, [&] { return encode_background(); });
	return result;
}

void PipeEncoder::alloc()
{
	ScopedDevice dev(c_device_id);

	if (m_encoder == nullptr) {
		CUcontext cudaContext;
		cuCtxGetCurrent(&cudaContext);
		m_encoder = new NvEncoderCuda(
		        cudaContext, m_width, m_height, c_buffer_format, c_extra_output_delay);

		NV_ENC_INITIALIZE_PARAMS initializeParams = {};
		initializeParams.version = NV_ENC_INITIALIZE_PARAMS_VER;
		NV_ENC_CONFIG encodeConfig = {};
		encodeConfig.version = NV_ENC_CONFIG_VER;
		setup_encoder_params(
		        m_encoder, &initializeParams, &encodeConfig, m_codec, m_bitrate, m_targetFPS);
		m_encoder->CreateEncoder(&initializeParams);
	}
}

void PipeEncoder::dispose()
{
	ScopedDevice dev(c_device_id);
	ck(cudaDeviceSynchronize());

	if (m_future.valid()) {
		m_future.get();
	}

	if (m_encoder) {
		std::vector<NvEncOutputFrame> tmp;
		m_encoder->EndEncode(tmp);
		m_encoder->DestroyEncoder();
		delete m_encoder;
		m_encoder = nullptr;
	}
}

void PipeEncoder::restart()
{
	dispose();
	while (m_sender->isPause() == true) {
		auto c_wait_usec = 1000000u;
		aux_message(0, "wait for client (%d)...\n", m_restartCount++);  // force to output
		sleep_microsec(c_wait_usec);
	}
	alloc();
}

PipeDecoder::PipeDecoder(const Attrs &attrs, Receiver *receiver)
        : Pipe(attrs, cudaGraphicsRegisterFlagsWriteDiscard), m_receiver(receiver)
{
	alloc();
}

PipeDecoder::~PipeDecoder() { dispose(); }

bool PipeDecoder::receive_bitstreams(std::deque<ReceivedFrame> *bitstreams)
{
	return m_receiver->receiveFrames(bitstreams);
}

uint8_t *PipeDecoder::decode_one_frame(const ReceivedFrame &frame)
{
	auto &bitstream = frame.bitstream;
	int decoded_frame_count = 0;
	uint8_t *decoded_frame = nullptr;
	int64_t time_stamp = 0;

	for (auto i = 0u; (i < c_decode_tries) && (decoded_frame_count <= 0); ++i) {
		decoded_frame_count = m_decoder->Decode(
		        bitstream.data(), int(bitstream.size()), CUVID_PKT_ENDOFPICTURE, m_count++);
		for (auto frame_i = 0; frame_i < decoded_frame_count; ++frame_i) {
			decoded_frame = m_decoder->GetFrame(&time_stamp);
		}
	}
	if (decoded_frame_count != 1 || decoded_frame == nullptr) {
		aux_message(
		        0, "no frame decoded. encoder codec may be mismatch (decoder codec is \"%s\")\n",
		        m_codec.c_str());
		return nullptr;
	}
	return decoded_frame;
}

bool PipeDecoder::decode(uint32_t *embed_value)
{
	ScopedDevice dev(c_device_id);

	std::deque<ReceivedFrame> bitstreams;
	if (receive_bitstreams(&bitstreams) == false) {
		return false;
	}
	if (bitstreams.size() > 1) {
		aux_message(2, "video decoder drain: frames=%zu\n", bitstreams.size());
	}

	uint8_t *decoded_frame = nullptr;
	FrameMetadata decoded_metadata;
	for (auto &bitstream: bitstreams) {
		if (auto frame = decode_one_frame(bitstream)) {
			decoded_frame = frame;
			decoded_metadata = bitstream.metadata;
		}
	}
	if (decoded_frame == nullptr) {
		return false;
	}

	Nv12ToColor32<RGBA32>(
	        decoded_frame, m_decoder->GetDeviceFramePitch(), (u_char *)m_deviceBuffer, m_width * 4, m_width,
	        m_height);

	if (embed_value) {
		*embed_value = decoded_metadata.input_serial;
	}

	{
		ScopedMappedArray mapped_array(*this);
		ck(cudaMemcpy2DToArray(
		        mapped_array.array(), 0, 0, m_deviceBuffer, m_width * 4, m_width * 4, m_height,
		        cudaMemcpyDeviceToDevice));
	}
	return true;
}

void PipeDecoder::alloc()
{
	ScopedDevice dev(c_device_id);

	if (m_deviceBuffer == nullptr) {
		ck(cudaMalloc(&m_deviceBuffer, m_width * m_height * 4));
	}
	if (m_decoder == nullptr) {
		CUcontext cudaContext;
		cuCtxGetCurrent(&cudaContext);
		m_decoder = new NvDecoder(cudaContext, true, decode_codec(m_codec), true);
	}
}

void PipeDecoder::dispose()
{
	ScopedDevice dev(c_device_id);
	ck(cudaDeviceSynchronize());

	if (m_decoder) {
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#endif
		delete m_decoder;
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
		m_decoder = nullptr;
	}
	if (m_deviceBuffer) {
		cudaFree(m_deviceBuffer);
		m_deviceBuffer = nullptr;
	}
}
}  // namespace spu::libspu::video
