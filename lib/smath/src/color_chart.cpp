//
// ColorPair :
//
#include <smath/color_chart.h>

namespace spu {
namespace {

struct ColorPair {
	const char *name;
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

ColorPair color_pairs[] = {
        {"aliceblue",            0xf0, 0xf8, 0xff},
        {"lightgreen",           0x90, 0xee, 0x90},
        {"antiquewhite",         0xfa, 0xeb, 0xd7},
        {"lightgrey",            0xd3, 0xd3, 0xd3},
        {"aqua",                 0x00, 0xff, 0xff},
        {"lightpink",            0xff, 0xb6, 0xc1},
        {"aquamarine",           0x7f, 0xff, 0xd4},
        {"lightsalmon",          0xff, 0xa0, 0x7a},
        {"azure",                0xf0, 0xff, 0xff},
        {"lightseagreen",        0x20, 0xb2, 0xaa},
        {"beige",                0xf5, 0xf5, 0xdc},
        {"lightskyblue",         0x87, 0xce, 0xfa},
        {"bisque",               0xff, 0xe4, 0xc4},
        {"lightslategray",       0x77, 0x88, 0x99},
        {"black",                0x00, 0x00, 0x00},
        {"lightsteelblue",       0xb0, 0xc4, 0xde},
        {"blanchedalmond",       0xff, 0xeb, 0xcd},
        {"lightyellow",          0xff, 0xff, 0xe0},
        {"blue",                 0x00, 0x00, 0xff},
        {"lime",                 0x00, 0xff, 0x00},
        {"blueviolet",           0x8a, 0x2b, 0xe2},
        {"limegreen",            0x32, 0xcd, 0x32},
        {"brass",                0xb5, 0xa6, 0x42},
        {"linen",                0xfa, 0xf0, 0xe6},
        {"brown",                0xa5, 0x2a, 0x2a},
        {"magenta",              0xff, 0x00, 0xff},
        {"burlywood",            0xde, 0xb8, 0x87},
        {"maroon",               0x80, 0x00, 0x00},
        {"cadetblue",            0x5f, 0x9e, 0xa0},
        {"mediumaquamarine",     0x66, 0xcd, 0xaa},
        {"chartreuse",           0x7f, 0xff, 0x00},
        {"mediumblue",           0x00, 0x00, 0xcd},
        {"chocolate",            0xd2, 0x69, 0x1e},
        {"mediumorchid",         0xba, 0x55, 0xd3},
        {"coolcopper",           0xd9, 0x87, 0x19},
        {"mediumpurple",         0x93, 0x70, 0xdb},
        {"copper",               0xbf, 0x00, 0xdf},
        {"mediumseagreen",       0x3c, 0xb3, 0x71},
        {"coral",                0xff, 0x7f, 0x50},
        {"mediumslateblue",      0x7b, 0x68, 0xee},
        {"cornflower",           0xbf, 0xef, 0xdf},
        {"mediumspringgreen",    0x00, 0xfa, 0x9a},
        {"cornflowerblue",       0x64, 0x95, 0xed},
        {"mediumturquoise",      0x48, 0xd1, 0xcc},
        {"cornsilk",             0xff, 0xf8, 0xdc},
        {"mediumvioletred",      0xc7, 0x15, 0x85},
        {"crimson",              0xdc, 0x14, 0x3c},
        {"midnightblue",         0x19, 0x19, 0x70},
        {"cyan",                 0x00, 0xff, 0xff},
        {"mintcream",            0xf5, 0xff, 0xfa},
        {"darkblue",             0x00, 0x00, 0x8b},
        {"mistyrose",            0xff, 0xe4, 0xe1},
        {"darkbrown",            0xda, 0x0b, 0x00},
        {"moccasin",             0xff, 0xe4, 0xb5},
        {"darkcyan",             0x00, 0x8b, 0x8b},
        {"navajowhite",          0xff, 0xde, 0xad},
        {"darkgoldenrod",        0xb8, 0x86, 0x0b},
        {"navy",                 0x00, 0x00, 0x80},
        {"darkgray",             0xa9, 0xa9, 0xa9},
        {"oldlace",              0xfd, 0xf5, 0xe6},
        {"darkgreen",            0x00, 0x64, 0x00},
        {"olive",                0x80, 0x80, 0x00},
        {"darkkhaki",            0xbd, 0xb7, 0x6b},
        {"olivedrab",            0x6b, 0x8e, 0x23},
        {"darkmagenta",          0x8b, 0x00, 0x8b},
        {"orange",               0xff, 0xa5, 0x00},
        {"darkolivegreen",       0x55, 0x6b, 0x2f},
        {"orangered",            0xff, 0x45, 0x00},
        {"darkorange",           0xff, 0x8c, 0x00},
        {"orchid",               0xda, 0x70, 0xd6},
        {"darkorchid",           0x99, 0x32, 0xcc},
        {"palegoldenrod",        0xee, 0xe8, 0xaa},
        {"darkred",              0x8b, 0x00, 0x00},
        {"palegreen",            0x98, 0xfb, 0x98},
        {"darksalmon",           0xe9, 0x96, 0x7a},
        {"paleturquoise",        0xaf, 0xee, 0xee},
        {"darkseagreen",         0x8f, 0xbc, 0x8f},
        {"palevioletred",        0xdb, 0x70, 0x93},
        {"darkslateblue",        0x48, 0x3d, 0x8b},
        {"papayawhip",           0xff, 0xef, 0xd5},
        {"darkslategray",        0x2f, 0x4f, 0x4f},
        {"peachpuff",            0xff, 0xda, 0xb9},
        {"darkturquoise",        0x00, 0xce, 0xd1},
        {"peru",                 0xcd, 0x85, 0x3f},
        {"darkviolet",           0x94, 0x00, 0xd3},
        {"pink",                 0xff, 0xc0, 0xcb},
        {"deeppink",             0xff, 0x14, 0x93},
        {"plum",                 0xdd, 0xa0, 0xdd},
        {"deepskyblue",          0x00, 0xbf, 0xff},
        {"powderblue",           0xb0, 0xe0, 0xe6},
        {"dimgray",              0x69, 0x69, 0x69},
        {"purple",               0x80, 0x00, 0x80},
        {"dodgerblue",           0x1e, 0x90, 0xff},
        {"red",                  0xff, 0x00, 0x00},
        {"feldsper",             0xfe, 0xd0, 0xe0},
        {"richblue",             0x0c, 0xb0, 0xe0},
        {"firebrick",            0xb2, 0x22, 0x22},
        {"rosybrown",            0xbc, 0x8f, 0x8f},
        {"floralwhite",          0xff, 0xfa, 0xf0},
        {"royalblue",            0x41, 0x69, 0xe1},
        {"forestgreen",          0x22, 0x8b, 0x22},
        {"saddlebrown",          0x8b, 0x45, 0x13},
        {"fuchsia",              0xff, 0x00, 0xff},
        {"salmon",               0xfa, 0x80, 0x72},
        {"gainsboro",            0xdc, 0xdc, 0xdc},
        {"sandybrown",           0xf4, 0xa4, 0x60},
        {"ghostwhite",           0xf8, 0xf8, 0xff},
        {"seagreen",             0x2e, 0x8b, 0x57},
        {"gold",                 0xff, 0xd7, 0x00},
        {"seashell",             0xff, 0xf5, 0xee},
        {"goldenrod",            0xda, 0xa5, 0x20},
        {"sienna",               0xa0, 0x52, 0x2d},
        {"gray",                 0x80, 0x80, 0x80},
        {"silver",               0xc0, 0xc0, 0xc0},
        {"green",                0x00, 0x80, 0x00},
        {"skyblue",              0x87, 0xce, 0xeb},
        {"greenyellow",          0xad, 0xff, 0x2f},
        {"slateblue",            0x6a, 0x5a, 0xcd},
        {"honeydew",             0xf0, 0xff, 0xf0},
        {"slategray",            0x70, 0x80, 0x90},
        {"hotpink",              0xff, 0x69, 0xb4},
        {"snow",                 0xff, 0xfa, 0xfa},
        {"indianred",            0xcd, 0x5c, 0x5c},
        {"springgreen",          0x00, 0xff, 0x7f},
        {"indigo",               0x4b, 0x00, 0x82},
        {"steelblue",            0x46, 0x82, 0xb4},
        {"ivory",                0xff, 0xff, 0xf0},
        {"tan",                  0xd2, 0xb4, 0x8c},
        {"khaki",                0xf0, 0xe6, 0x8c},
        {"teal",                 0x00, 0x80, 0x80},
        {"lavender",             0xe6, 0xe6, 0xfa},
        {"thistle",              0xd8, 0xbf, 0xd8},
        {"lavenderblush",        0xff, 0xf0, 0xf5},
        {"tomato",               0xff, 0x63, 0x47},
        {"lawngreen",            0x7c, 0xfc, 0x00},
        {"turquoise",            0x40, 0xe0, 0xd0},
        {"lemonchiffon",         0xff, 0xfa, 0xcd},
        {"violet",               0xee, 0x82, 0xee},
        {"lightblue",            0xad, 0xd8, 0xe6},
        {"wheat",                0xf5, 0xde, 0xb3},
        {"lightcoral",           0xf0, 0x80, 0x80},
        {"white",                0xff, 0xff, 0xff},
        {"lightcyan",            0xe0, 0xff, 0xff},
        {"whitesmoke",           0xf5, 0xf5, 0xf5},
        {"lightgoldenrodyellow", 0xfa, 0xfa, 0xd2},
        {"yellow",               0xff, 0xff, 0x00},
        {"yellowgreen",          0xff, 0xff, 0x00},
};
}  // namespace

Vec3f color_by_name(const char *name)
{
	if (name) {
		for (const auto &c: color_pairs) {
			if (strcmp(name, c.name) == 0) {
				return Vec3f(c.r, c.g, c.b) / 255.0;
			}
		}
	}
	for (const auto &c: color_pairs) {
		aux_printf("%02x %02x %02x %s\n", c.r, c.g, c.b, c.name);
	}
	aux_printf("%s: no such color\n", name);
	return eone<Vec4f>();
}

Vec3f linear_to_srgb(const Vec3f &linear)
{
	auto srgb = linear;
	for (auto i = 0; i < 3; i++) {
		auto &f = srgb.f[i];
		f = std::clamp(f, 0.0f, 1.0f);
		f = f < 0.0031308 ? 12.92 * f : (1.055 * powf(f, 1.0 / 2.4) - 0.055);
	}
	return srgb;
}

Vec3f srgb_to_linear(const Vec3f &srgb)
{
	auto linear = srgb;
	for (auto i = 0; i < 3; i++) {
		auto &f = linear.f[i];
		f = f <= 0.04045 ? (f * (1.0 / 12.92)) : powf((f + 0.055) * (1.0 / 1.055), 2.4);
	}
	return linear;
}

// Convert rgb floats ([0-1],[0-1],[0-1]) to hsv floats ([0-1],[0-1],[0-1]), from Foley & van Dam p592
// Optimized http://lolengine.net/blog/2013/01/13/fast-rgb-to-hsv
Vec3f rgb_to_hsv(const Vec3f &rgb)
{
	auto r = rgb.r;
	auto g = rgb.g;
	auto b = rgb.b;

	auto K = 0.0f;
	if (g < b) {
		std::swap(g, b);
		K = -1.f;
	}
	if (r < g) {
		std::swap(r, g);
		K = -2.0f / 6.0f - K;
	}
	const auto chroma = r - (g < b ? g : b);
	return {
	        std::abs(K + (g - b) / (60.0f * chroma + epsilon())),
	        chroma / (r + epsilon()),
	        r,
	};
}

// Convert hsv floats ([0-1],[0-1],[0-1]) to rgb floats ([0-1],[0-1],[0-1]), from Foley & van Dam p593
// also http://en.wikipedia.org/wiki/HSL_and_HSV
Vec3f hsv_to_rgb(const Vec3f &hsv)
{
	auto h = hsv.x;
	auto s = hsv.y;
	auto v = hsv.z;

	if (s == 0.0f) {
		// gray
		return Vec3f(v);
	}

	h = std::fmod(h, 1.0f) / (60.0f / 360.0f);

	auto i = (int)h;
	auto f = h - (float)i;
	auto p = v * (1.0f - s);
	auto q = v * (1.0f - s * f);
	auto t = v * (1.0f - s * (1.0f - f));

	switch (i) {
	case 0: return {v, t, p};
	case 1: return {q, v, p};
	case 2: return {p, v, t};
	case 3: return {p, q, v};
	case 4: return {t, p, v};
	case 5:
	default: return {v, p, q};
	}
}

}  // namespace spu
