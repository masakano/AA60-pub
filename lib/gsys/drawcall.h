//
// GsDrawcall :
//
#pragma once

#include "object.h"  // GsObject::ClassCreator
#include "canvas.h"
#include <gsys/shaders/default/ub_material.us>

namespace spu {

struct GsDrawcall : public SpuRenderstate {
public:
	static constexpr int32_t e_class_depth = 0;

	std::vector<SpuCommand> coms;
	SpuTexture albedomap;
	SpuTexture specularmap;
	SpuTexture emissionmap;
	SpuTexture armmap;
	SpuTexture normalmap;
	SpuTexture heightmap;
	SpuTexture brdfmap;
	SpuTexture lightmap;
	SpuTexture irradmap;

	UB_MATERIAL ub_material;

	GsDrawcall();

	void set(const Attrs &attrs);
	void report(const char *str) const;

	friend bool operator==(const GsDrawcall &d0, const GsDrawcall &d1);

	static void resetDefault();
	static void resetInitial();
	static void sort(std::vector<GsDrawcall> &drawcalls);

	static GsDrawcall *getInitial() { return ms_initial; }
	static GsDrawcall *getDefault() { return ms_default; }

private:
	void setMaterial(const Attrs &attrs);
	void setTextures(const Attrs &attrs);
	Attrs completeTexturePath(const char *preface, const char *ext = ".png") const;

	inline static GsDrawcall *ms_initial = nullptr;
	inline static GsDrawcall *ms_default = nullptr;

	friend struct GsObject::ClassCreator<GsDrawcall>;
	static void startup(const Attrs &attrs);
	static void shutdown();
	static GsObject::ClassCreator<GsDrawcall> ms_classCreator;
};
}  // namespace spu
