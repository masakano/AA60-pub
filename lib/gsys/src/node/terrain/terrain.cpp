//
// Terrain :
//
#include "terrain_inspector.h"
#include <bit>  // bit_width()

namespace spu::gs_node {
void Terrain::init(const Attrs &attrs)
{
	const auto c_ndiv = 128;
	auto mesh_grid = Vec4i(c_ndiv, c_ndiv, 1, 1);
	auto mapnode = Mat4f().rot("x", pi() / 2).scale(2.0);
	auto maptexc = Mat4f();

	Attrs init_attrs = {
	        {"shape",     "plate"  },
	        {"mesh_grid", mesh_grid},
	        {"mapnode",   &mapnode },
	        {"maptexc",   &maptexc },
	};
	Manifold2D::init(init_attrs + attrs);
	setProperty("lazy", 0);
	setProperty("spread", 1);
}

void Terrain::update()
{
	updateHeight();
	updateSubstances();
	Manifold2D::update();
}


bool Terrain::doBuild(const std::vector<Mat4f> &nodeworlds)
{
	assert(nodeworlds.size() == 1);
	buildInstances(nodeworlds);

	auto base_instance = 0;
	std::vector<uint32_t> instance_counts(instances().size());
	std::vector<uint32_t> base_instances(instances().size());
	for (auto slot = 0u; slot < instances().size(); slot++) {
		auto instance_count = instances().at(slot).size() / substanceStride();
		instance_counts.at(slot) = instance_count;
		base_instances.at(slot) = base_instance;
		base_instance += instance_count;
	}

	if (base_instance == 0) {
		return false;  // clip
	}
	auto *painter = getPainter();
	if (painter) {
		for (auto &drawcall: painter->getDrawcalls()) {
			for (auto slot = 0u; slot < drawcall.coms.size(); slot++) {
				auto &com = drawcall.coms[slot];
				if (slot < base_instances.size()) {
					com.base_instance = base_instances.at(slot);
					com.instance_count = instance_counts.at(slot);
				}
			}
		}
	}
	return true;
}

#define APPLY(func, p) Vec3f(func((p).x), func((p).y), func((p).z))

Range3f Terrain::calcSpreadRange(const Mat4f &worldscreen) const
{
	if (!getProperty(e_spread)) {
		return Range3f(Vec3f(0), Vec3f(0));
	}

	Range3f spread_range;
	const auto range = getARange();
	const auto plane = Plane3f(Vec3f(0), Vec3f(0, 1, 0));

	std::vector<Vec3f> intersect_points;
	worldscreen.inside(plane, &intersect_points);

	auto intersect_range = Range3f(intersect_points);
	auto span = range.span();

	spread_range.p0 = APPLY(floorf, intersect_range.p0 / span) * span - span;
	spread_range.p1 = APPLY(ceilf, intersect_range.p1 / span) * span + span;

	return spread_range;
}


void Terrain::updateSubstances()
{
	auto *current = GsCanvas::getCurrent();
	const auto range = getARange();

	auto worldview = current->worldview(0);
	auto viewscreen = current->viewscreen();
	auto screenfrag = current->screenfrag(0);

	auto worldscreen = viewscreen * worldview;
	auto worldfrag = screenfrag * worldscreen;
	auto viewfrag = screenfrag * viewscreen;
	auto normal_view = worldview * Vec4f(0, 1, 0, 0);

	float far = calcFar(viewfrag, normal_view);
	viewscreen.set_projection(nullptr, nullptr, nullptr, &far);
	viewscreen.get_projection(nullptr, nullptr, nullptr, &far);

	worldscreen = viewscreen * worldview;
	worldfrag = screenfrag * worldscreen;
	viewfrag = screenfrag * viewscreen;

	auto span = range.span();
	auto spread_range = calcSpreadRange(worldscreen);

	std::vector<std::vector<Mat4f>> substances;
	substances.resize(8);

	for (auto z = spread_range.p0.z; z <= spread_range.p1.z; z += span.z) {
		for (auto x = spread_range.p0.x; x <= spread_range.p1.x; x += span.x) {
			auto center = Vec4f(x, 0, z, 1);
			if (worldscreen.intersect(Mat4f(range + Vec3f(center)))) {
				auto nodeworld = Mat4f().trans(center);
				auto nodefrag = worldfrag * nodeworld;
				auto level = calcLevel(nodefrag, 7);
				substances.at(level).push_back(nodeworld);
			}
		}
	}
	dupSubstances(8);
	for (auto slot = 0; slot < 8; slot++) {
		getSubstances(slot) = substances.at(slot);
	}
}

void Terrain::updateHeight()
{
	auto *painter = getPainter();
	if (painter == nullptr) return;

	auto &drawcall = painter->getADrawcall();
	auto &height_texture = drawcall.heightmap;


	if (height_texture.id() != m_updatedHeightmap && height_texture.sync(true) == 0) {
		int32_t width, height;
		height_texture.sync(false);
		height_texture.get("width", &width);
		height_texture.get("height", &height);
		std::vector<float> pixels(width * height);
		height_texture.recv(pixels.data(), GL_R32F);

		m_heightRange.invalidate();
		m_heightRange.expand(pixels);
		m_updatedHeightmap = height_texture.id();

		printf("heighRange=%f,%f\n", m_heightRange.p0, m_heightRange.p1);

	}

	auto height_scale = drawcall.ub_material.height_scale;
	auto &range = getARange();

	range.p0.y = m_heightRange.p0 * height_scale;
	range.p1.y = m_heightRange.p1 * height_scale;
}

float Terrain::calcFar(const Mat4f &viewfrag, const Vec3f &normal_view) const
{
	auto dx = viewfrag.c[0].x;
	auto dy = viewfrag.c[1].y;
	auto w = sqrtf(dx * dy);
	auto far = w * std::max(0.05f, std::abs(normal_view.z));
	return far;
}

int32_t Terrain::calcLevel(const Mat4f &nodefrag, int32_t max_level) const
{
	std::vector<Vec3f> points_node = {
	        {-1, 0, -1},
	        {+1, 0, -1},
	        {-1, 0, +1},
	        {+1, 0, +1},
	};
	auto p = nodefrag.pers3(points_node);
	auto area = length(cross(p[1] - p[0], p[2] - p[0])) + length(cross(p[3] - p[0], p[2] - p[0]));
	auto len = uint32_t(sqrtf(area));
	return std::max(0, max_level - std::bit_width(len));
}

void Terrain::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new TerrainInspector(this);
	}
}

}  // namespace spu::gs_node
