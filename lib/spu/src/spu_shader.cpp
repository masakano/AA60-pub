//
// Manager :
//
#include "spu_shader_object.h"

namespace spu {
namespace libspu::spu_shader {

uint32_t ms_current_id = 0;

class ShaderManager : public Manager<ShaderObject> {
public:
	explicit ShaderManager(const char *name) : Manager<ShaderObject>(name, 0) {}

	void shutdown() override { F(glUseProgram, 0); }

	ShaderObject *at(uint32_t handle) override
	{
		if (handle == ~0u) {
			handle = ms_current_id;
		}
		return Manager<ShaderObject>::at(handle);
	}

	uint32_t handleToIndex(const Handle &handle) const override
	{
		if (handle.ui == ~0u) {
			return ms_current_id;
		}
		return handle.ui;
	}

	Handle add(const Attrs &attrs)  // not virtual
	{
		return Manager::add<ShaderObject>(attrs);
	}
};

ShaderManager s_objects("shader");
}  // namespace libspu::spu_shader

using namespace libspu;
using namespace libspu::spu_shader;

uint32_t spu_shader_new(const Attrs &attrs)
{
	SET();
	auto shader_id = s_objects.add(attrs).ui;
	return shader_id;
}

void spu_shader_delete(uint32_t shader_id)
{
	SET();
	if (shader_id == ms_current_id) spu_shader_use(0);
	if (shader_id) s_objects.remove(shader_id);
}

void spu_shader_report(uint32_t shader_id)
{
	SET();
	s_objects.at(shader_id)->report();
}

void spu_shader_loc(
        uint32_t shader_id, const char *names[], int32_t locs[], uint32_t sizes[], uint32_t types[], uint32_t n)
{
	SET();
	s_objects.at(shader_id)->getLocs(names, locs, sizes, types, n);
}

void spu_shader_set(uint32_t shader_id, const Attrs &attrs)
{
	SET();
	for (auto &attr: attrs) {
		s_objects.at(shader_id)->set(attr.key().c_str(), (const void *)attr);
	}
}

int32_t spu_shader_get(uint32_t shader_id, const hash32_t &key, void *value)
{
	GET_CHK(shader_id, key);
	return s_objects.at(shader_id)->get(key.c_str(), value) ? sizeof(void *) : 0;
}

uint32_t spu_shader_use(uint32_t shader_id, const int32_t locs[], const void *const ptrs[], uint32_t n)
{
	SET();
	if (shader_id == ~0u) {
		return ms_current_id;  // do nothing
	}
	if (shader_id == 0) {
		if (s_objects.isValid(ms_current_id)) {
			s_objects.at(ms_current_id)->clear();
		}
		F(glUseProgram, 0);
		return ms_current_id = shader_id;
	}

	if (shader_id != ms_current_id) {
		s_objects.at(shader_id)->use();
		ms_current_id = shader_id;
	}
	if (locs && n > 0) {
		s_objects.at(ms_current_id)->setUnif(locs, ptrs, n);
	}
	return ms_current_id;
}

}  // namespace spu
