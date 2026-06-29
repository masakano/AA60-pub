//
// Buffer :
//
#include <utility>
#include "spu_array_buffer.h"
#include "spu_array_attrib.h"

namespace spu {
namespace libspu::spu_array {

struct SpuCommand {
	uint32_t count;
	uint32_t instance_count;
	uint32_t first;
	uint32_t base_vertex;
	uint32_t base_instance;
	uint32_t mode;
	uint32_t target;
	uint32_t flags;  // reserve (zero filled)
};

struct ArrayCommand {
	uint32_t count;
	uint32_t instance_count;
	uint32_t base_vertex;
	uint32_t base_instance;

	explicit ArrayCommand(const SpuCommand &c) noexcept
	        : count(c.count), instance_count(c.instance_count), base_vertex(c.first),
	          base_instance(c.base_instance)
	{
	}
};

struct IndexCommand {
	uint32_t count;
	uint32_t instance_count;
	uint32_t base_index;
	uint32_t base_vertex;
	uint32_t base_instance;

	explicit IndexCommand(const SpuCommand &c) noexcept
	        : count(c.count), instance_count(c.instance_count), base_index(c.first),
	          base_vertex(c.base_vertex), base_instance(c.base_instance)
	{
	}
};

enum DrawType {
	e_multi = 0,
	e_single,
};

enum DrawSlot {
	e_array = 0,
	e_index = -1,
	e_command = -2,
};

class VertexBuffer : public Buffer {
public:
	void bindAttrib()
	{
		if (id() && target() == GL_ARRAY_BUFFER) {
			bind();
			size_t ptr = 0;
			for (const auto &attrib: m_attribs) {
				attrib.bind(ptr, stride(), m_divisor);
				ptr += attrib.size();
			}
			unbind();
		}
	}

	void bindBuffer()
	{
		auto buffer_id = id();
		if (buffer_id && !m_attribs.empty()) {
			auto loc0 = m_attribs[0].loc();
			switch (target()) {
			case GL_ARRAY_BUFFER: return; return;
			case GL_SHADER_STORAGE_BUFFER:
				F(glBindBufferBase, GL_SHADER_STORAGE_BUFFER, loc0, buffer_id);
				return;
			default: assert(0);
			}
		}
	}
	void fixup(uint32_t shader_id)
	{
		for (auto &attrib: m_attribs) {
			attrib.fixup(shader_id);
		}
	}

	void report() const
	{
		if (id() == 0) {
			aux_printf("\tnelem : %d\n", nelem());
		}
		else {
			Buffer::report();
			aux_printf("\tdivisor : %d\n", m_divisor);
			aux_printf("\tattribs :\n");
			for (const auto &attrib: m_attribs) {
				if (&attrib == &m_attribs[0]) {
					Attrib::reportTitle();
				}
				attrib.report();
			}
		}
	}

