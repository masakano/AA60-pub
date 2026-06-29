//
// GsNodeMixer :
//
#include <gsys/util/node_mixer.h>

namespace spu {

bool GsNodeMixer::isSingleNode(const GsNode *node)
{
	if (node->instances().size() > 1) {
		return false;
	}
	if (node->getNotches().size() > 1) {
		return false;
	}
	if (node->getRanges().size() > 1) {
		return false;
	}
	return true;
}

bool GsNodeMixer::isCombinable(const GsNode *node) const
{
	if (m_node->getPainter()->getDrawcalls().size() != node->getPainter()->getDrawcalls().size()) {
		return false;
	}
	return isSingleNode(node);
}

bool GsNodeMixer::isDividable() const
{
	for (auto &drawcall: m_node->getPainter()->getDrawcalls()) {
		for (auto &com: drawcall.coms) {
			if (com.mode != GL_TRIANGLES) {
				return false;
			}
		}
	}
	return isSingleNode(m_node);
}

void GsNodeMixer::init(GsNode *node)
{
	assert(node);
	m_node = node;
	m_node->sync(false);

	auto *painter = m_node->getPainter();
	assert(painter);
	painter->recv(m_vertices, m_indices);
}

void GsNodeMixer::divide(const Vec4i &grid)
{
	auto regions = makeRanges(grid);
	auto *painter = m_node->getPainter();
	auto &drawcalls = painter->getDrawcalls();

	std::vector<int32_t> dst_indices;
	std::vector<Range3f> ranges(grid.x * grid.y * grid.x);

	for (auto &range: ranges) {
		range.invalidate();
	}

	for (auto &drawcall: drawcalls) {
		auto com = drawcall.coms[0];

		drawcall.coms.clear();
		for (size_t i = 0; i < regions.size(); i++) {
			auto new_com = com;
			new_com.first = dst_indices.size();
			assert(new_com.mode == GL_TRIANGLES);  // triangle only

			for (auto j = com.first; j < com.first + com.count; j += 3) {
				std::vector<int32_t> face = {
				        m_indices[j + 0],
				        m_indices[j + 1],
				        m_indices[j + 2],
				};

				auto center = ezero();
				for (auto &index: face) {
					center += m_vertices[index].p;
				}
				center /= face.size();

				if (regions[i].inside(center)) {
					for (auto &index: face) {
						ranges[i].expand(m_vertices[index].p);
						dst_indices.push_back(index);
					}
				}
			}
			new_com.count = int32_t(dst_indices.size()) - new_com.first;
			drawcall.coms.push_back(new_com);
		}
	}

	// erawe from tail
	for (auto i = grid.x * grid.y * grid.z - 1; i >= 0; i--) {
		auto count = 0;
		for (auto &drawcall: drawcalls) {
			count += drawcall.coms[i].count;
		}
		if (count == 0) {
			for (auto &drawcall: drawcalls) {
				drawcall.coms.erase(drawcall.coms.begin() + i);
			}
			ranges.erase(ranges.begin() + i);
		}
	}
	painter->SpuArray::send(dst_indices, -1);
	m_node->dupSubstances(ranges.size());
	m_node->getRanges() = ranges;
}

void GsNodeMixer::add(GsNode *sub_node)
{
	sub_node->sync(false);
	assert(isCombinable(sub_node));

	std::vector<Mesh::Vertex> sub_vertices;
	std::vector<int32_t> sub_indices;

	auto &ranges = m_node->getRanges();
	auto &sub_range = sub_node->getARange();
	auto *sub_painter = sub_node->getPainter();

	auto &drawcalls = m_node->getPainter()->getDrawcalls();
	auto &sub_drawcalls = sub_node->getPainter()->getDrawcalls();

	// drawcall
	for (size_t i = 0; i < sub_drawcalls.size(); i++) {
		auto coms = sub_drawcalls[i].coms;
		coms[0].base_vertex = m_vertices.size();
		coms[0].first += m_indices.size();
		drawcalls[i].coms.push_back(coms[0]);
	}
	sub_painter->recv(sub_vertices, sub_indices);
	vector_cat(m_vertices, sub_vertices);
	vector_cat(m_indices, sub_indices);
	ranges.push_back(sub_range);
}

void GsNodeMixer::send()
{
	auto *painter = m_node->getPainter();
	painter->send(m_vertices, m_indices);
}

std::vector<Range3f> GsNodeMixer::makeRanges(const Vec4i &grid)
{
	auto range = m_node->getARange();
	range.grow(Vec3f(1.01));  // 1% margin

	auto ndiv = Vec3f(grid);
	auto span = range.span() / ndiv;

	std::vector<Range3f> regions;
	for (auto z = 0; z < grid.z; z++) {
		for (auto y = 0; y < grid.y; y++) {
			for (auto x = 0; x < grid.x; x++) {
				auto p0 = range.p0 + Vec3f(float(x), float(y), float(z)) * span;
				auto p1 = p0 + span;
				regions.emplace_back(p0, p1);
			}
		}
	}
	return regions;
}

namespace {
template<class T> void vec_save(File &file, const std::vector<T> &v)
{
	uint32_t n = v.size();
	file.write(&n, sizeof(n));
	file.write(v.data(), sizeof(T) * n);
}

template<class T> void vec_load(File &file, std::vector<T> &v)
{
	uint32_t n;
	file.read(&n, sizeof(n));
	v.resize(n);
	file.read(v.data(), sizeof(T) * n);
}
}  // namespace

void GsNodeMixer::save(const char *cache_path)  // combine only for now
{
	File file(cache_path, "wb");
	for (auto &drawcall: m_node->getPainter()->getDrawcalls()) {
		vec_save(file, drawcall.coms);
	}
	vec_save(file, m_indices);
	vec_save(file, m_vertices);
}

bool GsNodeMixer::load(const char *cache_path)
{
	File file;

	if (!file.open(cache_path, "rb", false)) {
		aux_message(0, "%s: can't open\n", cache_path);
		return false;
	}

	for (auto &drawcall: m_node->getPainter()->getDrawcalls()) {
		vec_load(file, drawcall.coms);
	}

	vec_load(file, m_indices);
	vec_load(file, m_vertices);
	send();
	return true;
}

template<> size_t serialize(uint8_t *heap, bool is_dry, const GsNodeMixer &o)
{
	auto *hp = heap;
	for (auto &drawcall: o.m_node->getPainter()->getDrawcalls()) {
		hp += serialize(heap, is_dry, drawcall.coms);
	}
	hp += serialize(heap, is_dry, o.m_indices);
	hp += serialize(heap, is_dry, o.m_vertices);
	return hp - heap;
}
template<> size_t deserialize(const uint8_t *heap, GsNodeMixer &o)
{
	auto *hp = heap;
	for (auto &drawcall: o.m_node->getPainter()->getDrawcalls()) {
		hp += deserialize(heap, drawcall.coms);
	}
	hp += deserialize(heap, o.m_indices);
	hp += deserialize(heap, o.m_vertices);
	o.send();
	return hp - heap;
}

}  // namespace spu
