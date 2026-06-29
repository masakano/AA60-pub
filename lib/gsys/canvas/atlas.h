//
// Atlas :
//
#pragma once

#include <gsys/canvas.h>
#include <gsys/node/gui/tweakbar.h>

namespace spu::gs_canvas {

/// MRT on single frame buffer
class Atlas : public GsCanvas {
public:
	explicit Atlas(const char *name = nullptr) : GsCanvas(name) {}
	explicit Atlas(const Attrs &attrs) : Atlas() { init(attrs); }

	void init(const Attrs &attrs) override;
	void subBegin(int32_t atlas_index, bool is_clear = false);
	void subEnd();
	Vec4f getScreentexc(uint32_t atlas_index) const;

	std::vector<Composition> &getLocalCompositions() { return m_localCompositions; }
	const std::vector<Composition> &getLocalCompositions() const { return m_localCompositions; }

	template<class T>
	void recv(std::vector<std::vector<T>> &datas, int32_t atlas_index, uint32_t format) const
	{
		constexpr hash32_t c_color_names[] = {
		        "color0", "color1", "color2", "color3", "color4", "color5", "color6", "color7",
		};

		// partial copy not supported
		auto &viewport = getViewports().at(0);
		std::vector<T> all_data(viewport.sx * viewport.sy);
		getBuffer(c_color_names[atlas_index]).recv(all_data.data(), format);

		for (const auto &local_composition: m_localCompositions) {
			auto i = &local_composition - &m_localCompositions[0];
			auto &lv = local_composition.getViewports().at(0);
			auto *p = datas.at(i).data();
			for (auto y = lv.oy; y < lv.oy + lv.sy; y++) {
				for (auto x = lv.ox; x < lv.ox + lv.sx; x++) {
					*p++ = all_data[y * viewport.sx + x];
				}
			}
		}
	}

protected:
	std::vector<Composition> m_localCompositions;
	Composition m_mainComposition;
};

class CubeAtlas : public Atlas {
public:
	struct Drawfunc {
		uint32_t cube_index;
		Vec3f center;
		float near;
		float far;
		uint32_t face_mask;
		std::function<void()> func;
		Drawfunc() : face_mask(~0u), func(nullptr) {}
	};

	explicit CubeAtlas(const char *name = nullptr) : Atlas(name) {}
	explicit CubeAtlas(const Attrs &attrs) : CubeAtlas() { init(attrs); }

	Drawfunc &getDrawfunc() { return m_drawfunc; }
	const Drawfunc &getDrawfunc() const { return m_drawfunc; }

	void init(const Attrs &attrs) override;
	void update() override;

	void cubeToAtlas(
	        uint32_t cube_index, const SpuTexture &cube_texture, uint32_t layer = 0,
	        const hash32_t &slot = "color0");

	void atlasToCube(
	        uint32_t cube_index, SpuTexture &cube_texture, uint32_t layer = 0,
	        const hash32_t &slot = "color0");

	void initCubeArrayTexture(
	        SpuTexture &texture, uint32_t layer_count = 0, const Attrs &aux_attrs = Attrs(),
	        const hash32_t &slot = "color0");

	uint32_t iformat(const hash32_t &slot = "color0") const;

	uint32_t cubeSize() const { return m_cubeSize; }
	uint32_t cubeCount() const { return m_cubeCount; }

private:
	Drawfunc m_drawfunc;
	uint32_t m_cubeCount = 0;
	uint32_t m_cubeSize = 0;
	uint32_t m_iformat = 0;

	void checkFormat(const SpuTexture &cube_texture) const;
};
}  // namespace spu::gs_canvas