	void alloc(const Attrs &attrs, uint32_t shader_id)
	{
		attrs.peek("iformat", "use 'format' instead");

		auto *buffer_ptr = (void *)nullptr;
		auto is_norm = false;
		auto buffer_nelem = 0;
		auto format = uint32_t(GL_FLOAT);
		auto oformat = uint32_t(GL_FLOAT);
		auto buffer_id = attrs.get("buffer_id", 0);
		auto buffer_target = attrs.get("buffer_target", 0u);
		auto is_probe = attrs.get("probe", 0);

		shader_id = attrs.get("shader_id", shader_id);  // override
		m_divisor = attrs.get("divisor", 0);

		auto stride = 0;
		m_attribs.clear();

		for (const auto &attr: attrs) {
			if (attrs.testAndLog(attr, Attr("data", buffer_ptr))) {
				buffer_ptr = attr;
			}
			else if (attrs.testAndLog(attr, Attr("nelem", buffer_nelem))) {
				buffer_nelem = int32_t(attr);
			}
			else if (attrs.testAndLog(attr, Attr("normalize", is_norm))) {
				is_norm = bool(attr);
			}
			else if (attrs.testAndLog(attr, Attr("format", format))) {
				format = uint32_t(attr);
			}
			else if (attrs.testAndLog(attr, Attr("oformat", oformat))) {
				oformat = uint32_t(attr);
			}
			else if (attrs.testAndLog(attr, Attr("a.", 0), 2)) {
				auto attr_key = attr.key().c_str();
				auto attr_value = int32_t(attr);
				attr_key += 2;
				for (auto &attrib: m_attribs) {
					aux_error(
					        attrib.name() == std::string(attr_key),
					        "%s: attrib redefined\n", attr_key);
				}

				Attrib attrib = {
				        attr_key, attr_value, int(attr_value * spu_gl_sizeof(format)),
				        format,   oformat,    is_norm,
				};

				auto new_target = attrib.fixup(shader_id);

				if (new_target != 0xffff) {
					if (buffer_target == 0) {
						buffer_target = new_target;
					}
					aux_error(buffer_target != new_target, "buffer target conflict\n");
				}

				if (shader_id == 0 && buffer_target == 0) {
					attrs.report("spu_array");
					aux_error(
					        true,
					        "need \"shader_id\" or \"buffer_target\" in slot#0 to resolve\n");
				}

				m_attribs.push_back(attrib);
				stride += attrib.size();
			}
		}
		if (is_probe) {
			return;
		}

		aux_error(buffer_nelem == 0 && m_attribs.empty(), "no elem, nor attributes, nor storage\n");

		if (!m_attribs.empty()) {
			Buffer::alloc(buffer_target, buffer_id, stride);
			send(buffer_ptr, buffer_nelem, stride);
			bindAttrib();
		}
		else {
			setStride(stride);
			setNelem(buffer_nelem);
		}
	}

	void draw(const SpuCommand &com)
	{
		F(glDrawArraysInstancedBaseInstance, com.mode, com.first, com.count, com.instance_count,
		  com.base_instance);
	}

	void setDivisor(int32_t divisor) { m_divisor = divisor; }

private:
	int32_t m_divisor = 0;
	std::vector<Attrib> m_attribs;
};

class IndexBuffer : public Buffer {
public:
	uint32_t getType()
	{
		switch (stride()) {
		case 1: return GL_UNSIGNED_BYTE;
		case 2: return GL_UNSIGNED_SHORT;
		case 4: return GL_UNSIGNED_INT;
		default: aux_error(true, "invalid index stride(%d)\n", stride());
		}
	}
	void send(const void *data, uint32_t nelem, uint32_t stride)
	{
		Buffer::send(data, nelem, stride, GL_ELEMENT_ARRAY_BUFFER);
	}

	void draw(const SpuCommand &com)
	{
		const void *gl_ptr = reinterpret_cast<const void *>(size_t(com.first) * stride());
		auto type = getType();
		F(glDrawElementsInstancedBaseVertexBaseInstance, com.mode, com.count, type, gl_ptr,
		  com.instance_count, com.base_vertex, com.base_instance);
	}

	void report() const
	{
		aux_printf("      element:\n");
		Buffer::report();
		aux_printf("\n");
	}
};

class CommandBuffer : public Buffer {
public:
	void draw(const SpuCommand &com, DrawSlot draw_slot, int32_t index_type)
	{
		aux_error(com.count <= 0, "invalid command count (%d)\n", com.count);

		auto array_stride = draw_slot == e_array ? sizeof(ArrayCommand) : sizeof(IndexCommand);
		auto total_size = nelem() * stride();
		auto nelem = total_size / array_stride;

		setStride(array_stride);
		setNelem(nelem);

		bind();

		if (draw_slot == e_array) {
			const void *gl_ptr = reinterpret_cast<const void *>(size_t(com.first) * array_stride);
			F(glMultiDrawArraysIndirect, com.mode, gl_ptr, com.count, array_stride);
		}
		else {
			const void *gl_ptr = reinterpret_cast<const void *>(size_t(com.first) * array_stride);
			F(glMultiDrawElementsIndirect, com.mode, index_type, gl_ptr, com.count, array_stride);
		}

		unbind();
	}

