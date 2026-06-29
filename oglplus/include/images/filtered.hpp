//
//$<<Header>>$
//

#pragma once

#include <images/image.hpp>

namespace spu::oglplus::images {

class Filtered : public Image {
private:
	template<typename Filter, typename Sampler, typename Extractor>
	void calculate(const Image &input, Filter filter, Sampler sampler, Extractor extractor, float one)
	{
		sampler.setInput(input);
		auto p = this->begin<float>();
		auto w = input.width();
		auto h = input.height();
		auto d = input.depth();

		for (auto k = 0; k != d; ++k) {
			for (auto j = 0; j != h; ++j) {
				for (auto i = 0; i != w; ++i) {
					sampler.setOrigin(i, j, k);
					auto outv = filter(extractor, sampler, one);
					memcpy(p, outv.data(), outv.size() * sizeof(*outv.data()));
					p += outv.size();
				}
			}
		}
		assert(p == this->end<float>());
	}

public:
	struct DefaultFilter {
		template<typename Extractor, typename Sampler>
		Vec4f operator()(const Extractor &extractor, const Sampler &sampler, float one) const
		{
			return Vec4f(extractor(sampler(0, 0, 0)) * one);
		}
	};

	struct NoCoordTransform {
		void operator()(int32_t, int32_t, int32_t, uint32_t, uint32_t, uint32_t) const {}
	};

	class MatrixCoordTransform {
	private:
		Mat4f m_transf;

	public:
		MatrixCoordTransform(const Mat4f &transf) : m_transf(transf) {}

		void operator()(int32_t &x, int32_t &y, int32_t &z, double w, double h, double d) const
		{
			Vec4f in((x + 0.5) / w, (y + 0.5) / h, (z + 0.5) / d, 1);
			Vec4f out = m_transf * in;

			x = int32_t(out.x * w);
			y = int32_t(out.y * h);
			z = int32_t(out.z * d);
		}
	};

	struct RepeatSample {
		Vec4f operator()(
		        const Image &image, uint32_t width, uint32_t height, uint32_t depth, int32_t xpos,
		        int32_t ypos, int32_t zpos) const
		{
			if (xpos >= int(width)) {
				xpos %= width;
			}
			while (xpos < 0) {
				xpos += width;
			}

			if (ypos >= int(height)) {
				ypos %= height;
			}
			while (ypos < 0) {
				ypos += height;
			}

			if (zpos >= int(depth)) {
				zpos %= depth;
			}
			while (zpos < 0) {
				zpos += depth;
			}

			assert((xpos >= 0) && (xpos < int(width)));
			assert((ypos >= 0) && (ypos < int(height)));
			assert((zpos >= 0) && (zpos < int(depth)));

			return image.pixel(xpos, ypos, zpos);
		}
	};

	template<typename Transform, typename SampleFunc> class SamplerTpl {
	private:
		Transform m_transf;
		SampleFunc m_sample;

		const Image *m_image = nullptr;
		int32_t m_ori_x = 0, m_ori_y = 0, m_ori_z = 0;

	public:
		SamplerTpl(const Transform &transf = Transform(), const SampleFunc &sample = SampleFunc())
		        : m_transf(transf), m_sample(sample)
		{
		}

		void setInput(const Image &image) { m_image = &image; }

		void setOrigin(uint32_t x, uint32_t y, uint32_t z)
		{
			m_ori_x = int32_t(x);
			m_ori_y = int32_t(y);
			m_ori_z = int32_t(z);

			assert(m_image);

			m_transf(
			        m_ori_x, m_ori_y, m_ori_z, m_image->width(), m_image->height(),
			        m_image->depth());
		}

		Vec4f operator()(int32_t xoffs, int32_t yoffs, int32_t zoffs) const
		{
			assert(m_image != nullptr);
			return m_sample(
			        *m_image, m_image->width(), m_image->height(), m_image->depth(),
			        m_ori_x + xoffs, m_ori_y + yoffs, m_ori_z + zoffs);
		}
	};

