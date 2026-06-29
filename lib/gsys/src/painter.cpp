//
// GsPainter :
//

//
// painter base class
//
#include <gsys/node.h>
#include <gsys/painter.h>
#include <gsys/canvas.h>
#include "gsys/drawcall.h"

namespace spu {

GsPainter::GsPainter(const char *name) : GsObject(name), m_drawcalls(1)
{
	m_range.invalidate();
	setProperty(e_render, 1);
	setProperty(e_flip, 0);
}

GsPainter::~GsPainter() { dispose(); }

void GsPainter::dispose()
{
	SpuArray::dispose();
	m_shaders.clear();
	m_vertices.clear();
	m_indices.clear();

	for (auto &decorator: m_decorators) {
		delete decorator;
	}

	m_decorators.clear();
	m_drawcalls.clear();
	GsObject::dispose();
}

void GsPainter::init(const Attrs &attrs)
{
	attrs.peek("painter", "deprecated");
	attrs.peek("array_id", "deprecated");

	auto shader_id = attrs.get("shader_id", 0);
	m_isKeepInHost = attrs.get("keep_in_host", m_isKeepInHost);

	// parent
	aux_error(SpuArray::id(), "%s: duplicate init()\n", prettyName().c_str());
	GsObject::init(attrs);

	if (shader_id) {
		m_shaders["radiance"].reset(shader_id);
	}

	// shader
	std::map<hash32_t, const char *> shader_paths;
	for (auto &attr: attrs) {
		if (attrs.testAndLog(attr, Attr("path", nullptr), 4)) {
			auto attr_name = attr.key().c_str();
			if (attr_name[4] == '.') {
				shader_paths[hash32_t(attr_name + 5)] = attr;
			}
			else if (attr_name[4] == '\0') {  // just "path" only
				shader_paths[e_radiance] = attr;
				shader_paths[e_depth] = attr;
			}
		}
	}

	auto shader_attrs = attrs;
	auto *shadowmap = attrs.get<GsCanvas *>("shadowmap", nullptr);
	if (shadowmap) {
		shader_attrs += shadowmap->getShaderAttrs();
	}
	//shader_attrs.prepend("use_unif_block", true); // EXPERIMENTAL
	for (auto &shader_path: shader_paths) {
		auto &shader = m_shaders[shader_path.first];
		shader.init(shader_path.second, shader_attrs);
		if (shader_id == 0) {
			shader_id = shader.id();
		}
	}

	// array
	auto array_attrs = attrs;
	array_attrs.emplace_back("shader_id", shader_id);  // after "rewind"
	SpuArray::init(array_attrs);
}

bool GsPainter::hasShader(const hash32_t &type) const
{
	auto it = m_shaders.find(type);
	return it != std::end(m_shaders) && it->second.id();
}

void GsPainter::update() { GsObject::update(); }

void GsPainter::addUniforms(const Attrs &unif_attrs)
{
	for (auto &shader: m_shaders) {
		shader.second.addUniforms(unif_attrs);
	}
}

GsDrawcall &GsPainter::getADrawcall()
{
	assert(m_drawcalls.size() == 1);
	return m_drawcalls[0];
}

const GsDrawcall &GsPainter::getADrawcall() const
{
	assert(m_drawcalls.size() == 1);
	return m_drawcalls[0];
}

uint32_t GsPainter::instanceStride() const
{
	if (m_instanceStride == 0) {
		if (SpuArray::id()) {
			SpuArray::get("1.stride", &m_instanceStride);
		}
		if (m_instanceStride == 0) {
			m_instanceStride = sizeof(Mat4f);  // minumum sizeof(Mat4f)
		}
	}
	return m_instanceStride;
}

bool GsPainter::doSync(bool is_nonblock) { return getNode() ? getNode()->sync(is_nonblock) : false; }

void GsPainter::send() { aux_error(true, "%s: 'send()' not overloaded\n", prettyName().c_str()); }

void GsPainter::send(const std::vector<Mesh::Vertex> &, const std::vector<int32_t> &)
{
	aux_error(true, "%s: 'send()' not overloaded\n", prettyName().c_str());
}

void GsPainter::recv(std::vector<Mesh::Vertex> &, std::vector<int32_t> &) const
{
	aux_error(true, "%s: 'recv()' not overloaded\n", prettyName().c_str());
}

void GsPainter::recv(std::vector<uint8_t> &vertices, std::vector<int32_t> &indices) const
{
	uint32_t vertex_count = 0;
	uint32_t vertex_stride = 0;
	uint32_t index_count = 0;

	SpuArray::get("0.nelem", &vertex_count);
	SpuArray::get("0.stride", &vertex_stride);
	SpuArray::get("-1.nelem", &index_count);

	vertices.resize(vertex_count * vertex_stride);
	indices.resize(index_count);

	SpuArray::recv(vertices.data(), vertex_count, 0);
	SpuArray::recv(indices.data(), index_count, -1);
}

void GsPainter::draw(
        uint32_t mode, const std::vector<Mat4f> &nodeworlds, uint32_t first, uint32_t count,
        uint32_t instance_count, uint32_t target, uint32_t base_vertex, uint32_t base_instance)
{
	auto &coms = getDrawcalls().at(0).coms;
	coms.resize(1);
	coms[0].mode = mode;
	coms[0].first = first;
	coms[0].count = count;
	coms[0].instance_count = std::max<uint32_t>(nodeworlds.size(), instance_count);
	coms[0].target = target;
	coms[0].base_vertex = base_vertex;
	coms[0].base_instance = base_instance;
	GsPainter::render(nodeworlds);
}

void GsPainter::draw(
        uint32_t mode, uint32_t first, uint32_t count, uint32_t instance_count, uint32_t target,
        uint32_t base_vertex, uint32_t base_instance)
{
	draw(mode, {Mat4f()}, first, count, instance_count, target, base_vertex, base_instance);
}

void GsPainter::render(const void *instance_ptr, uint32_t instance_count)
{
	aux_error(instance_ptr == nullptr, "%s: no instance_ptr\n", prettyName().c_str());
	aux_error(SpuArray::id() == 0, "%s: draw() to uninitialized painter\n", prettyName().c_str());

	m_instancePtr = instance_ptr;
	m_instanceCount = instance_count;

	auto shader_type = m_shaderType.value() ? m_shaderType : GsCanvas::getCurrent()->getShaderType();
	if (m_shaders.find(shader_type) == std::end(m_shaders)) {  // not hasShader()
		aux_error(true, "shader '%s' not found\n", shader_type.c_str());
	}

	static constexpr std::pair<hash32_t, hash32_t> sub_shaders[] = {
	        {e_radiance, e_sub_radiance},
	        {e_depth,    e_sub_depth   },
	        {e_deferred, e_sub_deferred},
	};
	for (auto &sub_shader: sub_shaders) {
		if (shader_type == sub_shader.first && hasShader(sub_shader.second)) {
			coreRender(sub_shader.second);
		}
	}

	// depth prepass
	if (shader_type == e_radiance && getProperty(e_lazy) && hasShader(e_radiance) && hasShader(e_depth)
	    && m_shaders.at(e_radiance).id() != m_shaders.at(e_depth).id()) {
		coreRender(e_depth);
		auto drawcalls_save = getDrawcalls();
		for (auto &drawcall: getDrawcalls()) {
			drawcall.depth_func = GL_EQUAL;
		}
		coreRender(e_radiance);
		getDrawcalls() = drawcalls_save;
	}
	// else
	else {
		coreRender(shader_type);
	}
}

void GsPainter::coreRender(const hash32_t &shader_type)
{
	if (!hasShader(shader_type)) return;

	auto shader_type_save = m_shaderType;
	auto drawcalls_save = getDrawcalls();

	auto is_flip = getProperty(e_flip);

	m_shaderType = shader_type;
	m_callback = [&](uint32_t id) -> std::pair<void *, int32_t> {
		if (id >= getDrawcalls().size()) {
			return {nullptr, 0};
		}
		auto &drawcall = getDrawcalls()[id];

		if (is_flip) {
			drawcall.cull_face = drawcall.cull_face == GL_FRONT ? GL_BACK : GL_FRONT;
		}
		for (auto &decorator: m_decorators) {
			decorator->doUse(id);
		}
		doUse(id);

		auto &shader = m_shaders.at(m_shaderType);
		auto &coms = drawcall.coms;

		if (coms.empty()) {
			return {nullptr, 0};
		}
#if 1
		if (getDrawcalls().size() > 1 && coms.size() == 1 && coms[0].first == 0 && coms[0].count == 0) {
			return {coms.data(), 0};
		}
#endif
		if (shader.id() == 0 || drawcall.ub_material.invisible) {
			return {coms.data(), 0};
		}

		drawcall.use();
		shader.use();

		return {coms.data(), coms.size()};
	};

	// auto &props = properties();
	for (auto &decorator: m_decorators) {
		decorator->doRender();
	}
	if (getProperty(e_render)) {
		doRender();
	}
	if (getProperty(e_debug_render)) {
		doDebugRender();
	}
	m_shaderType = shader_type_save;
	getDrawcalls() = drawcalls_save;
}

void GsPainter::doRender() { SpuArray::draw(m_callback); }

void GsPainter::set(const Attrs &attrs)
{
	attrs.peek("node", "deprecated");
	attrs.subpeek({"array."}, "remove 'array.' prefix");
	attrs.subpeek({"u_"}, "use \"shader.\" prefix");

	for (auto &decorator: m_decorators) {
		decorator->set(attrs);
	}

	auto shader_attrs = attrs.select("shader.");
	for (auto &shader: m_shaders) {
		shader.second.setUniforms(shader_attrs);
	}
	SpuArray::set(attrs);
	GsObject::set(attrs);
}

void GsPainter::report(const char *str) const
{
	GsObject::report(str);
	aux_printf("\n");

	// drawcall (digest)
	{
		aux_printf("    drawcalls:\n");
		aux_printf(
		        "\t%3s %3s %3s %3s %3s %3s %6s %6s %6s %6s %9s %9s %s\n", "idx", "vis", "bl", "cf",
		        "ccw", "dt", "rough", "metal", "alpha", "psize", "first", "count", "albedo tex");

		auto idx = 0;
		for (const auto &drawcall: getDrawcalls()) {
			const auto &m = drawcall.ub_material;
			const auto &f = drawcall.flags;
			std::string albedo_texture = "-";
			if (drawcall.albedomap.id() != 0) {
				const char *signature = nullptr;
				// spu_inventory_get("texture", drawcall.u_albedomap,
				// "signature", &signature);
				drawcall.albedomap.get("signature", &signature);
				albedo_texture = extract_from_string(signature, ":").at(0);
			}

			for (auto &c: drawcall.coms) {
				aux_printf(
				        "\t%3d %3d %3d %3d %3d %3d %6.4f %6.4f %6.4f %6.4f %9d %9d %s\n", idx,
				        !m.invisible, f.blend, f.cull_face, f.ccw, f.depth_test, m.roughness,
				        m.metallic, m.min_alpha, m.point_size, c.first, c.count,
				        albedo_texture.c_str());
			}
			idx++;
		}
		aux_printf("\n");
	}

	// shader
	for (auto &pair: m_shaders) {
		auto &shader = pair.second;
		if (shader.id()) {
			const char *signature = nullptr;
			spu_inventory_get(shader.id(), "signature", &signature);
			aux_printf("    shader #%04x: %s\n", shader.id(), pretty_string(signature, 64).c_str());
			aux_printf("\t%-14s %-8s %s\n", "cpu addr", "gpu loc", "uniform");

			const auto &names = shader.names();
			const auto &locs = shader.locs();
			const auto &ptrs = shader.ptrs();
			for (auto i = 0u; i < names.size(); i++) {
				aux_printf("\t%p %08x %s\n", ptrs[i], locs[i], names[i].c_str());
			}
		}
		aux_printf("\n");
	}
	// array
	if (SpuArray::id()) {
		SpuArray::report("array");
	}
}
}  // namespace spu