	void send(const void *data, int32_t nelem, int32_t stride)
	{
		Buffer::send(data, nelem, stride, GL_DRAW_INDIRECT_BUFFER);
	}

	void report() const
	{
		aux_printf("      command:\n");
		Buffer::report();
	}
};

class ArrayObject : public Object {
public:
	std::vector<VertexBuffer> m_vertices;
	IndexBuffer m_index;

	CommandBuffer m_command;
	uint32_t m_shaderId = 0;
	uint32_t m_patchVertices = 0;
	float m_patchInnerLevel[4] = {0, 0, 0, 0};
	float m_patchOuterLevel[4] = {0, 0, 0, 0};
	uint32_t m_baseVertex = 0;
	uint32_t m_baseInstance = 0;
	int32_t m_restartIndex = -1;

	explicit ArrayObject(const Attrs &attrs)
	{
		attrs.peek("parameter_id", "deprecated");  // reject legacy

		auto array_attrs = attrs.rewind();
		if (array_attrs.empty()) {
			F(glGenVertexArrays, 1, &m_handle.ui);  // for pure compute shader
			return;
		}
		auto prefix_index_size = array_attrs.pick("index_size", 4);
		auto prefix_index_id = array_attrs.pick("index_id", 0);

		m_shaderId = array_attrs.pick("shader_id", 0);
		m_restartIndex = array_attrs.pick("restart", -1);

		F(glGenVertexArrays, 1, &m_handle.ui);
		F(glBindVertexArray, m_handle.ui);

		m_vertices.resize(1);
		m_vertices[0].alloc(array_attrs, m_shaderId);

		if (prefix_index_id) {
			m_index.alloc(GL_ELEMENT_ARRAY_BUFFER, prefix_index_id, prefix_index_size);
		}
		auto prefix_command_id = array_attrs.pick("command_id", 0);
		if (prefix_command_id) {
			m_command.alloc(GL_DRAW_INDIRECT_BUFFER, prefix_command_id, 1);
		}
		F(glBindVertexArray, 0);
	}

	~ArrayObject() override
	{
		for (auto &v: m_vertices) {
			v.free();
		}

		m_index.free();
		m_command.free();
		F(glDeleteVertexArrays, 1, &m_handle.ui);
	}

	void aux(const Attrs &attrs, int32_t slot)
	{
		auto array_attrs = attrs.rewind();

		const std::vector<hash32_t> badargs = {
		        "index_size",
		        "index_id",
		        "command_id",
		};
		array_attrs.peek("badargs", "cannot use in aux slot");

		F(glBindVertexArray, m_handle.ui);

		if (slot >= int32_t(m_vertices.size())) {
			m_vertices.resize(slot + 1);
		}
		m_vertices[slot].alloc(array_attrs, m_shaderId);
		F(glBindVertexArray, 0);
	}

	void setupVAO()
	{
		F(glBindVertexArray, m_handle.ui);
		for (auto &v: m_vertices) {
			v.bindAttrib();
		}
		F(glBindVertexArray, 0);
	}

	void set(const Attrs &attrs)
	{
		attrs.peek("array_mask", "deprecated");

		auto shader_id = attrs.get("shader_id", 0);
		if (shader_id) {
			setShader(shader_id);
		}

		attrs.apply("base_vertex", m_baseVertex);
		attrs.apply("base_instance", m_baseInstance);
		attrs.apply("restart", m_restartIndex);
		attrs.apply("patch_vertices", m_patchVertices);

		auto *inner_level = attrs.get<float *>("patch_inner_level", nullptr);
		auto *outer_level = attrs.get<float *>("patch_outer_level", nullptr);

		if (inner_level) {
			memcpy(m_patchInnerLevel, inner_level, 16);
		}
		if (outer_level) {
			memcpy(m_patchOuterLevel, outer_level, 16);
		}

		static constexpr hash32_t divisor_keys[8] = {
		        "0.divisor", "1.divisor", "2.divisor", "3.divisor",
		        "4.divisor", "5.divisor", "6.divisor", "7.divisor",
		};
		assert(m_vertices.size() < 8);
		for (auto i = 0u; i < m_vertices.size(); i++) {
			auto divisor = attrs.getf<int32_t>(divisor_keys[i]);
			if (divisor.hit) {
				m_vertices[i].setDivisor(divisor.value);
				setupVAO();
				break;
			}
		}
	}

