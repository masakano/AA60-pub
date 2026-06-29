//
// Dummy :
//
#pragma once
#include <gsys/painter.h>

namespace spu::gs_painter {

class Dummy : public GsPainter {
public:
	using Vertex = Mesh::Vertex;

	Dummy(const char *name = nullptr) : GsPainter(name) {}
	Dummy(const Attrs &attrs) { init(attrs); }

	auto getVerticesView() const { return doGetVerticesView<Vertex>(); }
	auto getVerticesView() { return doGetVerticesView<Vertex>(); }

	void send() override { /* do nothing */ }

	void recv(std::vector<uint8_t> &vertices, std::vector<int32_t> &indices) const override
	{
		getVerticesAndIndices(vertices, indices);
	}
	void send(const std::vector<Mesh::Vertex> &mesh_vertices, const std::vector<int32_t> &indices) override
	{
		sendMeshVertices<Vertex>(mesh_vertices, indices);
	}
	void recv(std::vector<Mesh::Vertex> &mesh_vertices, std::vector<int32_t> &indices) const override
	{
		recvMeshVertices<Vertex>(mesh_vertices, indices);
	}
	void init(const Attrs &) override { setIsKeepInHost(true); }
};
}  // namespace spu::gs_painter
