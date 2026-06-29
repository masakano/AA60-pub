//
//
//
#include <gsys/painter/ft_text.h>
#include "edtaa3func.h"
#include <cfloat>

namespace spu::gs_painter::freetype {

std::vector<double> make_distance_mapd(std::vector<double> &data, uint32_t width, uint32_t height)
{
	std::vector<short> xdist(width * height);
	std::vector<short> ydist(width * height);
	std::vector<double> gx(width * height);
	std::vector<double> gy(width * height);
	std::vector<double> outside(width * height);
	std::vector<double> inside(width * height);

	auto vmin = DBL_MAX;
	uint32_t i;

	// Compute outside = edtaa3(bitmap); % Transform background (0's)
	computegradient(data.data(), width, height, gx.data(), gy.data());
	edtaa3(data.data(), gx.data(), gy.data(), width, height, xdist.data(), ydist.data(), outside.data());
	for (auto i = 0u; i < width * height; ++i) {
		if (outside[i] < 0.0) outside[i] = 0.0;
	}

	// Compute inside = edtaa3(1-bitmap); % Transform foreground (1's)
	std::fill(begin(gx), end(gx), 0);
	std::fill(begin(gy), end(gy), 0);

	for (i = 0; i < width * height; ++i) {
		data[i] = 1 - data[i];
	}
	computegradient(data.data(), width, height, gx.data(), gy.data());
	edtaa3(data.data(), gx.data(), gy.data(), width, height, xdist.data(), ydist.data(), inside.data());
	for (i = 0; i < width * height; ++i) {
		if (inside[i] < 0) inside[i] = 0.0;
	}

	// distmap = outside - inside; % Bipolar distance field
	for (i = 0; i < width * height; ++i) {
		outside[i] -= inside[i];
		if (outside[i] < vmin) vmin = outside[i];
	}

	vmin = fabs(vmin);

	for (i = 0; i < width * height; ++i) {
		double v = outside[i];
		if (v < -vmin) {
			outside[i] = -vmin;
		}
		else if (v > +vmin) {
			outside[i] = +vmin;
		}
		data[i] = (outside[i] + vmin) / (2 * vmin);
	}
	return data;
}

std::vector<uint8_t> make_distance_mapb(const std::vector<uint8_t> &img, uint32_t width, uint32_t height)
{
	std::vector<double> data(width * height);
	std::vector<uint8_t> out(width * height);

	// find minimum and maximum values
	auto img_min = DBL_MAX;
	auto img_max = DBL_MIN;

	for (auto i = 0u; i < width * height; ++i) {
		auto v = img[i];
		data[i] = v;
		if (v > img_max) img_max = v;
		if (v < img_min) img_min = v;
	}

	// Map values from 0 - 255 to 0.0 - 1.0
	for (auto i = 0u; i < width * height; ++i) {
		data[i] = (img[i] - img_min) / img_max;
	}
	data = make_distance_mapd(data, width, height);

	// map values from 0.0 - 1.0 to 0 - 255
	for (auto i = 0u; i < width * height; ++i) {
		out[i] = (uint8_t)(255 * (1 - data[i]));
	}
	return out;
}
}  // namespace spu::gs_painter::freetype
