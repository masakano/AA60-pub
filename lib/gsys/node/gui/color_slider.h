//
// ColorSlider :
//
#pragma once
#include <gsys/node/gui/slider.h>

namespace spu::gs_node::gui {

class ColorSlider : public Slider {
public:
	struct Desc {
		std::vector<const char *> items;
		std::vector<Vec4f *> value_ptrs;
	};

	explicit ColorSlider(const char *name = nullptr) : Slider(name) {}
	explicit ColorSlider(const Attrs &attrs) : ColorSlider() { init(attrs); }
	void init(const Attrs &attrs) override;
	void update() override;
	void changeState(const std::vector<const void *> &ptrs, const hash32_t &state) override;
	void save(File &file) const override;
	void load(File &file) override;

private:
	static constexpr const int32_t c_h_texture_size = 256;
	static constexpr const int32_t c_sv_texture_size = 32;

	struct AuxState {
		Vec4f *rgb_ptr = nullptr;
		Vec3f curr_hsv = eone();
		Vec3f prev_hsv = eone();

		SpuTexture sv_texture;
		Rectf h_rect;
		Rectf sv_rect;
		Rectf result_rect;
		uint32_t result_rect_hash;
	};
	std::vector<AuxState> m_auxStates;
	SpuTexture m_hTexture;

	void newRect(uint32_t hash_value, const Rectf &r, const SpuTexture &albedomap);
	void sendPallete(AuxState &aux_state);
	void pickPallete(AuxState &aux_state);
};
}  // namespace spu::gs_node::gui
