//
// PerlinNoisef :
//
/*
  Ken Perlin's "Improved Noise"
  http://mrl.nyu.edu/~perlin/noise/
*/
#include <cmath>
#include <smath/perlin_noisef.h>

namespace spu {
namespace {

/* clang-format off */	
int32_t permutation[] = {
	151, 160, 137, 91,  90,  15,  131, 13,  201, 95,  96,  53,  194, 233, 7,   225, 140, 36,  103, 30,
	69,  142, 8,   99,  37,  240, 21,  10,  23,  190, 6,   148, 247, 120, 234, 75,  0,   26,  197, 62,
	94,  252, 219, 203, 117, 35,  11,  32,  57,  177, 33,  88,  237, 149, 56,  87,  174, 20,  125, 136,
	171, 168, 68,  175, 74,  165, 71,  134, 139, 48,  27,  166, 77,  146, 158, 231, 83,  111, 229, 122,
	60,  211, 133, 230, 220, 105, 92,  41,  55,  46,  245, 40,  244, 102, 143, 54,  65,  25,  63,  161,
	1,   216, 80,  73,  209, 76,  132, 187, 208, 89,  18,  169, 200, 196, 135, 130, 116, 188, 159, 86,
	164, 100, 109, 198, 173, 186, 3,   64,  52,  217, 226, 250, 124, 123, 5,   202, 38,  147, 118, 126,
	255, 82,  85,  212, 207, 206, 59,  227, 47,  16,  58,  17,  182, 189, 28,  42,  223, 183, 170, 213,
	119, 248, 152, 2,   44,  154, 163, 70,  221, 153, 101, 155, 167, 43,  172, 9,   129, 22,  39,  253,
	19,  98,  108, 110, 79,  113, 224, 232, 178, 185, 112, 104, 218, 246, 97,  228, 251, 34,  242, 193,
	238, 210, 144, 12,  191, 179, 162, 241, 81,  51,  145, 235, 249, 14,  239, 107, 49,  192, 214, 31,
	181, 199, 106, 157, 184, 84,  204, 176, 115, 121, 50,  45,  127, 4,   150, 254, 138, 236, 205, 93,
	222, 114, 67,  29,  24,  72,  243, 141, 128, 195, 78,  66,  215, 61,  156, 180
};

float g[][3] = {
	{1, 1, 0}, {-1,  1, 0}, {1, -1, 0}, {-1, -1,  0}, {1, 0, 1}, {-1,  0, 1}, { 1, 0, -1}, {-1,  0, -1},
	{0, 1, 1}, { 0, -1, 1}, {0, 1, -1}, { 0, -1, -1}, {1, 1, 0}, { 0, -1, 1}, {-1, 1,  0}, { 0, -1, -1},
};
/* clang-format on */

int32_t p[512];

float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }

float lerp(float t, float a, float b) { return a + t * (b - a); }

float dot(float a, float b, float c, float x, float y, float z) { return a * x + b * y + c * z; }

// use look-up table
float grad(int32_t hash, float x, float y, float z)
{
	auto h = hash & 15;
	return dot(g[h][0], g[h][1], g[h][2], x, y, z);
}

float grad(int32_t hash, float x, float y, float z, float w)
{
	// CONVERT LO 5 BITS OF HASH TO 32 GRAD DIRECTIONS. X,Y,Z
	// OR, DEPENDING ON HIGH ORDER 2 BITS:
	auto h = hash & 31;
	auto a = y;
	auto b = z;
	auto c = w;
	switch (h >> 3) {
	case 1:
		a = w;
		b = x;
		c = y;
		break;  // W,X,Y
	case 2:
		a = z;
		b = w;
		c = x;
		break;  // Z,W,X
	case 3:
		a = y;
		b = z;
		c = w;
		break;  // Y,Z,W
	}
	return ((h & 4) == 0 ? -a : a) + ((h & 2) == 0 ? -b : b) + ((h & 1) == 0 ? -c : c);
}
}  // namespace

PerlinNoisef::PerlinNoisef()
{
	// need seed
	for (auto i = 0; i < 256; i++) {
		p[256 + i] = p[i] = permutation[i];
	}
}

