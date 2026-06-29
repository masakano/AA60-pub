//
// PerlinNoisef : fractal brown noise generator
//
#include "vec.h"

namespace spu {

class PerlinNoisef {
public:
	PerlinNoisef();

	static float noisef(float x, float y, float z);
	static float noisef(float x, float y, float z, float w);

	static float noisef(const Vec3f &p) { return noisef(p.x, p.y, p.z); }
	static Vec3f noise3f(const Vec3f &p);

	template<class T> T fBm(const Vec3f &p, int32_t octaves = 4, float lacunarity = 2.0, float gain = 0.5)
	{
		float freq = 1.0;
		float amp = 0.5;
		T sum = T(0.0);
		for (auto i = 0; i < octaves; i++) {
			T value;
			noise(p * freq, value);
			sum += value * amp;
			freq *= lacunarity;
			amp *= gain;
		}
		return sum;
	}

	template<class T>
	T ridgefBm(
	        const Vec3f &p, int32_t octaves = 4, float lacunarity = 2.0, float gain = 0.5,
	        const T &offset = 1.0, float ridgeness = 1.0)
	{
		// Ridged is too ridgy.  So interpolate between ridge and fBm for the coarse
		// octaves. See Kenton Musgrave (2002). "Texturing and Modeling, Third Edition:
		// A Procedural Approach." these hardcoded constants make it look nice.

		T f0 = T(10) * fBm<float>(p, octaves, lacunarity, gain);
		T f1 = offset - std::abs(f0);
		return lerp(f0, f1, ridgeness);
	}

private:
	static void noise(const Vec3f &p, float &value) { value = noisef(p); }
	static void noise(const Vec3f &p, Vec3f &value) { value = noise3f(p); }
};
}  // namespace spu
