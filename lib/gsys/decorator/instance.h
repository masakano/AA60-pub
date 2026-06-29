//
// Instance :
//
#pragma once

#include <gsys/painter.h>
#include <gsys/shaders/default/ub_composition.us>

namespace spu::gs_decorator {
class Instance : public GsDecorator {
public:
	struct Bone {
		Vec4i index;
		Vec4f weight;
	};

	struct Morph {
		int32_t type;
		int32_t index;
		float weight;
	};
	UB_COMPOSITION ub_composition;

	Instance(GsPainter *painter, const Attrs &attrs);

	void sendBones(const std::vector<Bone> &bones);
	void sendMorphs(const std::vector<Morph> &morphs);
	void sendMorphTargets(const std::vector<Vec4f> &morph_targets);
	void sendBoneMatrices(const std::vector<Mat4f> &bone_matrices);

	Attrs uniforms() const override;
	void doUse(uint32_t id) override;
	void doRender() override;

protected:
	bool def_use_animation = false;
	bool def_use_morphing = false;
};
}  // namespace spu::gs_decorator
