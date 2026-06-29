//
//$<<Header>>$
//
#pragma once

#include <shapes/shape.hpp>
#include <spu++/spu++.h>

namespace spu::oglplus::shapes {

inline void loadShader(SpuShader &shader, const Attrs &shader_attrs, const Attrs &unif_attrs)
{
	Attrs pre_attrs = {
	        {"use_unif_block", true},
	};
	shader.init(pre_attrs + shader_attrs);
	shader.addUniforms(unif_attrs);
}

inline void loadShader(SpuShader &shader, const char *path, const Attrs &shader_attrs, const Attrs &unif_attrs)
{
	Attrs pre_attrs = {
	        {"use_unif_block", true},
	};
	shader.init(path, pre_attrs + shader_attrs);
	shader.addUniforms(unif_attrs);
}

class Array : public SpuArray {
public:
	Array() : m_shaders(1) {}

	void setMaxShaderType(uint32_t max_type) { m_shaders.resize(max_type); }

	void setShaderType(uint32_t type)
	{
		assert(type < uint32_t(m_shaders.size()));
		m_shaderType = type;
	}

	void initShader(const Attrs &shader_attrs, const Attrs &unif_attrs, uint32_t type = 0)
	{
		auto &shader = m_shaders.at(type);
		loadShader(shader, shader_attrs, unif_attrs);
	}

	void initShader(const char *path, const Attrs &shader_attrs, const Attrs &unif_attrs, uint32_t type = 0)
	{
		auto &shader = m_shaders.at(type);
		loadShader(shader, path, shader_attrs, unif_attrs);
	}

	template<class shape_t, class shape_tag_t = shapes::Shape::DefaultTag>
	void initArray(const shape_t &shape, std::vector<const char *> names)
	{
		assert(!names.empty());

		m_coms = shape.instructions();
		m_ccw = shape.faceWinding() == GL_CCW;
		m_restartIndex = shape.restartIndex();

		for (auto &name: names) {
			std::vector<float> datas;
			uint32_t dim = 0;
			const char *a_name = nullptr;

			if (strcmp(name, "position") == 0) {
				a_name = "a.a_position";
				dim = shape.positions(datas);
				setBoundingSphere(datas, dim);
			}
			else if (strcmp(name, "normal") == 0) {
				a_name = "a.a_normal";
				dim = shape.normals(datas);
			}
			else if (strcmp(name, "tangent") == 0) {
				a_name = "a.a_tangent";
				dim = shape.tangents(datas);
			}
			else if (strcmp(name, "bitangent") == 0) {
				a_name = "a.a_bitangent";
				dim = shape.bitangents(datas);
			}
			else if (strcmp(name, "texcoord") == 0) {
				a_name = "a.a_texcoord";
				dim = shape.texCoordinates(datas);
			}
			else if (strcmp(name, "material") == 0) {
				a_name = "a.a_material";
				dim = shape.materialNumbers(datas);
			}
			else {
				assert(0);
			}

			Attrs attrs = {
			        {"buffer_target", GL_ARRAY_BUFFER   },
			        {a_name,          dim               },
			        {"data",          datas.data()      },
			        {"nelem",         datas.size() / dim},
			};
			SpuArray::aux(attrs, &name - &names[0]);
		}
		aux_error(id() == 0, "no vertex found\n");

		auto indices = shape.indices(shape_tag_t());
		SpuArray::send(indices, -1, sizeof(indices[0]));

		Attrs set_attrs = {
		        {"shader_id", m_shaders.at(0).id()}, // for further attrib
		        {"restart",   m_restartIndex      },
		};
		SpuArray::set(set_attrs);
	}

	void useShader()
	{
		for (auto &shader: m_subShaders) {
			shader->use();
		}
		m_shaders.at(m_shaderType).use();
	}

