//
//$<<Header>>$
//

#pragma once

#include <images/image.hpp>

namespace spu::oglplus::images {

class NewtonFractal : public Image {
private:
	static Vec2f _cdiv(const Vec2f& a, const Vec2f& b)
	{
		auto d = dot(b, b);
		if (d < FLT_EPSILON) {
			d = FLT_EPSILON;
		}
		return Vec2f((a.x * b.x + a.y * b.y) / d, (a.y * b.x - a.x * b.y) / d);
	}

	template<typename T> static T mix(T a, T b, float coef) { return a * (1.0F - coef) + b * coef; }

	template<typename Function, typename Mixer, typename Vec>
	void _make(
	        int32_t width, int32_t height, Function, Mixer mixer, const Vec2f& lb, const Vec2f& rt,
	        const Vec& c1, const Vec& c2)
	{
		auto p = this->begin<float>();

		for (auto i = 0; i != width; ++i) {
			for (auto j = 0; j != height; ++j) {
				Vec2f z = {
				        mix(lb.x, rt.x, float(i) / float(width - 1)),
				        mix(lb.y, rt.y, float(j) / float(height - 1)),
				};

				auto max = 256;
				auto n = 0;
				for (n = 0; n != max; ++n) {
					auto zn = z - _cdiv(Function::f(z), Function::df(z));
					if (distance(zn, z) < 0.00001F) {
						break;
					}
					z = zn;
				}
				auto c = mix(c1, c2, mixer(float(n) / float(max - 1)));
				memcpy(p, c.data(), c.size() * sizeof(float));
				p += c.size();
			}
		}
		assert(p == this->end<float>());
	}

public:
	struct X3Minus1 {
		static Vec2f f(const Vec2f& n)
		{
			return Vec2f(
			        +n.x * n.x * n.x - 3.F * n.x * n.y * n.y - 1.F,
			        -n.y * n.y * n.y + 3.F * n.x * n.x * n.y);
		}

		static Vec2f df(const Vec2f& n) { return 3.0F * Vec2f(n.x * n.x - n.y * n.y, 2.0 * n.x * n.y); }
	};

	struct X4Minus1 {
		static Vec2f f(const Vec2f& n)
		{
			return Vec2f(
			        n.x * n.x * n.x * n.x + n.y * n.y * n.y * n.y - 6.F * n.x * n.x * n.y * n.y
			                - 1.F,
			        4.F * n.x * n.x * n.x * n.y - 4.F * n.x * n.y * n.y * n.y);
		}

		static Vec2f df(const Vec2f& n)
		{
			return 4.0F
			     * Vec2f(n.x * n.x * n.x - 3.F * n.x * n.y * n.y,
			             -n.y * n.y * n.y + 3.F * n.x * n.x * n.y);
		}
	};

	using DefaultFunction = X3Minus1;

	struct NoopMixer {
		template<typename T> T operator()(T value) const { return value; }
	};

	struct PowMixer {
		float m_exponent;

		PowMixer(float exponent) : m_exponent(exponent) {}

		template<typename T> T operator()(T value) const { return std::pow(value, T(m_exponent)); }
	};

	using DefaultMixer = NoopMixer;

	template<typename Function = DefaultFunction, typename Mixer = DefaultMixer>
	NewtonFractal(
	        int32_t width, int32_t height, Vec3f c1, Vec3f c2, Vec2f lb = Vec2f(-1.0F, -1.0F),
	        Vec2f rt = Vec2f(1.0F, 1.0F), Function func = Function(), Mixer mixer = Mixer())
	        : Image(width, height, 1, 3, static_cast<float*>(nullptr))
	{
		_make(width, height, func, mixer, lb, rt, c1, c2);
	}

	template<typename Function = DefaultFunction, typename Mixer = DefaultMixer>
	NewtonFractal(int32_t width, int32_t height, Function func = Function(), Mixer mixer = Mixer())
	        : Image(width, height, 1, 1, static_cast<float*>(nullptr))
	{
		_make(width, height, func, mixer, Vec2f(-1.0F, -1.0F), Vec2f(1.0F, 1.0F), 0.0F, 1.0F);
	}
};

}  // namespace spu::oglplus::images
