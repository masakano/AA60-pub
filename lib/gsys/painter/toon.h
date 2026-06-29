//
// Toon :
//
#include "pbr.h"

namespace spu::gs_painter {

class Toon : public PBR {
public:
	explicit Toon(const char *name = nullptr) : PBR(name) {}
	explicit Toon(const Attrs &attrs) : Toon() { init(attrs); }

	void init(const Attrs &attrs) override;
	void startInspector() override;

	Vec4f u_edge_color = eone<Vec4f>();
	float u_edge_width = 0.01;
	float u_edge_mix_rate = 0.5;
	float u_toon_ao = 0.5;
	float u_toon_threshold = 0.5;

protected:
	void doUse(uint32_t id) override;
};
}  // namespace spu::gs_painter
