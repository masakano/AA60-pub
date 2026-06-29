//
// Mesh :
//
#include <gsys/node/manifold_2d.h>

namespace spu::gs_node {

void Manifold2D::init(const Attrs &attrs)
{
	GsNode::init(attrs);

	m_attrs = attrs;
	m_meshGrid = m_attrs.get("mesh_grid", vec4i_t(1, 1, 1, 1));

	auto mapnode_scale = m_attrs.get("mapnode_scale", vec4f_t(1.0));
	auto maptexc_scale = m_attrs.get("maptexc_scale", vec4f_t(1.0));

	Mat4f unit;
	auto mapnode = unit.rot("x", pi() / 2);
	mapnode = *m_attrs.get("mapnode", &mapnode) * unit.scale(mapnode_scale);

	auto maptexc = unit;
	maptexc = *m_attrs.get("maptexc", &maptexc) * unit.scale(maptexc_scale);

	assert(m_meshGrid.x >= 0 && m_meshGrid.y >= 0);

	auto shape = m_attrs.getf<const char *>("shape");
	if (shape.hit) {
		auto modifier = getDefaultModifier(shape.value);
		createVertices(modifier, mapnode, maptexc);
	}
}

void Manifold2D::createVertices(modifier_t modifier, const Mat4f &mapnode, const Mat4f &maptexc)
{
	aux_error(modifier == nullptr, "no modifer set\n");
	auto &vertices = m_mesh.getVertices();
	vertices.clear();
	for (auto iy = 0; iy <= m_meshGrid.y; iy++) {
		for (auto ix = 0; ix <= m_meshGrid.x; ix++) {
			Vec2f uv = {
			        (2.0f * ix) / m_meshGrid.x - 1.0f,
			        (2.0f * iy) / m_meshGrid.y - 1.0f,
			};
			auto vertex = modifier(uv, mapnode, maptexc);
			vertices.push_back(vertex);
		}
	}
}

void Manifold2D::createIndices(int32_t patch_vertices, int32_t step)
{
	const int32_t offsets[4] = {0, step, step * (m_meshGrid.x + 2), step * (m_meshGrid.x + 1)};  // ccw
	auto &faces = m_mesh.getFaces();
	faces.clear();

	for (auto y = 0; y < m_meshGrid.y; y += step) {
		for (auto x = 0; x < m_meshGrid.x; x += step) {
			auto base = y * (m_meshGrid.x + 1) + x;

			if (patch_vertices == 4) {
				Mesh::Face quad = {
				        base + offsets[0],
				        base + offsets[1],
				        base + offsets[2],
				        base + offsets[3],
				};
				faces.push_back(quad);
			}
			else {
				Mesh::Face triangle0 = {
				        base + offsets[0],
				        base + offsets[1],
				        base + offsets[2],
				};

				Mesh::Face triangle1 = {
				        base + offsets[0],
				        base + offsets[2],
				        base + offsets[3],
				};
				faces.push_back(triangle0);
				faces.push_back(triangle1);
			}
		}
	}
}

void Manifold2D::replacePainter(GsPainter *painter)
{
	auto patch_vertices = 0;
	painter->get("patch_vertices", &patch_vertices);

	assert(!m_mesh.getVertices().empty());
	createIndices(patch_vertices, 1);
	Mesh::postproc(m_attrs, m_mesh);

	auto step = 1;
	auto first = 0;
	// auto count = 0;

	auto &notches = getNotches();
	auto &drawcall = painter->getADrawcall();
	auto com = drawcall.coms[0];  // copy

	std::vector<int32_t> indices;

	drawcall.coms.clear();
	notches.clear();

	while (m_meshGrid.x / step * step == m_meshGrid.x && m_meshGrid.y / step * step == m_meshGrid.y) {
		createIndices(patch_vertices, step);
		auto notch = m_mesh.getNotch();
		for (auto &face: m_mesh.getFaces()) {
			for (auto &index: face) {
				indices.push_back(index);
			}
		}
		auto count = m_mesh.getIndexCount();
		// printf("count = %d\n", count);
		com.mode = patch_vertices == 4 ? GL_QUADS : GL_TRIANGLES;
		com.first = first;
		com.count = count;

		first += count;
		// count = 0;
		step *= 2;
		drawcall.coms.push_back(com);
		notches.push_back(notch);
	}
	// printf("coms=%ld\n", painter->getADrawcall().coms.size());
	painter->send(m_mesh.getVertices(), indices);
	painter->update();
	GsNode::replacePainter(painter);
}

Manifold2D::modifier_t Manifold2D::getDefaultModifier(const std::string &shape)
{
	if (shape == "plate") {
		auto modifier = [](const Vec2f &uv, const Mat4f &mapnode, const Mat4f &maptexc) {
			// y-up
			auto u = uv.x;
			auto v = uv.y;
			auto p = mapnode.ortho3(Vec3f(u, v, 0));
			auto t = maptexc.ortho3(Vec3f(u, v, 0));
			auto n = normalize(mapnode.rot3(ez()));
			return Mesh::Vertex(p, t, n);
		};
		return modifier;
	}
	else if (shape == "sphere") {
		auto modifier = [](const Vec2f &uv, const Mat4f &mapnode, const Mat4f &maptexc) {
			auto u = uv.x;
			auto v = uv.y;
			auto au = radians(u * 180);
			auto av = radians(v * 90);

			auto r0 = cosf(av);
			auto px = r0 * cosf(au);
			auto py = r0 * sinf(au);
			auto pz = sinf(av);

			auto p = mapnode.ortho3(Vec3f(px, py, pz));
			auto t = maptexc.ortho3(Vec3f(u * 2.0, v, 0));
			auto n = normalize(p);

			return Mesh::Vertex(p, t, n);
		};
		return modifier;
	}
	else if (shape == "cylinder") {
		auto modifier = [](const Vec2f &uv, const Mat4f &mapnode, const Mat4f &maptexc) {
			const auto c_scale = sqrtf(2.0);
			auto u = uv.x;
			auto v = uv.y;

			auto au = radians(u * 180);
			auto av = radians(v * 90);

			auto r0 = std::min(c_scale * cosf(av), 1.0f);
			auto px = r0 * cosf(au);
			auto py = r0 * sinf(au);
			auto pz = std::clamp(c_scale * sinf(av), -1.0f, +1.0f);

			auto p = mapnode.ortho3(Vec3f(px, py, pz));
			auto t = maptexc.ortho3(Vec3f(u * 2.0, v * 2.0, 0));

			auto n0 = v > 0.5 ? ez() : v < -0.5 ? -ez() : Vec3f(px, py, 0);
			auto n = normalize(Vec3f(mapnode * n0));

			return Mesh::Vertex(p, t, n);
		};
		return modifier;
	}
	else if (shape == "torus") {
		auto modifier = [](const Vec2f &uv, const Mat4f &mapnode, const Mat4f &maptexc) {
			const auto r0 = 1.00f;
			const auto r1 = 0.25f;

			auto u = uv.x;
			auto v = uv.y;

			auto au = radians(u * 180);
			auto av = radians(v * 180);

			auto x = r1 * sinf(au);
			auto y = r1 * cosf(au);

			Mat4f m;
			m.c[0] = {
			        sinf(av),
			        0.0,
			        cosf(av),
			        0.0,
			};
			m.c[1] = {0.0, 1.0, 0.0, 0.0};
			m.c[2] = cross(m.c[0], m.c[1]);
			m.c[3] = {
			        r0 * sinf(av),
			        0.0,
			        r0 * cosf(av),
			        1.0,
			};

			auto p = (mapnode * m).ortho3(Vec3f(x, y, 0));
			auto t = maptexc.ortho3(Vec3f(u, v * 4.0, 0));
			auto n = normalize((mapnode * m).rot3(Vec3f(x, y, 0)));
			return Mesh::Vertex(p, t, n);
		};
		return modifier;
	}
	else if (shape == "spring") {
		auto modifier = [](const Vec2f &uv, const Mat4f &mapnode, const Mat4f &maptexc) {
			const auto r0 = 1.00f;

			auto u = uv.x;
			auto v = uv.y;
			auto r1 = (v == -1.0f || v == 1.0f) ? 0.0f : 0.25f;

			auto au = u * pi();
			auto av = v * pi() * 4.0;

			auto x = r1 * sinf(au);
			auto y = r1 * cosf(au);

			Mat4f m;
			m.c[0] = {sinf(av), 0.0, cosf(av), 0.0};
			m.c[1] = {0.0, 1.0, 0.0, 0.0};
			m.c[2] = cross(m.c[0], m.c[1]);
			m.c[3] = {r0 * sinf(av), v * 2.0f, r0 * cosf(av), 1.0f};

			auto p = (mapnode * m).ortho3(Vec3f(x, y, 0));
			auto t = maptexc.ortho3(Vec3f(u, v * 16.0, 0));

			Vec3f n;
			if (v == -1.0 || v == 1.0) {
				n = normalize((mapnode * m).rot3(ez()));
			}
			else {
				n = normalize((mapnode * m).rot3(Vec3f(x, y, 0)));
			}
			return Mesh::Vertex(p, t, n);
		};
		return modifier;
	}
	else {
		aux_error(true, "unknown shape '%s'. must be plate:sphere:torus:spring \n", shape.c_str());
	}
}
}  // namespace spu::gs_node
