//
//
//
#pragma once

namespace spu::sb6 {

#define SB6M_FOURCC(a, b, c, d) \
	((uint32_t(a) << 0) | (uint32_t(b) << 8) | (uint32_t(c) << 16) | (uint32_t(d) << 24))
#define SB6M_MAGIC SB6M_FOURCC('S', 'B', '6', 'M')

#pragma pack(push, 1)

typedef enum SB6M_CHUNK_TYPE_t {
	SB6M_CHUNK_TYPE_INDEX_DATA = SB6M_FOURCC('I', 'N', 'D', 'X'),
	SB6M_CHUNK_TYPE_VERTEX_DATA = SB6M_FOURCC('V', 'R', 'T', 'X'),
	SB6M_CHUNK_TYPE_VERTEX_ATTRIBS = SB6M_FOURCC('A', 'T', 'R', 'B'),
	SB6M_CHUNK_TYPE_SUB_OBJECT_LIST = SB6M_FOURCC('O', 'L', 'S', 'T'),
	SB6M_CHUNK_TYPE_COMMENT = SB6M_FOURCC('C', 'M', 'N', 'T')
} SB6M_CHUNK_TYPE;

typedef struct SB6M_HEADER_t {
	union {
		uint32_t magic;
		char magic_name[4];
	};
	uint32_t size;
	uint32_t num_chunks;
	uint32_t flags;
} SB6M_HEADER;

typedef struct SB6M_CHUNK_HEADER_t {
	union {
		uint32_t chunk_type;
		char chunk_name[4];
	};
	uint32_t size;
} SB6M_CHUNK_HEADER;

typedef struct SB6M_CHUNK_INDEX_DATA_t {
	SB6M_CHUNK_HEADER header;
	uint32_t index_type;
	uint32_t index_count;
	uint32_t index_data_offset;
} SB6M_CHUNK_INDEX_DATA;

typedef struct SB6M_CHUNK_VERTEX_DATA_t {
	SB6M_CHUNK_HEADER header;
	uint32_t data_size;
	uint32_t data_offset;
	uint32_t total_vertices;
} SB6M_CHUNK_VERTEX_DATA;

typedef struct SB6M_VERTEX_ATTRIB_DECL_t {
	char name[64];
	uint32_t size;
	uint32_t type;
	uint32_t stride;
	uint32_t flags;
	uint32_t data_offset;
} SB6M_VERTEX_ATTRIB_DECL;

#define SB6M_VERTEX_ATTRIB_FLAG_NORMALIZED 0x00000001
#define SB6M_VERTEX_ATTRIB_FLAG_INTEGER 0x00000002

typedef struct SB6M_VERTEX_ATTRIB_CHUNK_t {
	SB6M_CHUNK_HEADER header;
	uint32_t attrib_count;
	SB6M_VERTEX_ATTRIB_DECL attrib_data[1];
} SB6M_VERTEX_ATTRIB_CHUNK;

typedef struct SB6M_SUB_OBJECT_DECL_t {
	uint32_t first;
	uint32_t count;
} SB6M_SUB_OBJECT_DECL;

typedef struct SB6M_CHUNK_SUB_OBJECT_LIST_t {
	SB6M_CHUNK_HEADER header;
	uint32_t count;
	SB6M_SUB_OBJECT_DECL sub_object[1];
} SB6M_CHUNK_SUB_OBJECT_LIST;

typedef struct SB6M_CHUNK_COMMENT_t {
	SB6M_CHUNK_HEADER header;
	char comment[1];
} SB6M_CHUNK_COMMENT;
}  // namespace spu::sb6

#pragma pack(pop)
