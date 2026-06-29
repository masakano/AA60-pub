
#include <images/cell.hpp>
#include <lib/incl_begin.ipp>
#include <lib/incl_end.ipp>

namespace spu {
namespace oglplus {
namespace images {

struct VoronoiNearestPointColor {
	Vector3d operator()(const double *dists, const Vector3d *colors, int32_t count) const
	{
		auto md = 2.0;
		auto mc = 0;

		for (auto c = 0; c < count; ++c) {
			if (md > dists[c]) {
				md = dists[c];
				mc = c;
			}
		}
		return colors[mc];
	}
};

inline VoronoiDiagram::VoronoiDiagram(int32_t cell_w, int32_t cell_h, int32_t cell_d, const Image &input)
        : Image(static_cast<Image &&>(CellImageGen<u_char, 3>(
                  cell_w, cell_h, cell_d, input, CellImageGen<u_char, 3>::EulerDistance(),
                  VoronoiNearestPointColor())))
{
}

struct VoronoiCellDistance {
	double operator()(const std::vector<double> &d) const
	{
		assert(!d.empty());
		return d.front();
	}
};

inline VoronoiCells::VoronoiCells(int32_t cell_w, int32_t cell_h, int32_t cell_d, const Image &input)
        : Image(static_cast<Image &&>(WorleyCellGen(cell_w, cell_h, cell_d, input, VoronoiCellDistance(), 1)))
{
}

}  // namespace images
}  // namespace oglplus
}  // namespace spu
