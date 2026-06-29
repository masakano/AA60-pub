//
// Stdout :
//
#pragma once

#include <gsys/painter/text.h>
#include <smath/mesh.h>
#include <smath/convex3f.h>

namespace spu::gs_painter {
class Stdout : public GsPainter {
public:
	static constexpr int32_t e_class_depth = GsPainter::e_class_depth + 1;

	struct Vertex {
		Vec3f p;
		Vec4f c;

		Vertex() = default;
		Vertex(const Vec3f &p, const Vec4f &c) noexcept : p(p), c(c) {}
		explicit Vertex(const Mesh::Vertex &v) noexcept : Vertex(v.p, v.c) {}
		operator Mesh::Vertex() const noexcept { return Mesh::Vertex(p, ez(), ez(), c); }
		operator Vec3f() const { return p; }
	};

	explicit Stdout(const char *name = nullptr) : GsPainter(name) {}
	explicit Stdout(const Attrs &attrs) : Stdout() { init(attrs); }

	~Stdout() = default;

	void setColor(const Vec4f &color);
	void setColor(const char *name, float alpha = 1.0);
	//void setCharScale(float scale) { m_text.setCharScale(scale); }

	void addPrim(const Vec3f &point, const Mat4f &nodeworld = Mat4f());
	void addPrim(const std::vector<Vec3f> &points, const Mat4f &nodeworld = Mat4f());
	void addPrim(const Segment3f &segment, const Mat4f &nodeworld = Mat4f());
	void addPrim(const Mat4f &frustum, const Mat4f &nodeworld = Mat4f());
	void addPrim(const Range2f &range2, const Mat4f &nodeworld = Mat4f());
	void addPrim(const Range3f &range3, const Mat4f &nodeworld = Mat4f());
	void addPrim(const Convex2f &convex2, const Mat4f &nodeworld = Mat4f());
	void addPrim(const Convex3f &convex3, const Mat4f &nodeworld = Mat4f());
	void addPrim(const Mesh &mesh, const Mat4f &nodeworld = Mat4f());
	void addJoint(const Vec3f &p0, const Vec3f &p1, const Mat4f &nodeworld = Mat4f());
	void addAxis(const Mat4f &nodeworld = Mat4f());
	void addFloor(float range, float step0, float step1, float step1_alpha = 0.2, bool is_yup = true);
	void puts(const Vec3f &position, const std::string &str, const Mat4f &nodeworld = Mat4f());

	const Vec4f &color() const { return m_color; }
	Text &getText() { return m_text; }

	void init(const Attrs &attrs) override;
	void set(const Attrs &attrs) override;

	void begin();
	void end();

	static Stdout *get();

	Vec4f u_albedo = eone<Vec4f>();
	float u_min_alpha = 0.5;
	float u_point_size = 1.0;
	int32_t u_is_point_sprite = 0;
	uint32_t u_albedomap = 0;

	void setFillAlpha(float alpha) { m_fillAlpha = alpha; }

	void addElements(
	        const std::vector<int32_t> &point_indices, const std::vector<int32_t> &line_indices,
	        const std::vector<std::vector<int32_t>> &faces, const std::vector<Vec3f> &points,
	        const Mat4f &nodeworld = Mat4f());

	using GsPainter::draw;
	void draw(
	        uint32_t mode, const std::vector<Mat4f> &nodeworlds, uint32_t first = 0, uint32_t count = 0,
	        uint32_t instance_count = 1, uint32_t target = 0, uint32_t base_vertex = 0,
	        uint32_t base_instance = 0) override;

	const std::map<uint32_t, std::vector<int32_t>> &subIndices() const { return m_subIndices; }

	PAINTER_VERTEX_FUNCS;

protected:
	struct String {
		Vec3f p;
		std::string s;
	};

	Text m_text;
	float m_fillAlpha = 0.20;
	Vec4f m_color = eone<Vec4f>();

	std::vector<String> m_strings;
	std::map<uint32_t, std::vector<int32_t>> m_subIndices;
	std::map<uint32_t, int32_t> m_subFirsts;

	void doRender() override;
	inline static Stdout *ms_painter = nullptr;

private:
	friend class ClassCreator<Stdout>;
	static void startup(const Attrs &);
	static void shutdown();
	static ClassCreator<Stdout> ms_classCreator;
};
}  // namespace spu::gs_painter
