//
// Substance :
//
#include <smath/substance.h>
#include <smath/geometry.h>

namespace spu {

Substance::Substance()
{
	initSubstances(sizeof(Mat4f), 1);
	m_notches.resize(1, 0);
	m_ranges.resize(1);
	m_ranges[0].invalidate();
}

void Substance::clearInstances()
{
	m_instancedPoints.clear();
	m_instances.clear();
	m_instancedFrustums.clear();
	m_instancedNodeworlds.clear();
}

uint32_t Substance::substanceCount(uint32_t slot) const
{
	return m_substances.at(slot).size() / m_substanceStride;
}
uint32_t Substance::instanceCount(uint32_t slot) const
{
	return m_instances.at(slot).size() / m_substanceStride;
}

void Substance::clearSubstances(uint32_t slot_count)
{
	m_substances.clear();
	m_substances.resize(slot_count);
}

void Substance::dupSubstance(uint32_t slot, uint32_t substance_count)
{
	auto &substances = m_substances.at(slot);
	substances.resize(substance_count * m_substanceStride);
	for (auto i = 1u; i < substance_count; i++) {
		auto substance_index = i * m_substanceStride;
		memcpy(&substances[substance_index], &substances[0], m_substanceStride);
	}
}

void Substance::dupSubstances(uint32_t slot_count)
{
	m_substances.resize(slot_count);
	for (auto slot = 1u; slot < slot_count; slot++) {
		if (substanceCount(slot) < substanceCount(0)) {
			m_substances[slot] = m_substances[0];
		}
	}
}

void Substance::copyInstance(std::vector<uint8_t> &dst, std::vector<uint8_t> &src, uint32_t src_index)
{
	auto dst_index = dst.size();
	dst.resize(dst_index + m_substanceStride);
	memcpy(&dst[dst_index], &src[src_index], m_substanceStride);
}

void Substance::copyInstance(
        std::vector<uint8_t> &dst, std::vector<uint8_t> &src, uint32_t src_index, const Mat4f &nodeworld)
{
	auto dst_index = dst.size();
	dst.resize(dst_index + m_substanceStride);
	memcpy(&dst[dst_index], &src[src_index], m_substanceStride);
	memcpy(&dst[dst_index], &nodeworld, sizeof(Mat4f));
}

void Substance::buildInstances(const std::vector<Mat4f> &nodeworlds)
{
	clearInstances();

	m_instances.resize(m_substances.size());
	for (auto slot = 0u; slot < m_substances.size(); slot++) {
		auto &substances = m_substances.at(slot);
		auto substance_count = substances.size() / m_substanceStride;
		auto substance_nodeworlds = getSubstances<Mat4f>(slot);

		auto &instances = m_instances.at(slot);
		for (auto i = 0u; i < substance_count; i++) {
			auto substance_index = i * m_substanceStride;
			auto substance_nodeworld = substance_nodeworlds.at(i);
			for (const auto &nodeworld: nodeworlds) {
				auto instance_nodeworld = nodeworld * substance_nodeworld;
				copyInstance(instances, substances, substance_index, instance_nodeworld);
			}
		}
	}
}

const std::vector<std::vector<Mat4f>> &Substance::instancedNodeworlds()
{
	if (!m_instancedNodeworlds.empty()) return m_instancedNodeworlds;
	if (m_instances.empty()) buildInstances();

	for (auto slot = 0u; slot < m_instances.size(); slot++) {
		auto nodeworlds = getInstances<Mat4f>(slot);
		m_instancedNodeworlds.emplace_back(nodeworlds);
	}
	return m_instancedNodeworlds;
}

const std::vector<std::vector<Vec3f>> &Substance::instancedPoints()
{
	if (!m_instancedPoints.empty()) return m_instancedPoints;
	if (m_instances.empty()) buildInstances();

	m_instancedPoints.resize(m_instances.size());
	for (auto slot = 0u; slot < m_instances.size(); slot++) {
		const auto &range = vector_at(m_ranges, slot);
		if (range.valid()) {
			auto nodeworlds = getInstances<Mat4f>(slot);
			auto points = range.points();
			for (auto &nodeworld: nodeworlds) {
				vector_cat(m_instancedPoints.at(slot), nodeworld.pers3(points));
			}
		}
	}
	return m_instancedPoints;
}

const std::vector<std::vector<Mat4f>> &Substance::instancedFrustums()
{
	if (!m_instancedFrustums.empty()) return m_instancedFrustums;
	if (m_instances.empty()) buildInstances();

	m_instancedFrustums.resize(m_instances.size());
	for (auto slot = 0u; slot < m_instances.size(); slot++) {
		const auto &range = vector_at(m_ranges, slot);
		if (range.valid()) {
			auto nodeworlds = getInstances<Mat4f>(slot);
			auto frustum = Mat4f(range);
			for (auto &nodeworld: nodeworlds) {
				auto worldnode = nodeworld.inverse();
				m_instancedFrustums.at(slot).push_back(frustum * worldnode);
			}
		}
	}
	return m_instancedFrustums;
}

void Substance::pruneInstances(const Mat4f &worldscreen, const Range3f &range, bool is_lod)
{
	if (m_instances.empty()) buildInstances();

	std::vector<std::vector<uint8_t>> pruned_instances;

	// LOD
	if (is_lod && m_instances.size() == 1 && m_notches.size() > 1) {
		auto frustum = Mat4f(range);
		auto center = Vec4f(range.center(), 1);
		auto dx = length(worldscreen.c[0]);
		auto dy = length(worldscreen.c[1]);
		auto dz = length(worldscreen.c[2]);
		auto scale = std::max({dx, dy, dz});

		auto &instances = m_instances.at(0);
		auto nodeworlds = getInstances<Mat4f>(0);

		for (auto i = 0u; i < nodeworlds.size(); i++) {
			auto &nodeworld = nodeworlds[i];
			auto worldnode = nodeworld.inverse();
			if (worldscreen.intersect(frustum * worldnode)) {
				auto w = ((worldscreen * nodeworld) * center).w;
				auto slot = 0u;
				if (w > 0) {
					auto scale_screen = scale / w;
					while (slot < m_notches.size()
					       && m_notches[slot] * scale_screen < m_minNotch) {
						slot++;
					}
				}
				if (slot >= pruned_instances.size()) {
					pruned_instances.resize(slot + 1);
				}
				auto instance_index = i * m_substanceStride;
				copyInstance(pruned_instances.at(slot), instances, instance_index);
			}
		}
	}
	// NORMAL
	else {
		auto frustum = Mat4f(range);
		pruned_instances.resize(m_instances.size());
		for (auto slot = 0u; slot < m_instances.size(); slot++) {
			auto &instances = m_instances.at(slot);
			auto nodeworlds = getInstances<Mat4f>(slot);
			for (auto i = 0u; i < nodeworlds.size(); i++) {
				auto worldnode = nodeworlds[i].inverse();
				if (worldscreen.intersect(frustum * worldnode)) {
					auto instance_index = i * m_substanceStride;
					copyInstance(pruned_instances.at(slot), instances, instance_index);
				}
			}
		}
	}
	clearInstances();
	m_instances = pruned_instances;
}

void Substance::report(const char *str) const
{
	if (str && *str) aux_printf("%s:\n", str);

	aux_printf("  instanced nodeworlds: %ld\n", m_instances.size());
	aux_printf("  stride : %d\n", m_substanceStride);
	aux_printf("  notches: %ld ( ", m_notches.size());
	for (const auto &notch: m_notches) {
		aux_printf("%f ", notch);
	}
	aux_printf(")\n");

	aux_printf("  substance: %ld ( ", m_substances.size());
	for (auto slot = 0u; slot < m_substances.size(); slot++) {
		aux_printf("%ld ", substanceCount(slot));
	}
	aux_printf(")\n");
}

void Substance::initSubstances(uint32_t byte_stride, uint32_t slot_count)
{
	aux_error(byte_stride < sizeof(Mat4f), "too small stride (%d)\n", byte_stride);
	if (byte_stride != m_substanceStride || slot_count != m_substances.size()) {
		m_substanceStride = byte_stride;
		m_substances.resize(slot_count);
		for (auto slot = 0u; slot < slot_count; slot++) {
			getSubstances(slot) = {Mat4f()};
		}
	}
}

const Range3f &Substance::getARange() const
{
	assert(m_ranges.size() == 1);
	return m_ranges[0];
}

Range3f &Substance::getARange()
{
	return const_cast<Range3f &>(const_cast<const Substance *>(this)->getARange());
}

template<> size_t serialize(uint8_t *heap, bool is_dry, const Substance &object)
{
	auto *hp = heap;
	hp += serialize(hp, is_dry, object.m_substanceStride);
	hp += serialize(hp, is_dry, object.m_minNotch);
	hp += serialize(hp, is_dry, object.m_notches);
	hp += serialize(hp, is_dry, object.m_ranges);
	hp += serialize(hp, is_dry, object.m_substances);
	return hp - heap;
}

template<> size_t deserialize(const uint8_t *heap, Substance &object)
{
	object.clearInstances();

	auto *hp = heap;
	hp += deserialize(hp, object.m_substanceStride);
	hp += deserialize(hp, object.m_minNotch);
	hp += deserialize(hp, object.m_notches);
	hp += deserialize(hp, object.m_ranges);
	hp += deserialize(hp, object.m_substances);
	return hp - heap;
}
}  // namespace spu