	void draw(const callback_t &callback) override
	{
		assert(callback == nullptr);
		auto override_callback = [&](uint32_t id) -> std::pair<void *, uint32_t> {
			if (id < m_coms.size()) {
				auto &com = m_coms[id];
				return {&com, m_driver(com.flags) ? 1 : 0};
			}
			return {nullptr, 0};
		};

		SpuScopedRenderstate renderstate(true);
		renderstate.flags.ccw = m_ccw;
		renderstate.use();
		useShader();
		SpuArray::draw(override_callback);
	}

	void draw(
	        uint32_t mode, uint32_t first = 0, uint32_t count = 0, uint32_t instance_count = 1,
	        uint32_t target = 0, uint32_t base_vertex = 0, uint32_t base_instance = 0) override
	{
		SpuScopedRenderstate renderstate(true);
		renderstate.flags.ccw = m_ccw;
		renderstate.use();
		useShader();
		SpuArray::draw(mode, first, count, instance_count, target, base_vertex, base_instance);
	}

	const Sphere3f &boundingSphere() const { return m_boundingSphere; }

	std::vector<SpuShader> &getShaders() { return m_shaders; }
	SpuShader &getShader(int index) { return m_shaders.at(index); }
	SpuShader &getAShader()
	{
		assert(m_shaders.size() == 1);
		return m_shaders.at(0);
	}

	void setDriver(const std::function<bool(uint32_t)> &driver) { m_driver = driver; }

	void setMode(uint32_t mode)
	{
		if (mode == m_coms[0].mode) {
			aux_message(0, "setMode: redundant setMode (%s)\n", opengl_const(mode));
			assert(++m_setcount < 8);
		}
		for (auto &com: m_coms) {
			com.mode = mode;
		}
	}

	void setCount(uint32_t count)
	{
		if (count == m_coms[0].count) {
			aux_message(0, "setMode: redundant setCount (%d)\n", count);
			assert(++m_setcount < 8);
		}
		for (auto &com: m_coms) {
			com.count = count;
		}
	}

	void setInstanceCount(uint32_t instance_count)
	{
		for (auto &com: m_coms) {
			com.instance_count = instance_count;
		}
	}

protected:
	std::vector<SpuShader> m_shaders;
	std::vector<SpuShader *> m_subShaders;
	std::function<bool(uint32_t)> m_driver = [](uint32_t) noexcept { return 1; };
	std::vector<SpuCommand> m_coms;

	Sphere3f m_boundingSphere;
	uint32_t m_ccw = 0;
	int32_t m_restartIndex = 0;
	uint32_t m_shaderType = 0;
	uint32_t m_setcount = 0;

	void init(const Attrs &attrs) override { SpuArray::init(attrs); }

	void setBoundingSphere(const std::vector<float> &datas, uint32_t dim)
	{
		Range3f range;
		range.invalidate();
		for (auto i = 0u; i < datas.size(); i += dim) {
			auto point = ezero();
			for (auto j = 0u; j < dim; j++) {
				point.f[j] = datas[i + j];
			}
			range.expand(point);
		}
		auto center = range.center();
		auto radius = length(range.span()) * 0.5;
		m_boundingSphere = Sphere3f(center, radius);
	}
};

class EdgeArray : public Array {
public:
	std::vector<SpuCommand> m_shapeComs;
	std::vector<SpuCommand> m_edgeComs;

	template<class shape_t>
	void initArray(const shape_t &shape, std::vector<const char *> names = std::vector<const char *>())
	{
		Array::initArray(shape, names);
		m_shapeComs = m_coms;

		auto indices = shape.indices();
		m_edgeComs = shape.instructions(shapes::Shape::EdgesTag());

		for (auto &op: m_edgeComs) {
			op.first += indices.size();
		}
		auto edge_indices = shape.indices(shapes::Shape::EdgesTag());
		indices.insert(indices.end(), edge_indices.begin(), edge_indices.end());

		Array::send(indices.data(), indices.size(), -1, sizeof(indices[0]));
	}

	void useEdge(bool is_edge) { m_coms = is_edge ? m_edgeComs : m_shapeComs; }
};
}  // namespace spu::oglplus::shapes
