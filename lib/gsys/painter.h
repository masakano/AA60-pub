//
// GsPainter :
//
#pragma once

#include "object.h"
#include "drawcall.h"
#include "decorator.h"
#include <smath/mesh.h>
#include <smath/substance.h>

#define PAINTER_VERTEX_FUNCS                                                                               \
	auto getVerticesView() const { return doGetVerticesView<Vertex>(); }                               \
	auto getVerticesView() { return doGetVerticesView<Vertex>(); }                                     \
	void send() override { doSend<Vertex>(); }                                                         \
	void send(const std::vector<Mesh::Vertex> &vertices, const std::vector<int32_t> &indices) override \
	{                                                                                                  \
		sendMeshVertices<Vertex>(vertices, indices);                                               \
	}                                                                                                  \
	void recv(std::vector<Mesh::Vertex> &vertices, std::vector<int32_t> &indices) const override       \
	{                                                                                                  \
		recvMeshVertices<Vertex>(vertices, indices);                                               \
	}

#define PAINTER_VERTEX_FUNCS_WITHOUT_MESH                                    \
	auto getVerticesView() const { return doGetVerticesView<Vertex>(); } \
	auto getVerticesView() { return doGetVerticesView<Vertex>(); }       \
	void send() override { doSend<Vertex>(); }

namespace spu {

/// object renderer
class GsPainter : public SpuArray, public GsObject {
public:
	static constexpr int32_t e_class_depth = GsObject::e_class_depth + 1;
	static constexpr hash32_t e_flip = "flip";
	static constexpr hash32_t e_radiance = "radiance";
	static constexpr hash32_t e_depth = "depth";
	static constexpr hash32_t e_deferred = "deferred";
	static constexpr hash32_t e_sub_radiance = "sub_radiance";
	static constexpr hash32_t e_sub_depth = "sub_depth";
	static constexpr hash32_t e_sub_deferred = "sub_deferred";

	explicit GsPainter(const char *name = nullptr);
	explicit GsPainter(const Attrs &attrs) : GsPainter() { init(attrs); }
	~GsPainter();

	virtual void draw(
	        uint32_t mode, const std::vector<Mat4f> &nodeworlds, uint32_t first = 0, uint32_t count = 0,
	        uint32_t instance_count = 1, uint32_t target = 0, uint32_t base_vertex = 0,
	        uint32_t base_instance = 0);

	void draw(
	        uint32_t mode, uint32_t first = 0, uint32_t count = 0, uint32_t instance_count = 1,
	        uint32_t target = 0, uint32_t base_vertex = 0, uint32_t base_instance = 0) override;

	virtual void render(const void *instance_ptr, uint32_t instance_count);
	virtual bool hasShader(const hash32_t &type) const;

	virtual void recv(std::vector<uint8_t> &vertices, std::vector<int32_t> &indices) const;
	virtual void recv(std::vector<Mesh::Vertex> &mesh_vertices, std::vector<int32_t> &indices) const;

	virtual void send();
	virtual void send(const std::vector<Mesh::Vertex> &mesh_vertices, const std::vector<int32_t> &indices);

	virtual std::map<hash32_t, SpuShader> &getShaders() { return m_shaders; }
	virtual const std::map<hash32_t, SpuShader> &getShaders() const { return m_shaders; }

	virtual hash32_t &getShaderType() { return m_shaderType; }
	virtual const hash32_t &getShaderType() const { return m_shaderType; }

	virtual std::vector<GsDrawcall> &getDrawcalls() { return m_drawcalls; }
	virtual const std::vector<GsDrawcall> &getDrawcalls() const { return m_drawcalls; }

	virtual std::vector<GsDecorator *> &getDecorators() { return m_decorators; }
	virtual const std::vector<GsDecorator *> &getDecorators() const { return m_decorators; }

	virtual Range3f &getRange() { return m_range; }
	virtual const Range3f &getRange() const { return m_range; }

