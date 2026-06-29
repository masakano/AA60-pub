//
// Analizer :
//
#include <smath/vec.h>

namespace spu::gs_canvas::irradiance {

class Analizer {
public:
	Analizer(const std::vector<Vec3f> &pixels, uint32_t width, uint32_t height);
	const Vec3f &peakdir() const { return m_peakdir; }
	const Vec3f &emission() const { return m_emission; }
	const Vec3f &ambient() const { return m_ambient; }

private:
	const std::vector<Vec3f> &m_pixels;
	Vec3f m_peakdir;
	Vec3f m_emission;
	Vec3f m_ambient;
	uint32_t m_width;
	uint32_t m_height;

	void calcPeakdir();
	void calcEmissionAndAmbient();
};
}  // namespace spu::gs_canvas::irradiance
