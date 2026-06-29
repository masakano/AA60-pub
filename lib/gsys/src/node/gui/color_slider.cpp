//
// ColorSlider :
//
#include "key_value.h"
#include <gsys/node/gui/color_slider.h>
#include <ssys/random_generator.h>
#include <smath/color_chart.h>
#include <type_traits>

namespace spu::gs_node::gui {

void ColorSlider::save(File &file) const
{
	KeyValue kv(this);
	for (auto lineno = 0u; lineno < m_auxStates.size(); lineno++) {
		auto &s = m_states[lineno];
		auto &as = m_auxStates[lineno];
		kv.add(lineno, "key", s.text);
		kv.add(lineno, "value", *as.rgb_ptr);
	}
	kv.save(file);
}

void ColorSlider::load(File &file)
{
	KeyValue kv(this);
	kv.load(file);

	for (auto lineno = 0u; lineno < m_states.size(); lineno++) {
		std::string line_key;
		kv.get(lineno, "key", line_key);
		for (auto i = 0u; i < m_states.size(); i++) {
			auto &s = m_states[i];
			auto key = peeloff_string(s.text);
			if (key == line_key) {
				auto &as = m_auxStates[i];
				kv.get(lineno, "value", *as.rgb_ptr);
				as.curr_hsv = rgb_to_hsv(*as.rgb_ptr);
				*s.value_ptr = as.curr_hsv.x;
			}
		}
	}
}

void ColorSlider::init(const Attrs &attrs)
{
	auto uint_dist = std::uniform_int_distribution<uint32_t>(0u, ~0u);
	RandomGenerator<uint32_t, std::uniform_int_distribution<uint32_t>> irand(uint_dist);

	auto *name = attrs.get("name", "color");
	auto *desc = attrs.get<Desc *>("desc", nullptr);
	auto *parent = attrs.get<Base *>("parent", nullptr);
	auto columns = float(getColumns(name, parent));

	assert(desc);
	auto rect_ofs_x = 0.0f;
	auto rect_size = 0.0f;
	m_auxStates.resize(desc->items.size());

	// slider
	{
		Slider::Desc slider_desc;
		for (auto i = 0u; i < desc->items.size(); i++) {
			auto &aux_state = m_auxStates[i];
			auto &item = desc->items[i];

			aux_state.rgb_ptr = desc->value_ptrs[i];
			aux_state.curr_hsv = rgb_to_hsv(*aux_state.rgb_ptr);

			rect_ofs_x = std::max(rect_ofs_x, float(strlen(item)));

			slider_desc.items.push_back(item);
			slider_desc.value_ptrs.push_back(&aux_state.curr_hsv.x);  // redirect hue
			slider_desc.value_minmaxs.push_back(Vec2f(0.0, 1.0));
			slider_desc.value_powers.push_back(1.0);
		}
		rect_ofs_x += 1;
		rect_size = columns - rect_ofs_x - 1;

		slider_desc.line_spacing = int32_t(rect_size / c_charheight) + 1;
		Attrs slider_attrs = {
		        {"name",   name        },
		        {"parent", parent      },
		        {"desc",   &slider_desc},
		};
		Slider::init(slider_attrs);
	}

	// h-texture
	{
		std::array<Vec4f, c_h_texture_size> pixels;
		for (auto i = 0; i < c_h_texture_size; i++) {
			auto hsv = Vec3f(float(i) / c_h_texture_size, 1.0f, 1.0f);
			pixels[i] = Vec4f(hsv_to_rgb(hsv), 1.0f);
		}

		Attrs attrs = {
		        {"target",  GL_TEXTURE_2D},
                        {"iformat", GL_RGBA32F   },
                        {"width",   256          },
                        {"height",  1            },
		        {"data",    pixels.data()},
		};
		m_hTexture.init(attrs);
	}

	// aux state
	auto rect_ofs_y = -3.0f;
	for (auto &aux_state: m_auxStates) {
		Attrs attrs = {
		        {"target",  GL_TEXTURE_2D    },
                        {"iformat", GL_RGBA32F       },
		        {"width",   c_sv_texture_size},
                        {"height",  c_sv_texture_size},
		        {"wrap_s",  GL_CLAMP_TO_EDGE },
                        {"wrap_t",  GL_CLAMP_TO_EDGE },
		};

		aux_state.sv_texture.init(attrs);
		sendPallete(aux_state);

		const auto h_rect_height = c_charheight * 0.75;
		const auto result_rect_width = (rect_ofs_x - c_charheight) * 0.75f;
		const auto result_rect_height = c_charheight * 2.0;

		aux_state.h_rect.ox = rect_ofs_x;
		aux_state.h_rect.oy = rect_ofs_y - h_rect_height;
		aux_state.h_rect.sx = rect_size;
		aux_state.h_rect.sy = h_rect_height;

		aux_state.sv_rect.ox = rect_ofs_x;
		aux_state.sv_rect.oy = rect_ofs_y - c_charheight - rect_size;
		aux_state.sv_rect.sx = rect_size;
		aux_state.sv_rect.sy = rect_size;

		aux_state.result_rect.ox = result_rect_width * 0.25;
		aux_state.result_rect.oy = rect_ofs_y - 2.0 - result_rect_height;
		aux_state.result_rect.sx = result_rect_width;
		aux_state.result_rect.sy = result_rect_height;

		rect_ofs_y -= aux_state.sv_rect.ox + c_charheight / 2;

		auto h_rect_hash = irand();
		newRect(h_rect_hash, aux_state.h_rect, m_hTexture);

		auto sv_rect_hash = irand();
		newRect(sv_rect_hash, aux_state.sv_rect, aux_state.sv_texture);

		aux_state.result_rect_hash = irand();
		newRect(aux_state.result_rect_hash, aux_state.result_rect, GsObject::defaultWhiteTexture());
	}
	Slider::update();
}

void ColorSlider::changeState(const std::vector<const void *> & /*ptrs*/, const hash32_t &state)
{
	std::vector<const void *> slider_ptrs;
	for (auto &aux_state: m_auxStates) {
		slider_ptrs.push_back((const void *)(&aux_state.curr_hsv.x));
	}
	Slider::changeState(slider_ptrs, state);
}

void ColorSlider::update()
{
	auto anchor_cursor = getCursor(getGesture()->anchorR());
	if (!isInside(anchor_cursor)) return;

	for (auto &graphics: getGraphics()) {
		for (auto &vertex: graphics.vertices) {
			vertex.c = m_states[0].state == e_disabled ? Vec4f(0.25f) : Vec4f(1.0f);
		}
	}

	for (auto i = 0u; i < m_states.size(); i++) {
		auto &state = m_states[i];
		auto &aux_state = m_auxStates[i];

		if (aux_state.curr_hsv.x != aux_state.prev_hsv.x) {
			sendPallete(aux_state);
		}

		auto *gesture = getGesture();
		if (state.state == e_active && !gesture->prev().mouse_L && gesture->curr().mouse_L) {
			pickPallete(aux_state);
		}

		aux_state.prev_hsv = aux_state.curr_hsv;
		auto rgb = hsv_to_rgb(aux_state.curr_hsv);
		if (aux_state.rgb_ptr) *aux_state.rgb_ptr = Vec4f(rgb, 1);
		getGraphic(hash32_t(aux_state.result_rect_hash)).vertices.at(0).c = Vec4f(rgb, 1);
	}
	Slider::update();
}

void ColorSlider::sendPallete(AuxState &aux_state)
{
	const auto c_gamma = 2.2f;
	std::array<std::array<Vec4f, c_sv_texture_size>, c_sv_texture_size> pixels;
	for (auto y = 0; y < c_sv_texture_size; y++) {
		for (auto x = 0; x < c_sv_texture_size; x++) {
			auto h = aux_state.curr_hsv.x;
			auto s = float(x) / c_sv_texture_size;
			auto v = powf(float(y) / c_sv_texture_size, c_gamma);
			pixels[y][x] = Vec4f(hsv_to_rgb(Vec3f(h, s, v)), 1.0f);
		}
	}
	aux_state.sv_texture.send(pixels[0].data(), GL_RGBA32F);
}

void ColorSlider::pickPallete(AuxState &aux_state)
{
	auto cursor = getCursor(getGesture()->curr().cursor);
	auto &r = aux_state.sv_rect;
	auto s = (cursor.x - r.ox) / r.sx;
	auto v = (cursor.y - r.oy) / r.sy;
	if (0.0f < s && s < 1.0f && 0.0f < v && v < 1.0f) {
		aux_state.curr_hsv.y = s;
		aux_state.curr_hsv.z = v;
		ms_lastUpdateCount = getSeconds().count();
	}
}

void ColorSlider::newRect(uint32_t hash_value, const Rectf &r, const SpuTexture &albedomap)
{
	auto &graphic = newGraphic(hash32_t(hash_value));

	auto cx = r.ox + r.sx / 2.0f;
	auto cy = r.oy + r.sy / 2.0f;

	gs_painter::Sprite::Vertex v;
	v.p = Vec3f(cx, cy, 0);
	v.s = Vec4f(-r.sx / 2, -r.sy / 2, +r.sx / 2, +r.sy / 2);
	v.c = Vec4f(1.0f);
	v.t = Vec4f(0, 0, 1, 1);
	graphic.vertices.push_back(v);

	graphic.drawcall.albedomap = albedomap;
	graphic.drawcall.flags.depth_test = false;
	graphic.drawcall.flags.blend = true;
}

}  // namespace spu::gs_node::gui
