//
// FrameObject :
//
#include "spu_frame_object.h"

namespace spu {
namespace libspu::spu_frame {

class FrameManager : public Manager<FrameObject> {
public:
	explicit FrameManager(const char *name) : Manager<FrameObject>(name, 0) {}

	void startup() override
	{
		SpuPad *pad;
		spu_graphics_get("pad", &pad);

		vec4f_t viewport = {0, 0, float(pad->winsize[0]), float(pad->winsize[1])};
		vec4f_t scissor = {0, 0, 0, 0};
		vec4f_t bgcolor = {0, 0, 0, 0};
		float bgdepth = 1.0;
		int32_t bgstencil = 0;

		Attrs init_attrs = {
		        {"frame_id",  0        },
                        {"viewport0", viewport },
                        {"scissor0",  scissor  },
		        {"bgcolor0",  bgcolor  },
                        {"bgdepth",   bgdepth  },
                        {"bgstencil", bgstencil},
		};
		add(init_attrs);
		at(0)->begin();
	}

	void shutdown() override
	{
		at(0)->end();
		F(glBindFramebuffer, GL_FRAMEBUFFER, 0);
	}

	FrameObject *at(uint32_t handle) override
	{
		if (handle == ~0u) {
			return FrameObject::top();
		}
		return Manager<FrameObject>::at(handle);
	}

	uint32_t handleToIndex(const Handle &handle) const override
	{
		if (handle.ui == ~0u) {
			return FrameObject::top()->handle().id;
		}
		return handle.id;
	}

	Handle add(const Attrs &attrs)  // not virutal
	{
		return Manager::add<FrameObject>(attrs);
	}

private:
};
FrameManager s_objects("frame");
}  // namespace libspu::spu_frame

using namespace libspu;
using namespace libspu::spu_frame;

uint32_t spu_frame_new(const Attrs &attrs)
{
	SET();
	return s_objects.add(attrs).ui;
}

void spu_frame_delete(uint32_t frame_id)
{
	assert(frame_id != ~0u);
	SET();
	if (frame_id) {
		FrameObject::erase(s_objects.at(frame_id));
		s_objects.remove(frame_id);
	}
}

void spu_frame_set(uint32_t frame_id, const Attrs &attrs)
{
	SET();
	s_objects.at(frame_id)->set(attrs);
}

int32_t spu_frame_get(uint32_t frame_id, const hash32_t &key, void *value)
{
	GET_CHK(frame_id, key);
	return s_objects.at(frame_id)->get(key, value);
}

void spu_frame_report(uint32_t frame_id)
{
	SET();
	s_objects.at(frame_id)->report();
}

void spu_frame_begin(uint32_t frame_id)
{
	SET();
	s_objects.at(frame_id)->begin();
}

void spu_frame_end() { FrameObject::top()->end(); }

void spu_frame_clear(uint32_t frame_id)
{
	SET();
	s_objects.at(frame_id)->clear();
}
}  // namespace spu
