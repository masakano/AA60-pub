//
// MultisampleControl :
//
#pragma once
#include <spu++/spu++.h>

namespace spu {

class MultisampleControl {
public:
	MultisampleControl(uint32_t frame_id, const SpuGesture *gesture)
	{
		spu_printf(0, "hit \"l\" to toggle multipsample (%d)\n", ms_isMultisample);

		SpuRenderstate renderstate(true);
		m_prevContext = renderstate;

		if (gesture->pressed('l')) {
			ms_isMultisample = !ms_isMultisample;
		}

		if (ms_isMultisample) {
			renderstate.flags.multisample = true;
			renderstate.flags.sample_shading = true;
			renderstate.min_sample_shading = 1.0;
		}
		else {
			renderstate.flags.multisample = false;
			renderstate.flags.sample_shading = false;
		}
		renderstate.use();
		spu_frame_begin(frame_id);
		spu_frame_clear(frame_id);
	}

	~MultisampleControl()
	{
		spu_frame_end();
		m_prevContext.use();
	}

private:
	SpuRenderstate m_prevContext;
	static inline bool ms_isMultisample = true;
};

}  // namespace spu