	void send(int32_t slot, const void *data, int32_t nelem, int32_t stride = 4)
	{
		if (slot == -2 && m_command.id() == 0) {
			m_command.alloc(GL_DRAW_INDIRECT_BUFFER, 0, 1);
		}

		switch (slot) {
		case DrawSlot::e_index: {
			m_index.send(data, nelem, stride);
			break;
		}
		case DrawSlot::e_command: {
			if (isIndexDraw(0)) {
				m_command.send(data, nelem, sizeof(IndexCommand));
			}
			else {
				m_command.send(data, nelem, sizeof(ArrayCommand));
			}
			break;
		}
		default:
			if (slot < 0 || slot >= int32_t(m_vertices.size())) {
				spu_message(0, "slot[%d] is invalid (do nothing)\n", slot);
			}
			else {
				m_vertices.at(slot).send(data, nelem, m_vertices.at(slot).stride());
			}
			break;
		}
	}

	void update(int32_t slot, const void *data, int32_t first, int32_t count)
	{
		switch (slot) {
		case DrawSlot::e_index: {
			m_index.update(data, first, count);
			break;
		}
		default: {
			if (slot < 0 || slot >= int32_t(m_vertices.size())) {
				spu_message(0, "slot[%d] is invalid (do nothing)\n", slot);
			}
			else {
				m_vertices.at(slot).update(data, first, count);
			}
			break;
		}
		}
	}

	void draw(const SpuCommand &com)  // no callback
	{
		beginDraw();
		coreDraw(&com, 1);
		endDraw();
	}

	void draw(const std::function<std::pair<void *, uint32_t>(uint32_t)> &callback)
	{
		std::pair<void *, int32_t> coms;
		auto is_first = true;
		for (auto id = 0; (coms = callback(id)).first; id++) {
			if (coms.second > 0) {
				// this must be after the first callback() (after first shader_use())
				if (is_first) {
					is_first = false;
					beginDraw();
				}
				coreDraw((SpuCommand *)coms.first, coms.second);
			}
		}
		endDraw();
	}

	Buffer *getBuffer(int32_t slot, bool is_abort = true)
	{
		switch (slot) {
		case DrawSlot::e_index: {
			return &m_index;
		}
		case DrawSlot::e_command: {
			return &m_command;
		}
		default: {
			int32_t size = m_vertices.size();
			if (slot >= 0 && slot < size) {
				return &m_vertices[slot];
			}
			aux_error(is_abort, "slot #%d out of range [0,%d)\n", slot, size);
			return nullptr;
		}
		}
	}
	void report() const
	{
		aux_printf("      common:\n");
		aux_printf("\tbase_verbex   : %d\n", m_baseVertex);
		aux_printf("\tbase_instance : %d\n", m_baseInstance);
		aux_printf("\trestart_index : %d\n", m_restartIndex);
		aux_printf("\n");

		aux_printf("      patches:\n");
		aux_printf("\tpatch_vertices    : %d\n", m_patchVertices);
		aux_printf("\tpatch_inner_level : %g,%g\n", m_patchInnerLevel[0], m_patchInnerLevel[1]);
		aux_printf(
		        "\tpatch_outer_level : %g,%g,%g,%g\n", m_patchOuterLevel[0], m_patchOuterLevel[1],
		        m_patchOuterLevel[2], m_patchOuterLevel[3]);
		aux_printf("\n");
		if (m_index.nelem() > 0) {
			m_index.report();
		}
		if (m_command.nelem() > 0) {
			m_command.report();
		}
		for (const auto &v: m_vertices) {
			aux_printf("      slot #%d:\n", int32_t(&v - &m_vertices[0]));
			v.report();
			aux_printf("\n");
		}
	}

private:
	void setShader(uint32_t shader_id)
	{
		if (shader_id && m_shaderId != shader_id) {
			m_shaderId = shader_id;
			aux_error(
			        m_shaderId == 0,
			        "cannot retrive shader_id to resolve attribute.\n"
			        "\tcall spu_shader_use() in advance, OR\n"
			        "\tset shader_id by 'spu_array_set({{\"shader_id\", shader_id}}))'\n");
			for (auto &v: m_vertices) {
				v.fixup(m_shaderId);
			}
			setupVAO();
		}
	}

