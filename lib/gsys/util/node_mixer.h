//
// GsNodeMixer :
//
#pragma once
#include <gsys/node.h>
#include <gsys/painter.h>

namespace spu {

class GsNodeMixer {
public:
	static bool isSingleNode(const GsNode *node);
	bool isCombinable(const GsNode *node) const;
	bool isDividable() const;

	GsNode *get() { return m_node; }

	void init(GsNode *node);
	void add(GsNode *sub_node);
	void divide(const Vec4i &grid);
	void send();
	void save(const char *cache_path);
	bool load(const char *cache_path);

private:
	GsNode *m_node = nullptr;

	std::vector<Mesh::Vertex> m_vertices;
	std::vector<int32_t> m_indices;
	std::vector<Range3f> makeRanges(const Vec4i &grid);

	SPU_SERIALIZER_FRIENDS
};

template<> size_t serialize(uint8_t *heap, bool is_dry, const GsNodeMixer &object);
template<> size_t deserialize(const uint8_t *heap, GsNodeMixer &object);

}  // namespace spu
