//
// RenderstateObject :
//
#include "spu_object.h"
//#include <memory>  // need delete

#include "spu_renderstate_flags.h"
#include "spu_renderstate_lib.h"
#include "spu_renderstate_params.h"

namespace spu {
namespace libspu::spu_renderstate {

class RenderstateObject : public Object {
public:
	explicit RenderstateObject(const Attrs &attrs)
	{
		m_handle.ui = ms_pool.alloc();
		m_funcs = createParamsFuncs(attrs);
	}

	~RenderstateObject() override
	{
		for (auto &func: m_funcs) {
			delete func.f;
		}
		ms_pool.free(m_handle.ui);
	}

	bool use()
	{
		auto is_changed = false;
		for (auto &func: m_funcs) {
			if (func.f->is_deep()) {
				is_changed |= func.f->use(value_t(func.a));
			}
			else {
				is_changed |= func.f->use(value_t(*static_cast<uint32_t *>(func.a)));
			}
		}
		return is_changed;
	}

	void get() const
	{
		for (const auto &func: m_funcs) {
			if (func.f->is_deep()) {
				func.f->get(value_t(func.a));
			}
			else {
				// does not work in 64bit machine(?)
				*static_cast<uint32_t *>(func.a) = func.f->get(value_t(nullptr)).u;
			}
		}
	}

	void report() const
	{
		for (const auto &func: m_funcs) {
			func.f->report();
		}
	}

private:
	std::vector<ParamsFuncArg> m_funcs;
	static LocalPool ms_pool;
};

LocalPool RenderstateObject::ms_pool;
Manager<RenderstateObject> s_objects("renderstate", 0);

}  // namespace libspu::spu_renderstate

using namespace libspu;
using namespace libspu::spu_renderstate;

uint32_t spu_renderstate_new(const Attrs &attrs)
{
	SET();
	return s_objects.add(attrs).ui;
}

void spu_renderstate_delete(uint32_t renderstate_id)
{
	SET();
	s_objects.remove(renderstate_id);
}

void spu_renderstate_report(uint32_t renderstate_id)
{
	SET();
	// s_objects.at(renderstate_id)->get();
	s_objects.at(renderstate_id)->report();
}

void spu_renderstate_get(uint32_t renderstate_id)
{
	SET();
	s_objects.at(renderstate_id)->get();
}

bool spu_renderstate_use(uint32_t renderstate_id)
{
	SET();
	return s_objects.at(renderstate_id)->use();
}

}  // namespace spu