	bool isCommandDraw(uint32_t target)
	{
		switch (target) {
		case GL_DRAW_INDIRECT_BUFFER: return true;
		case GL_ARRAY_BUFFER: return false;
		case GL_ELEMENT_ARRAY_BUFFER: return false;
		case 0: return m_command.nelem() > 0;
		default: aux_error(true, "invalid target (%04x)\n", target);
		}
	}

	bool isIndexDraw(uint32_t target)
	{
		switch (target) {
		case GL_ELEMENT_ARRAY_BUFFER: return true;
		case GL_ARRAY_BUFFER: return false;
		case GL_DRAW_INDIRECT_BUFFER:  // renderstate dependent
		case 0: return m_index.nelem() > 0;
		default: aux_error(true, "invalid target (%04x)\n", target);
		}
	}

	DrawSlot getDrawSlot(uint32_t target)
	{
		switch (target) {
		case GL_ARRAY_BUFFER: {
			return e_array;
		}
		case GL_ELEMENT_ARRAY_BUFFER: {
			aux_error(m_index.nelem() == 0, "specified slot (%s) is empty\n", opengl_const(target));
			return e_index;
		}
		case 0: {
			return m_index.nelem() > 0 ? e_index : e_array;
		}
		default: aux_error(true, "invalid slot (%04x)\n", target);
		}
	}

	void execDraw(const SpuCommand &com, DrawType draw_type)
	{
		auto draw_slot = getDrawSlot(com.target);

		if (com.mode == GL_PATCHES && m_patchVertices <= 0) {
			spu_message(0, "invalid patch_vertices. use default. (forget setting?)\n");
		}
		if (draw_type == e_single) {
			if (draw_slot == e_array) {
				m_vertices[0].draw(com);
			}
			else {
				m_index.draw(com);
			}
		}
		else {
			uint32_t index_type = m_index.id() != 0 ? m_index.getType() : 0;
			m_command.draw(com, draw_slot, index_type);
		}
	}

	void beginDraw()
	{
		if (m_patchVertices > 0) {
			F(glPatchParameteri, GL_PATCH_VERTICES, m_patchVertices);
		}
		auto is_zero4
		        = [](const float a[]) { return a[0] == 0 && a[1] == 0 && a[2] == 0 && a[3] == 0; };
		if (!is_zero4(m_patchInnerLevel)) {
			F(glPatchParameterfv, GL_PATCH_DEFAULT_INNER_LEVEL, m_patchInnerLevel);
		}

		if (!is_zero4(m_patchOuterLevel)) {
			F(glPatchParameterfv, GL_PATCH_DEFAULT_OUTER_LEVEL, m_patchOuterLevel);
		}
		setShader(spu_shader_use(-1));

		auto is_need_vao_bound = false;
		for (auto &v: m_vertices) {
			if (v.id() != 0) {
				v.bindBuffer();
				is_need_vao_bound = true;
			}
		}

		if (is_need_vao_bound) {
			F(glBindVertexArray, m_handle.ui);
		}

		if (m_index.id() != 0) {
			m_index.bind();
			if (m_restartIndex != -1) {
				F(glEnable, GL_PRIMITIVE_RESTART);
				F(glPrimitiveRestartIndex, m_restartIndex);
			}
		}
	}

