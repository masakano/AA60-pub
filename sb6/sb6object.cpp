//
// Object :
//
#include "sb6object.h"
namespace spu::sb6 {
namespace {
int32_t get_width(int32_t type)
{
	switch (type) {
	case GL_FLOAT: return 4;
	case GL_INT: return 4;
	case GL_HALF_FLOAT: return 2;
	default: aux_error(1, "sb6object: unknown type (%s)\n", opengl_const(type)); return 0;
	}
}
}  // namespace
void Object::load(const char *filename, const Attrs &aux_attrs, const char **sym, uint32_t shader_id)
{
	File file(filename, "rb");
	size_t filesize = file.size();
	std::vector<char> data(filesize);
	file.read(data.data(), data.size());

	char *ptr = data.data();
	SB6M_HEADER *header = reinterpret_cast<SB6M_HEADER *>(ptr);
	ptr += header->size;
	SB6M_VERTEX_ATTRIB_CHUNK *vertex_attrib_chunk = 0;
	SB6M_CHUNK_VERTEX_DATA *vertex_data_chunk = 0;
	SB6M_CHUNK_INDEX_DATA *index_data_chunk = 0;
	SB6M_CHUNK_SUB_OBJECT_LIST *sub_object_chunk = 0;
	for (auto i = 0u; i < header->num_chunks; i++) {
		SB6M_CHUNK_HEADER *chunk = reinterpret_cast<SB6M_CHUNK_HEADER *>(ptr);
		ptr += chunk->size;
		switch (chunk->chunk_type) {
		case SB6M_CHUNK_TYPE_VERTEX_ATTRIBS:
			vertex_attrib_chunk = reinterpret_cast<SB6M_VERTEX_ATTRIB_CHUNK *>(chunk);
			break;
		case SB6M_CHUNK_TYPE_VERTEX_DATA:
			vertex_data_chunk = reinterpret_cast<SB6M_CHUNK_VERTEX_DATA *>(chunk);
			break;
		case SB6M_CHUNK_TYPE_INDEX_DATA:
			index_data_chunk = reinterpret_cast<SB6M_CHUNK_INDEX_DATA *>(chunk);
			break;
		case SB6M_CHUNK_TYPE_SUB_OBJECT_LIST:
			sub_object_chunk = reinterpret_cast<SB6M_CHUNK_SUB_OBJECT_LIST *>(chunk);
			break;
		case SB6M_CHUNK_TYPE_COMMENT: /*printf("%s: skip comment chunk\n", filename);*/ break;
		default: aux_error(1, "%s: invalid header chunk\n", filename); break;
		}
	}
	if (sub_object_chunk != NULL) {
		if (sub_object_chunk->count > MAX_MODEL) {
			sub_object_chunk->count = MAX_MODEL;
		}
		for (auto i = 0u; i < sub_object_chunk->count; i++) {
			m_models[i] = sub_object_chunk->sub_object[i];
		}
		m_modelCount = sub_object_chunk->count;
	}
	else {
		m_models[0].first = 0;
		m_models[0].count = vertex_data_chunk->total_vertices;
		m_modelCount = 1;
	}
	// if (m_arrayId) spu_array_delete(m_arrayId);
	std::vector<int32_t> data_offset;
	std::vector<int32_t> data_width;

	const char *default_sym[] = {"a.0", "a.1", "a.2", "a.3", "a.4", "a.5", "a.6", "a.7", "a.8", "a.9"};
	if (sym == nullptr) {
		sym = default_sym;
	}

	std::vector<Attrs> attrs_v;
	for (auto i = 0u; i < 10 && i < vertex_attrib_chunk->attrib_count; i++) {
		SB6M_VERTEX_ATTRIB_DECL &attrib_decl = vertex_attrib_chunk->attrib_data[i];
		Attrs attrs;
		if (sym[i] == nullptr) break;

		attrs.emplace_back("shader_id", shader_id);
		attrs.emplace_back("format", attrib_decl.type);
		attrs.emplace_back(sym[i], attrib_decl.size);
		attrs_v.push_back(attrs);
		data_offset.push_back(attrib_decl.data_offset);
		data_width.push_back(attrib_decl.size * get_width(attrib_decl.type));
	}
	data_offset.push_back(vertex_data_chunk->data_size);
	for (auto i = 0u; i < data_offset.size() - 1; i++) {
		// delayed allocate
		if (data_offset[i + 1] - data_offset[i] > 0) {
			Attrs &attrs = attrs_v[i];
			if (i == 0) {
				//(attrs + aux_attrs).report("array attrs");
				m_array.init(attrs + aux_attrs);
			}
			else {
				m_array.aux(attrs, i);
			}
			m_array.send(
			        data.data() + (vertex_data_chunk->data_offset + data_offset[i]),
			        (data_offset[i + 1] - data_offset[i]) / data_width[i], i);
			m_slotCount = i + 1;
		}
	}
	if (index_data_chunk != NULL) {
		m_array.send(
		        data.data() + index_data_chunk->index_data_offset, index_data_chunk->index_count, -1,
		        index_data_chunk->index_type == GL_UNSIGNED_SHORT ? 2 : 1);
		m_indexCount = index_data_chunk->index_count;
	}
	else {
		m_indexCount = vertex_data_chunk->total_vertices;
	}
}

void Object::draw(uint32_t instance_count, uint32_t base_instance)
{
	if (base_instance != ~0u) {
		m_array.set("base_instance", base_instance);
	}
	if (instance_count == 1) {
		m_array.draw(GL_TRIANGLES, m_models[0].first, m_models[0].count, instance_count);
	}
	else {
		m_array.draw(GL_TRIANGLES, 0, 0, instance_count);
	}
}
}  // namespace spu::sb6