	template<typename SampleFunc> struct SimpleSampler : SamplerTpl<NoCoordTransform, SampleFunc> {
		SimpleSampler(const SampleFunc &sample = SampleFunc())
		        : SamplerTpl<NoCoordTransform, SampleFunc>(NoCoordTransform(), sample)
		{
		}
	};

	struct DefaultSampler : SimpleSampler<RepeatSample> {};

	template<typename SampleFunc>
	struct MatrixTransformSampler : SamplerTpl<MatrixCoordTransform, SampleFunc> {
		MatrixTransformSampler(const Mat4f &transf, const SampleFunc &sample = SampleFunc())
		        : SamplerTpl<MatrixCoordTransform, SampleFunc>(MatrixCoordTransform(transf), sample)
		{
		}
	};

	struct FromRed {
		double operator()(const Vec4f &v) const { return v.f[0]; }
	};
	struct FromGreen {
		double operator()(const Vec4f &v) const { return v.f[1]; }
	};
	struct FromBlue {
		double operator()(const Vec4f &v) const { return v.f[2]; }
	};
	struct FromAlpha {
		double operator()(const Vec4f &v) const { return v.f[3]; }
	};
	struct FromRGB {
		vec3f_t operator()(const Vec4f &v) const { return v; }
	};
	struct FromRGBA {
		// PVec4f operator()(const Vec4f &v) const { return v; }
		Vec4f operator()(const Vec4f &v) const { return v; }
	};

	template<typename Filter, typename Sampler, typename Extractor>
	Filtered(const Image &input, Filter filter, Sampler sampler, Extractor extractor)
	        : Image(input.width(), input.height(), input.depth(), Vec4f::size(), (float *)nullptr)
	{
		calculate(input, filter, sampler, extractor, this->one(float(0)));
	}
};

class NormalMap : public Filtered {
public:
	struct NormalMapFilter {
		template<typename Extractor, typename Sampler>
		Vec4f operator()(const Extractor &extractor, const Sampler &sampler, float one) const
		{
			auto s = 0.05f;

			auto sc = extractor(sampler(0, 0, 0));
			auto spx = extractor(sampler(+1, 0, 0));
			auto spy = extractor(sampler(0, +1, 0));
			auto snx = extractor(sampler(-1, 0, 0));
			auto sny = extractor(sampler(0, -1, 0));

			auto vpx = Vec3f(+s, 0.0f, (spx - sc));
			auto vpy = Vec3f(0.0f, +s, (spy - sc));
			auto vnx = Vec3f(-s, 0.0f, (snx - sc));
			auto vny = Vec3f(0.0f, -s, (sny - sc));

			return Vec4f(normalize(
			                     cross(vpx, vpy) + cross(vpy, vnx) + cross(vnx, vny)
			                     + cross(vny, vpx)),
			             sc)
			     * one;
		}
	};

	NormalMap(const Image &image, FromRed) : Filtered(image, NormalMapFilter(), DefaultSampler(), FromRed())
	{
		m_format = GL_RGBA;
		m_internal = GL_RGBA16F;
	}

	NormalMap(const Image &image, FromAlpha)
	        : Filtered(image, NormalMapFilter(), DefaultSampler(), FromAlpha())
	{
		m_format = GL_RGBA;
		m_internal = GL_RGBA16F;
	}
};

class TransformComponents : public Filtered {
private:
	struct TransformFilter {
		Mat4f m_matrix;

		TransformFilter(const Mat4f &matrix) : m_matrix(matrix) {}

		template<typename Extractor, typename Sampler>
		Vec4f operator()(const Extractor &extractor, const Sampler &sampler, float one) const
		{
			const auto c = Vec4f(Vec3f(extractor(sampler(0, 0, 0))), 1.0);
			return m_matrix * c * one;
		}
	};

public:
	TransformComponents(const Image &input, const Mat4f &matrix)
	        : Filtered(input, TransformFilter(matrix), DefaultSampler(), FromRGB())
	{
		m_format = GL_RGB;
		m_internal = GL_RGB;
	}
};
}  // namespace spu::oglplus::images