	void coreDraw(const SpuCommand *coms, uint32_t com_count)
	{
		auto com = coms[0];  // copy
		if (com_count > 1) {
			if (isIndexDraw(com.target)) {
				std::vector<IndexCommand> index_coms;
				index_coms.reserve(com_count);
				for (auto i = 0u; i < com_count; i++) {
					index_coms.emplace_back(coms[i]);
				}
				m_command.send(index_coms.data(), index_coms.size(), sizeof(IndexCommand));
			}
			else {
				std::vector<ArrayCommand> array_coms;
				array_coms.reserve(com_count);
				for (auto i = 0u; i < com_count; i++) {
					array_coms.emplace_back(coms[i]);
				}
				m_command.send(array_coms.data(), array_coms.size(), sizeof(ArrayCommand));
			}
			com.first = 0;
			com.count = com_count;
			com.instance_count = 0;
			execDraw(com, e_multi);

			m_command.setNelem(0);
			m_command.setStride(0);
		}
		else if (com.mode == 0xffff) {
			F(glDispatchCompute, com.first, com.count, com.instance_count);
		}
		else if (isCommandDraw(com.target)) {
			if (com.first == 0 && com.count == 0) {
				com.count = m_command.nelem();
			}
			execDraw(com, e_multi);
		}
		else {
			if (m_baseVertex != 0 && com.base_vertex == 0) {
				com.base_vertex = m_baseVertex;
			}
			if (m_baseInstance != 0 && com.base_instance == 0) {
				com.base_instance = m_baseInstance;
			}
			if (com.first == 0 && com.count == 0) {
				com.count = isIndexDraw(com.target) ? m_index.nelem() : m_vertices[0].nelem();
			}
			execDraw(com, e_single);
		}
	}

	void endDraw()
	{
		F(glBindVertexArray, 0);
		if (m_index.id()) {
			m_index.unbind();
			if (m_restartIndex != -1) {
				F(glDisable, GL_PRIMITIVE_RESTART);
			}
		}
	}
};
class ArrayManager : public Manager<ArrayObject> {
public:
	uint32_t m_defaultId = 0;
	explicit ArrayManager(const char *name) : Manager<ArrayObject>(name, 0) {}

	void startup() override
	{
		Attrs init_attrs = {
		        {"nelem", 4},
		};
		m_defaultId = add(init_attrs).ui;
	}

