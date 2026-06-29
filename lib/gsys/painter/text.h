//
// Text :
//
#pragma once
#include <gsys/painter.h>
#include <gsys/canvas.h>

namespace spu::gs_painter {
namespace text {
struct CharInfo;
}
class Text : public GsPainter {
public:
	struct Vertex {
		float x, y, z, u, v, r, g, b;

		Vertex() = default;

		Vertex(float x, float y, float z, float u, float v, float r, float g, float b)
		        : x(x), y(y), z(z), u(u), v(v), r(r), g(g), b(b)
		{
		}
		explicit Vertex(const Mesh::Vertex &v) noexcept
		        : Vertex(v.p.x, v.p.y, v.p.z, v.t.x, v.t.y, v.c.x, v.c.y, v.c.z)
		{
		}
		operator Mesh::Vertex() const noexcept
		{
			return Mesh::Vertex({x, y, z}, {u, v, 1}, ez(), {r, g, b, 1});
		}
		operator Vec3f() const { return {x, y, z}; }
	};

	explicit Text(const char *name = nullptr) : GsPainter(name) {}
	explicit Text(const Attrs &attrs) : Text() { init(attrs); }

	auto &getPen() const { return m_pen; }
	auto &getPen() { return m_pen; }

	void puts(const char *text);

	void setTextscreen(const Mat4f &textscreen, const GsCanvas *current = nullptr);
	void setTextscreen(const Vec2f &origin, float pitch = 10, const GsCanvas *current = nullptr);
	void setColor(const Vec3f &color) { m_color = color; }
	void setCharScale(float char_scale) { m_charScale = char_scale; }
	void addTilt(const Vec2f &tilt) { m_pen += tilt; }

	float fontScale() const;
	float charScale() const { return m_charScale; }

	void init(const Attrs &attrs) override;
	void set(const Attrs &attrs) override;
	void update() override;
	void begin();
	void end();

	const Mat4f &textscreen() const { return u_textscreen; }

	using GsPainter::send;
	PAINTER_VERTEX_FUNCS;

protected:
	Vec3f m_pen = ezero();
	Vec4f m_color = eone();
	float m_charScale = 1.0;

	uint32_t u_font_texture;
	Vec3f u_color = eone();
	Mat4f u_textscreen;
	float u_smoothstep = 0;

	void putc(const uint8_t c);
	text::CharInfo charInfo(const int8_t c) const;

	void doRender() override;
	void doUse(uint32_t id) override;
};
}  // namespace spu::gs_painter
