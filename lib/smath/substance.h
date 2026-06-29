//
// Substance :
//

///
///
///

#pragma once
#include "mat4f.h"
#include <ssys/serializer.h>
#include <ssys/vector_view.h>

#define check_stride(T)                                                                                                \
	aux_error(                                                                                                     \
	        sizeof(T) % sizeof(uint32_t) != 0 || sizeof(T) > m_substanceStride,                                    \
	        "stride too small. (%ld > %d). Auto detection might be failed. try 'setInstanceStride()' explicity\n", \
	        sizeof(T), m_substanceStride);

namespace spu {

class Substance {
public:
	Substance();

	void setMinNotch(float min_notch) { m_minNotch = min_notch; }
	void initSubstances(uint32_t byte_stride, uint32_t slot_count);
	void buildInstances(const std::vector<Mat4f> &nodeworlds = {Mat4f()});
	void pruneInstances(const Mat4f &worldscreen, const Range3f &range, bool is_lod = false);

	const Range3f &getARange() const;
	Range3f &getARange();

	const std::vector<Range3f> &getRanges() const { return m_ranges; }
	std::vector<Range3f> &getRanges() { return m_ranges; }

	const std::vector<float> &getNotches() const { return m_notches; }
	std::vector<float> &getNotches() { return m_notches; }

	const std::vector<std::vector<uint8_t>> &instances() const { return m_instances; }
	const std::vector<std::vector<Mat4f>> &instancedNodeworlds();
	const std::vector<std::vector<Vec3f>> &instancedPoints();
	const std::vector<std::vector<Mat4f>> &instancedFrustums();

	float minNotch() const { return m_minNotch; }
	uint32_t substanceStride() const { return m_substanceStride; }
	uint32_t substanceSlotCount() const { return m_substances.size(); }
	uint32_t substanceCount(uint32_t slot) const;
	uint32_t instanceCount(uint32_t slot) const;

	void clearSubstances(uint32_t slot_count);
	void dupSubstance(uint32_t slot, uint32_t substance_count);
	void dupSubstances(uint32_t slot_count);

	template<class T = Mat4f> VectorView<T, const std::vector<uint8_t>> getInstances(uint32_t slot) const
	{
		check_stride(T);
		return VectorView<T, const std::vector<uint8_t>>(&m_instances.at(slot), m_substanceStride);
	}

	template<class T = Mat4f> VectorView<T, const std::vector<uint8_t>> getSubstances(uint32_t slot) const
	{
		check_stride(T);
		return VectorView<T, const std::vector<uint8_t>>(&m_substances.at(slot), m_substanceStride);
	}

	template<class T = Mat4f> VectorView<T, std::vector<uint8_t>> getSubstances(uint32_t slot)
	{
		check_stride(T);
		return VectorView<T, std::vector<uint8_t>>(&m_substances.at(slot), m_substanceStride);
	}

	template<class T = Mat4f> const T &getASubstance() const
	{
		check_stride(T);
		assert(m_substances.size() == 1 && substanceCount(0) == 1);
		return *reinterpret_cast<const T *>(&m_substances.at(0).at(0));
	}

	template<class T = Mat4f> T &getASubstance()
	{
		check_stride(T);
		assert(m_substances.size() == 1 && substanceCount(0) == 1);
		return *reinterpret_cast<T *>(&m_substances.at(0).at(0));
	}
	void report(const char *str) const;

private:
	std::vector<std::vector<uint8_t>> m_substances;
	std::vector<std::vector<uint8_t>> m_instances;
	std::vector<std::vector<Mat4f>> m_instancedNodeworlds;
	std::vector<std::vector<Vec3f>> m_instancedPoints;
	std::vector<std::vector<Mat4f>> m_instancedFrustums;

	std::vector<Range3f> m_ranges;
	std::vector<float> m_notches;
	float m_minNotch = 0.01;  // 1% of screen
	uint32_t m_substanceStride = 0;

	void clearInstances();
	void copyInstance(std::vector<uint8_t> &dst, std::vector<uint8_t> &src, uint32_t src_index);
	void copyInstance(
	        std::vector<uint8_t> &dst, std::vector<uint8_t> &src, uint32_t src_index,
	        const Mat4f &nodeworld);

	SPU_SERIALIZER_FRIENDS
};

template<> size_t serialize(uint8_t *heap, bool is_dry, const Substance &object);
template<> size_t deserialize(const uint8_t *heap, Substance &object);

}  // namespace spu
#undef check_stride
