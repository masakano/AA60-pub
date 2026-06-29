//
//
//
#pragma once
#include <spu/GL/gl.h>
#include <spu/spu_pad.h>
#include <ssys/attrs.h>

namespace spu {

//
// system
//
void spu_graphics_memory_barrier(uint32_t mode);
void spu_graphics_init(const Attrs &attrs);
void spu_graphics_set(const Attrs &attrs);
int32_t spu_graphics_get(const hash32_t &key, void *value);
bool spu_graphics_swap();
void spu_graphics_report(const char *str);
void spu_graphics_shutdown();
std::vector<std::vector<uint32_t>> spu_graphics_get_alives_list();
void spu_graphics_prune(const std::vector<std::vector<uint32_t>> &servivors_list);
//
// invetory
//
uint32_t spu_inventory_new(const hash32_t &target, const char *path, const Attrs &attrs);
uint32_t spu_inventory_reuse(const hash32_t &target, const char *signature);
void spu_inventory_append(uint32_t inventry_id, const char *signature);

void spu_inventory_delete(uint32_t inventory_id);
int32_t spu_inventory_sync(uint32_t inventory_id, bool is_nonblock);
int32_t spu_inventory_get(uint32_t inventory_id, const hash32_t &key, void *value);
int32_t spu_inventory_sync_all(const hash32_t &target, bool is_nonblock);

//
// renderstate
//
uint32_t spu_renderstate_new(const Attrs &attrs);
void spu_renderstate_delete(uint32_t renderstate_id);
bool spu_renderstate_use(uint32_t renderstate_id);
void spu_renderstate_get(uint32_t renderstate_id);
void spu_renderstate_report(uint32_t renderstate_id);

//
// frame
//
uint32_t spu_frame_new(const Attrs &attrs);
void spu_frame_delete(uint32_t frame_id);
void spu_frame_set(uint32_t frame_id, const Attrs &attrs);

int32_t spu_frame_get(uint32_t frame_id, const hash32_t &key, void *value);
void spu_frame_report(uint32_t frame_id);
void spu_frame_begin(uint32_t frame_id);
void spu_frame_end();
void spu_frame_clear(uint32_t frame_id);

//
// array
//
uint32_t spu_array_new(const Attrs &attrs);
void spu_array_delete(uint32_t array_id);
void spu_array_set(uint32_t array_id, const Attrs &attrs);

int32_t spu_array_get(uint32_t array_id, const hash32_t &key, void *value);
void spu_array_report(uint32_t array_id);
void spu_array_aux(uint32_t array_id, const Attrs &attrs, int32_t slot);
void spu_array_send(uint32_t array_id, const void *data, uint32_t nelem, int32_t slot = 0, uint32_t stride = 4);
void spu_array_update(uint32_t array_id, const void *data, uint32_t first, uint32_t count, int32_t slot = 0);
void spu_array_recv(uint32_t array_id, void *data, uint32_t nelem, int32_t slot = 0);

void spu_array_copy(
        uint32_t dst_array_id, uint32_t src_array_id, int32_t dst_slot = 0, int32_t src_slot = 0,
        uint32_t dst_offset = 0, uint32_t src_offset = 0, uint32_t size = 0);
void spu_array_link(uint32_t dst_array_id, uint32_t src_array_id, int32_t dst_slot = 0, int32_t src_slot = 0);
void *spu_array_map(uint32_t array_id, uint32_t access, int32_t slot = 0);
void spu_array_unmap(uint32_t array_id, int32_t slot = 0);
void spu_array_draw(uint32_t array_id, const std::function<std::pair<void *, uint32_t>(uint32_t)> &callback);
void spu_array_draw(
        uint32_t array_id, uint32_t mode, uint32_t first = 0, uint32_t count = 0, uint32_t instance_count = 1,
        uint32_t target = 0, uint32_t base_vertex = 0, uint32_t base_instance = 0);
//
// shader
//
uint32_t spu_shader_new(const Attrs &attrs);
void spu_shader_delete(uint32_t shader_id);
void spu_shader_report(uint32_t shader_id);
void spu_shader_set(uint32_t shader_id, const Attrs &attrs);
int32_t spu_shader_get(uint32_t shader_id, const hash32_t &key, void *value);
void spu_shader_loc(
        uint32_t shader_id, const char *names[], int32_t locs[], uint32_t sizes[], uint32_t types[],
        uint32_t n);
uint32_t spu_shader_use(
        uint32_t shader_id, const int32_t locs[] = nullptr, const void *const ptrs[] = nullptr, uint32_t n = 0);

//
// texture
//
uint32_t spu_texture_new(const Attrs &attrs);
void spu_texture_delete(uint32_t texture_id);
void spu_texture_set(uint32_t texture_id, const Attrs &attrs);

int32_t spu_texture_get(uint32_t texture_id, const hash32_t &key, void *value);
void spu_texture_report(uint32_t texture_id);
void spu_texture_update(uint32_t texture_id);
void spu_texture_save(uint32_t texture_id, const char *path, const hash32_t &target, uint32_t level = 0);
void spu_texture_send(
        uint32_t texture_id, const void *pix, uint32_t pformat, const int32_t loc[4] = nullptr,
        const uint32_t size[4] = nullptr, bool is_clear = false, uint32_t raw_pformat = 0,
        uint32_t raw_ptype = 0);
void spu_texture_recv(
        uint32_t texture_id, void *pix, uint32_t pformat, const int32_t loc[4] = nullptr,
        const uint32_t size[4] = nullptr, uint32_t raw_pformat = 0, uint32_t raw_ptype = 0);
void spu_texture_copy(
        uint32_t dst_texture_id, uint32_t src_texture_id, const int32_t dst_loc[4] = nullptr,
        const int32_t src_loc[4] = nullptr, const uint32_t size[4] = nullptr);
uint32_t spu_texture_alias(
        uint32_t texture_id, uint32_t alias_target, uint32_t alias_format = 0, uint32_t depth = 0,
        uint32_t offset = 0);

//
// query
//
uint32_t spu_query_new(const Attrs &attrs);
void spu_query_delete(uint32_t query_id);
int32_t spu_query_get(uint32_t query_id, const hash32_t &key, void *value);
bool spu_query_begin(uint32_t query_id, uint32_t mode = 0);
uint64_t spu_query_end(uint32_t query_id, uint64_t *value, bool is_nonblock);
bool spu_query_sync(uint32_t query_id, uint64_t *value, bool is_nonblock);

//
// video
//
bool spu_video_enc_new(const Attrs &attrs);
bool spu_video_enc_delete();
bool spu_video_enc(const uint32_t *embed_value);
bool spu_video_dec_new(const Attrs &attrs);
bool spu_video_dec_delete();
bool spu_video_dec(uint32_t *embed_value);

//
// print
//
uint32_t spu_print_new(const Attrs &attrs);
void spu_print_delete(uint32_t print_id);
void spu_print_begin(uint32_t print_id);
void spu_print_end(uint32_t print_id);
void spu_print_set(uint32_t print_id, const Attrs &attrs);
int32_t spu_print_get(uint32_t print_id, const hash32_t &key, void *value);
int32_t spu_printf(uint32_t print_id, const char *fmt, ...);

//
// shortcuts
//
template<class T> void spu_graphics_set(const hash32_t &key, const T &value)
{
	spu_graphics_set({Attr(key, value)});
}
template<class T> void spu_frame_set(uint32_t frame_id, const hash32_t &key, const T &value)
{
	spu_frame_set(frame_id, {Attr(key, value)});
}
template<class T> void spu_array_set(uint32_t array_id, const hash32_t &key, const T &value)
{
	spu_array_set(array_id, {Attr(key, value)});
}
template<class T> void spu_texture_set(uint32_t texture_id, const hash32_t &key, const T &value)
{
	spu_texture_set(texture_id, {Attr(key, value)});
}
template<class T> void spu_print_set(uint32_t print_id, const hash32_t &key, const T &value)
{
	spu_print_set(print_id, {Attr(key, value)});
}

template<class T> void spu_shader_set(uint32_t shader_id, const hash32_t &key, const T &value)
{
	spu_shader_set(shader_id, {Attr(key, value)});
}

}  // namespace spu
