//
// Text :
//
#include <gsys/painter/text.h>
#include <gsys/canvas.h>
#include "character.h"

namespace spu::gs_painter {

void Text::init(const Attrs & /*attrs*/)
{
	setIsKeepInHost(true);

	// init
	{
		uint32_t shader_id;
		spu_print_get(0, "shader_id", &shader_id);
		assert(shader_id);

		Attrs init_attrs = {
		        {"shader_id",    shader_id},
		        {"a.a_position", 3        },
		        {"a.a_texcoord", 2        },
		        {"a.a_color",    3        },
		};
		GsPainter::init(init_attrs);
	}

	// transforms
	{
		Attrs transform_attrs = {
		        {"divisor",               1 },
		        {"a.a_instance_nodetext", 16},
		};
		SpuArray::aux(transform_attrs, 1);
	}
	// uniforms
	{
		Attrs unif_attrs = {
		        {"u_textscreen",   &u_textscreen  },
		        {"u_color",        &u_color       },
		        {"u_font_texture", &u_font_texture},
		        {"u_smoothstep",   &u_smoothstep  },
		};
		GsPainter::addUniforms(unif_attrs);
		spu_print_get(0, "texture_id", &u_font_texture);
		assert(u_font_texture);
	}

	// drawcalls
	{
		auto &drawcall = getADrawcall();
		drawcall.flags.depth_test = false;
		drawcall.flags.cull_face = false;
		drawcall.flags.blend = true;
		drawcall.coms[0].mode = GL_QUADS;
		drawcall.ub_material.albedo = eone<Vec4f>();
	}
	setTextscreen(Vec2f(0));
}

void Text::setTextscreen(const Mat4f &textscreen, const GsCanvas *current)
{
	if (current == nullptr) current = GsCanvas::getCurrent();
	auto fragscreen = current->fragscreen(0);
	auto cs = charScale();
	auto textfrag = fragscreen.inverse() * textscreen;
	auto fp0 = textfrag.pers3(ex());
	auto fp1 = textfrag.pers3(Vec3f(0, 0, 0));
	auto pers_scale = distance(fp0, fp1);

	u_smoothstep = 1.0 / (cs * pers_scale * 1.2);  // ad-hoc
	u_textscreen = textscreen;
}

void Text::setTextscreen(const Vec2f &origin, float pitch, const GsCanvas *current)
{
	if (current == nullptr) current = GsCanvas::getCurrent();
	auto fragscreen = current->fragscreen(0);
	auto textfrag = Mat4f().scale({pitch, pitch, 1}).trans(origin);
	setTextscreen(fragscreen * textfrag);
}

void Text::begin()
{
	m_pen = {0.0, 0.0, 0.0};
	getRange().invalidate();  // dont' forget
	getVerticesView().clear();
	getIndices().clear();
}

void Text::end() { send(); }

void Text::update() { send(); }

void Text::puts(const char *text)
{
	auto cs = charScale();
	for (; *text; text++) {
		switch (*text) {
		case '\v': {
			switch (text[1]) {
			case 'c': {
				auto scan_count
				        = sscanf(text + 2, "(%f,%f,%f)", &m_color.r, &m_color.g, &m_color.b);
				assert(scan_count == 3);
				text = strchr(text, ')');
				break;
			}
			case 'p': {
				auto scan_count = sscanf(text + 2, "(%f,%f,%f)", &m_pen.x, &m_pen.y, &m_pen.z);
				assert(scan_count == 3);
				text = strchr(text, ')');
				break;
			}
			default: break;
			}
			break;
		}
		case '\r': {
			m_pen.x = 0;
			break;
		}
		case '\n': {
			getRange().expand(m_pen);  // tricky
			m_pen.x = 0;
			m_pen.y -= 2;  // not scale
			break;
		}
		case '\t': {
			auto tab = 8;
			m_pen.x = floor((m_pen.x + tab) / tab) * tab * cs;
			break;
		}
		default: {
			putc(*text);
			m_pen.x += cs;
		}
		}
	}
}

void Text::putc(const uint8_t c)
{
	auto &c_font = text::c_font_Monospace;
	auto fs = fontScale();
	auto cs = charScale();
	auto r = cs / fs;

	auto info = charInfo(c);
	if (info.codePoint == 0) {
		m_pen.x += cs;
		return;
	}

	auto aw = float(c_font.width);
	auto ah = float(c_font.height);

	auto cw = float(info.width);
	auto ch = float(info.height);

	auto ox = float(info.originX);
	auto oy = float(info.originY);

	auto x0 = m_pen.x - ox * r;
	auto y0 = m_pen.y - (ch - oy) * r;

	auto x1 = x0 + cw * r;
	auto y1 = y0 + ch * r;

	auto u0 = float(info.x);
	auto v1 = float(info.y);

	auto v0 = v1 + ch;
	auto u1 = u0 + cw;

	auto z = m_pen.z;

	auto add_vertex = [&](float x, float y, float z, float u, float v) {
		auto vertex = Vertex(x, y, z, u, v, m_color.r, m_color.g, m_color.b);
		getVerticesView().push_back(vertex);
		getRange().expand(Vec3f(x, y, z));
	};

	add_vertex(x0, y0, z, u0 / aw, 1.0f - v0 / ah);
	add_vertex(x1, y0, z, u1 / aw, 1.0f - v0 / ah);
	add_vertex(x1, y1, z, u1 / aw, 1.0f - v1 / ah);
	add_vertex(x0, y1, z, u0 / aw, 1.0f - v1 / ah);
}

text::CharInfo Text::charInfo(const int8_t c) const
{
	auto &font = text::c_font_Monospace;
	for (auto i = 0; i < font.characterCount; i++) {
		auto &info = font.charInfos[i];
		if (info.codePoint == c) {
			return info;
		}
	}
	return {};
}

float Text::fontScale() const
{
	const float c_spacing = 2.0f;
	return text::c_font_Monospace.size / 2.0f + c_spacing;
}

void Text::doUse(uint32_t id)
{
	assert(id == 0);
	auto &drawcall = getADrawcall();
	auto &coms = drawcall.coms;
	assert(coms.size() == 1);
	coms[0].instance_count = instanceCount();
	u_color = drawcall.ub_material.albedo;
	GsPainter::doUse(0);
}

void Text::doRender()
{
	SpuArray::send(instancePtr(), instanceCount(), 1);
	GsPainter::doRender();
}

void Text::set(const Attrs &attrs)
{
	GsPainter::set(attrs);
	attrs.apply<vec4f_t>("pen", m_pen);
	attrs.apply<vec4f_t>("color", m_color);
	attrs.apply("char_scale", m_charScale);
}
}  // namespace spu::gs_painter