	uint32_t handleToIndex(const Handle &handle) const override
	{
		if (handle.ui == 0) {
			return m_defaultId;
		}
		return handle.ui;
	}
};
ArrayManager s_objects("array");

}  // namespace libspu::spu_array

using namespace libspu;
using namespace libspu::spu_array;

uint32_t spu_array_new(const Attrs &attrs)
{
	SET();
	return s_objects.add(attrs).ui;
}

void spu_array_report(uint32_t array_id)
{
	SET();
	aux_printf("    array #%d:\n", array_id);
	s_objects.at(array_id)->report();
}

void spu_array_delete(uint32_t array_id)
{
	SET();
	s_objects.remove(array_id);
}

void spu_array_aux(uint32_t array_id, const Attrs &attrs, int32_t slot)
{
	SET();
	s_objects.at(array_id)->aux(attrs, slot);
}

void spu_array_set(uint32_t array_id, const Attrs &attrs)
{
	SET();
	s_objects.at(array_id)->set(attrs);
}

int32_t spu_array_get(uint32_t array_id, const hash32_t &key, void *value)
{
	GET_CHK(array_id, key);
	auto *array = s_objects.at(array_id);
	auto ret = 0;
	auto keystr = key.c_str();
	if (strchr("0123456789+-", keystr[0])) {
		auto *buffer = array->getBuffer(std::stoi(keystr), false);  // no abort
		if (buffer && buffer->id()) {
			auto dotpos = hash32_t(strchr(keystr, '.'));
			aux_error(dotpos == nullptr, "no attributed key=[%s]\n", key);

			if ((ret = getvalue(dotpos, value, ".buffer_id"_h32, buffer->id()))) {
				return ret;
			}
			if ((ret = getvalue(dotpos, value, ".stride"_h32, buffer->stride()))) {
				return ret;
			}
			if ((ret = getvalue(dotpos, value, ".nelem"_h32, buffer->nelem()))) {
				return ret;
			}
			return ret;
		}
	}
	if ((ret = getvalue(key, value, "patch_vertices"_h32, array->m_patchVertices))) {
		return ret;
	}
	return ret;
}

void spu_array_draw(uint32_t array_id, const std::function<std::pair<void *, uint32_t>(uint32_t)> &callback)
{
	SET();
	s_objects.at(array_id)->draw(callback);
}

void spu_array_draw(
        uint32_t array_id, uint32_t mode, uint32_t first, uint32_t count, uint32_t instance_count,
        uint32_t target, uint32_t base_vertex, uint32_t base_instance)
{
	SET();

	SpuCommand com = {
	        count, instance_count, first, base_vertex, base_instance, mode, target, 0,
	};
	s_objects.at(array_id)->draw(com);
}

void spu_array_send(uint32_t array_id, const void *data, uint32_t nelem, int32_t slot, uint32_t stride)
{
	SET();
	s_objects.at(array_id)->send(slot, data, nelem, stride);
}

void spu_array_update(uint32_t array_id, const void *data, uint32_t first, uint32_t count, int32_t slot)
{
	SET();
	s_objects.at(array_id)->update(slot, data, first, count);
}

void spu_array_recv(uint32_t array_id, void *data, uint32_t nelem, int32_t slot)
{
	SET();
	auto *buffer = s_objects.at(array_id)->getBuffer(slot);
	buffer->recv(data, nelem);
}

void *spu_array_map(uint32_t array_id, uint32_t access, int32_t slot)
{
	SET();
	auto *buffer = s_objects.at(array_id)->getBuffer(slot);
	return buffer->map(access);
}

void spu_array_unmap(uint32_t array_id, int32_t slot)
{
	SET();
	auto *buffer = s_objects.at(array_id)->getBuffer(slot);
	buffer->unmap();
}

void spu_array_copy(
        uint32_t dst_array_id, uint32_t src_array_id, int32_t dst_slot, int32_t src_slot, uint32_t dst_offset,
        uint32_t src_offset, uint32_t size)
{
	SET();
	switch (src_slot) {
	case DrawSlot::e_index: {
		aux_error(dst_slot != -1, "link between array and index\n");
		Buffer *src_buf = &s_objects.at(src_array_id)->m_index;
		Buffer *dst_buf = &s_objects.at(dst_array_id)->m_index;
		dst_buf->copy(src_buf, dst_offset, src_offset, size);
		return;
	}
	default: {
		aux_error(src_slot < 0, "unsupported slot (%d)\n", src_slot);
		Buffer *src_buf = &s_objects.at(src_array_id)->m_vertices.at(src_slot);
		Buffer *dst_buf = &s_objects.at(dst_array_id)->m_vertices.at(dst_slot);
		dst_buf->copy(src_buf, dst_offset, src_offset, size);
		return;
	}
	}
}

void spu_array_link(uint32_t dst_array_id, uint32_t src_array_id, int32_t dst_slot, int32_t src_slot)
{
	SET();
	aux_error(dst_slot == -1 && src_slot != -1, "invalid link between array and index\n");
	aux_error(src_slot == -1 && dst_slot != -1, "invalid link between array and index\n");

	auto *src_arrays = s_objects.at(src_array_id);
	auto *dst_arrays = s_objects.at(dst_array_id);

	auto *src_buf = src_arrays->getBuffer(src_slot);
	auto *dst_buf = dst_arrays->getBuffer(dst_slot);

	dst_buf->link(src_buf);

	if (dst_slot >= 0) {
		s_objects.at(dst_array_id)->setupVAO();
	}
}
}  // namespace spu