	virtual GsDrawcall &getADrawcall();
	virtual const GsDrawcall &getADrawcall() const;

	virtual void addUniforms(const Attrs &unif_attrs);
	virtual uint32_t instanceStride() const;

	virtual const void *instancePtr() const { return m_instancePtr; }
	virtual uint32_t instanceCount() const { return m_instanceCount; }

	virtual void setNode(GsNode *node) { m_node = node; }
	virtual GsNode *getNode() const { return m_node; }

	void update() override;
	void dispose() override;
	void init(const Attrs &attrs) override;

	void set(const Attrs &attrs) override;
	using GsObject::set;

	void report(const char *str) const override;

	const std::vector<int32_t> &getIndices() const { return m_indices; }
	std::vector<int32_t> &getIndices() { return m_indices; }

	template<class T = Mat4f> void render(const std::vector<T> &instances = std::vector<T>(1))
	{
		auto instance_stride = instanceStride();
		assert(instance_stride);

		auto instance_count = instances.size() * sizeof(T) / instance_stride;
		render(instances.data(), instance_count);
	}

protected:
	virtual void doRender();
	virtual void doUse(uint32_t) {}
	virtual void doDebugRender() {}

	void getVerticesAndIndices(std::vector<uint8_t> &vertices, std::vector<int32_t> &indices) const
	{
		vertices = m_vertices;
		indices = m_indices;
	}
	void setIsKeepInHost(bool is_keep_in_host) { m_isKeepInHost = is_keep_in_host; }

	bool doSync(bool is_nonblock) override;

	template<class T> auto doGetVerticesView() const
	{
		return VectorView<T, const std::vector<uint8_t>>(&m_vertices);
	}
	template<class T> auto doGetVerticesView() { return VectorView<T, std::vector<uint8_t>>(&m_vertices); }

	template<class T> void doSend()
	{
		auto vertices_view = VectorView<T, const std::vector<uint8_t>>(&m_vertices);
		m_range.invalidate();
		for (auto &v: vertices_view) {
			m_range.expand(Vec3f(v));
		}

		SpuArray::send(m_vertices.data(), m_vertices.size() / sizeof(T), 0);
		SpuArray::send(m_indices, -1);

		if (!m_isKeepInHost) {
			m_vertices.clear();
			m_indices.clear();
			m_vertices.shrink_to_fit();
			m_indices.shrink_to_fit();
		}
	}

	template<class T>
	void sendMeshVertices(
	        const std::vector<Mesh::Vertex> &mesh_vertices, const std::vector<int32_t> &indices)
	{
		m_range.invalidate();
		auto vertices_view = doGetVerticesView<T>();
		for (auto &mv: mesh_vertices) {
			m_range.expand(mv.p);
			vertices_view.push_back(T(mv));
		}
		m_indices = indices;
		send();
	}

	template<class T>
	void recvMeshVertices(std::vector<Mesh::Vertex> &mesh_vertices, std::vector<int32_t> &indices) const
	{
		std::vector<uint8_t> vertices;
		recv(vertices, indices);

		mesh_vertices.clear();
		auto vertices_view = VectorView<T, const std::vector<uint8_t>>(&vertices);
		for (auto &v: vertices_view) {
			mesh_vertices.push_back(Mesh::Vertex(v));
		}
	}

private:
	std::vector<uint8_t> m_vertices;
	std::vector<int32_t> m_indices;

	std::vector<GsDrawcall> m_drawcalls;
	std::map<hash32_t, SpuShader> m_shaders;
	std::vector<GsDecorator *> m_decorators;
	SpuArray::callback_t m_callback;

	const void *m_instancePtr = nullptr;
	hash32_t m_shaderType = nullptr;
	uint32_t m_instanceCount = 0;
	bool m_isKeepInHost = false;

	Range3f m_range;

	GsNode *m_node = nullptr;
	mutable uint32_t m_instanceStride = 0;

	void coreRender(const hash32_t &shader_type);
};
}  // namespace spu