float PerlinNoisef::noisef(float x, float y, float z)
{
	// FIND UNIT CUBE THAT CONTAINS POINT.
	auto X = int32_t(std::floor(x)) & 255;
	auto Y = int32_t(std::floor(y)) & 255;
	auto Z = int32_t(std::floor(z)) & 255;

	// FIND RELATIVE X,Y,Z OF POINT IN CUBE.
	x -= std::floor(x);
	y -= std::floor(y);
	z -= std::floor(z);

	// COMPUTE FADE CURVES FOR EACH OF X,Y,Z.
	auto u = fade(x);
	auto v = fade(y);
	auto w = fade(z);

	// HASH COORDINATES OF THE 8 CUBE CORNERS,
	auto A = p[X + 0] + Y;
	auto AA = p[A + 0] + Z;
	auto AB = p[A + 1] + Z;
	auto B = p[X + 1] + Y;
	auto BA = p[B + 0] + Z;
	auto BB = p[B + 1] + Z;

	// AND ADD BLENDED RESULTS FROM 8 CORNERS OF CUBE
	return lerp(
	        w,
	        lerp(v, lerp(u, grad(p[AA], x + 0, y + 0, z + 0), grad(p[BA], x - 1, y + 0, z + 0)),
	             lerp(u, grad(p[AB], x + 0, y - 1, z + 0), grad(p[BB], x - 1, y - 1, z + 0))),
	        lerp(v, lerp(u, grad(p[AA + 1], x + 0, y + 0, z - 1), grad(p[BA + 1], x - 1, y + 0, z - 1)),
	             lerp(u, grad(p[AB + 1], x + 0, y - 1, z - 1), grad(p[BB + 1], x - 1, y - 1, z - 1))));
}

// vector noise
Vec3f PerlinNoisef::noise3f(const Vec3f &p)
{
	return {noisef(p), noisef(p + Vec3f(32, 78, 7)), noisef(p + Vec3f(123, 11, 96))};
}

float PerlinNoisef::noisef(float x, float y, float z, float w)
{
	// FIND UNIT HYPERCUBE THAT CONTAINS POINT.
	auto X = int32_t(std::floor(x)) & 255;
	auto Y = int32_t(std::floor(y)) & 255;
	auto Z = int32_t(std::floor(z)) & 255;
	auto W = int32_t(std::floor(w)) & 255;

	// FIND RELATIVE X,Y,Z,W OF POINT IN CUBE.
	x -= std::floor(x);
	y -= std::floor(y);
	z -= std::floor(z);
	w -= std::floor(w);

	// COMPUTE FADE CURVES FOR EACH OF X,Y,Z,W.
	auto a = fade(x);
	auto b = fade(y);
	auto c = fade(z);
	auto d = fade(w);

	// HASH COORDINATES OF THE 16 CORNERS OF THE HYPERCUBE.
	auto A = p[X + 0] + Y;
	auto AA = p[A + 0] + Z;
	auto AB = p[A + 1] + Z;
	auto B = p[X + 1] + Y;
	auto BA = p[B + 0] + Z;
	auto BB = p[B + 1] + Z;

	auto AAA = p[AA + 0] + W;
	auto AAB = p[AA + 1] + W;
	auto ABA = p[AB + 0] + W;
	auto ABB = p[AB + 1] + W;
	auto BAA = p[BA + 0] + W;
	auto BAB = p[BA + 1] + W;
	auto BBA = p[BB + 0] + W;
	auto BBB = p[BB + 1] + W;

	// INTERPOLATE DOWN.
	// clang-format off
	return lerp(
	        d,
	        lerp(c,
		     lerp(b,
			  lerp(a,
			       grad(p[AAA], x + 0, y + 0, z + 0, w + 0),
			       grad(p[BAA], x - 1, y + 0, z + 0, w + 0)),
	                  lerp(a,
			       grad(p[ABA], x + 0, y - 1, z + 0, w + 0),
			       grad(p[BBA], x - 1, y - 1, z + 0, w + 0))),

	             lerp(b,
			  lerp(a,
			       grad(p[AAB], x + 0, y + 0, z - 1, w + 0),
			       grad(p[BAB], x - 1, y + 0, z - 1, w + 0)),
	                  lerp(a,
			       grad(p[ABB], x + 0, y - 1, z - 1, w + 0),
			       grad(p[BBB], x - 1, y - 1, z - 1, w + 0)))),

	        lerp(c,
	             lerp(b,
	                  lerp(a,
			       grad(p[AAA + 1], x + 0, y + 0, z + 0, w - 1),
			       grad(p[BAA + 1], x - 1, y + 0, z + 0, w - 1)),
	                  lerp(a,
			       grad(p[ABA + 1], x + 0, y - 1, z + 0, w - 1),
			       grad(p[BBA + 1], x - 1, y - 1, z + 0, w - 1))),

	             lerp(b,
	                  lerp(a,
			       grad(p[AAB + 1], x + 0, y + 0, z - 1, w - 1),
			       grad(p[BAB + 1], x - 1, y + 0, z - 1, w - 1)),
	                  lerp(a,
			       grad(p[ABB + 1], x + 0, y - 1, z - 1, w - 1),
	                       grad(p[BBB + 1], x - 1, y - 1, z - 1, w - 1)))));
	// clang-format on
}
}  // namespace spu
