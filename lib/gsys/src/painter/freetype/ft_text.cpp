//
// FTText :
//
#include <gsys/painter/ft_text.h>
#include "ft_atlas.h"

namespace spu::gs_painter {

namespace {
void set_glyph_vertex(
        FTText::Vertex &v, float x0, float y0, float z0, float s0, float t0, const Vec4f &color, float shift,
        float gamma)
{
	v.x = x0;
	v.y = y0;
	v.z = z0;
	v.s = s0;
	v.t = t0;
	v.r = color.r;
	v.g = color.g;
	v.b = color.b;
	v.a = color.a;
	v.shift = shift;
	v.gamma = gamma;
}
}  // namespace

FTText::FTText(const char *name) : GsPainter(name)
{
	m_atlas = new freetype::FTFontAtlas();
	setIsKeepInHost(true);
}

FTText::~FTText()
{
	if (m_isAtlasOwner) {
		delete m_atlas;
	}
}

void FTText::addVertices(
        const Vertex *vertices, const uint32_t vcount, const int32_t *indices, const uint32_t icount,
        bool is_relative)
{
	auto master_vertices = getVerticesView();
	auto &master_indices = getIndices();

	Vec4i item;
	item.x = master_vertices.size();
	item.y = vcount;
	item.z = master_indices.size();
	item.w = icount;

	for (auto i = 0u; i < vcount; i++) {
		master_vertices.push_back(vertices[i]);
	}

	for (auto i = 0u; i < icount; i++) {
		master_indices.push_back(indices[i] + (is_relative ? item.x : 0));
	}
	m_items.push_back(item);
	m_state = e_dirty;
}

void FTText::addChar(const freetype::FTMarkup &markup, const char *current, const char *previous)
{
	size_t vcount = 0;
	size_t icount = 0;

	// Maximum number of vertices is 20 (= 5x2 triangles) per glyph:
	//  - 2 triangles for background
	//  - 2 triangles for overline
	//  - 2 triangles for underline
	//  - 2 triangles for strikethrough
	//  - 2 triangles for glyph
	Vertex vertices[4 * 5];
	int32_t indices[6 * 5];
	float kerning = 0.0f;

	freetype::FTFont *font = markup.font;
	if (font == nullptr) {
		font = m_atlas->getFontFromMarkup(markup);
	}

	if (font->m_ascender > m_lineAscender) {
		float y = m_pen.y;
		m_pen.y -= (font->m_ascender - m_lineAscender);
		moveLastLine((int)(y - m_pen.y));
		m_lineAscender = font->m_ascender;
	}
	if (font->m_descender < m_lineDescender) {
		m_lineDescender = font->m_descender;
	}

	if (*current == '\n') {
		finishLine(true);
		return;
	}

	const auto *glyph = font->loadGlyph(current);
	const auto *black = font->loadGlyph(NULL);

	if (glyph == NULL) {
		return;
	}

	if (previous && font->m_kerning) {
		kerning = glyph->getKerning(previous);
	}
	m_pen.x += kerning;

	// Background
	if (markup.background_color.a > 0) {
		Vec4f c = markup.background_color;
		float x0 = (m_pen.x - kerning);
		float y0 = (int)(m_pen.y + font->m_descender);
		float x1 = (x0 + glyph->advance_x);
		float y1 = (int)(y0 + font->m_height + font->m_linegap);
		float s0 = black->s0;
		float t0 = black->t0;
		float s1 = black->s1;
		float t1 = black->t1;

		Vertex *vtx0 = &vertices[vcount];
		set_glyph_vertex(vtx0[0], (int)x0, y0, 0, s0, t0, c, x0 - ((int)x0), markup.gamma);
		set_glyph_vertex(vtx0[1], (int)x0, y1, 0, s0, t1, c, x0 - ((int)x0), markup.gamma);
		set_glyph_vertex(vtx0[2], (int)x1, y1, 0, s1, t1, c, x1 - ((int)x1), markup.gamma);
		set_glyph_vertex(vtx0[3], (int)x1, y0, 0, s1, t0, c, x1 - ((int)x1), markup.gamma);

		int32_t *idx0 = &indices[icount];
		idx0[0] = vcount + 0;
		idx0[1] = vcount + 1;
		idx0[2] = vcount + 2;
		idx0[3] = vcount + 0;
		idx0[4] = vcount + 2;
		idx0[5] = vcount + 3;
		vcount += 4;
		icount += 6;
	}

	// Underline
	if (markup.underline) {
		Vec4f c = markup.underline_color;
		float x0 = (m_pen.x - kerning);
		float y0 = (int)(m_pen.y + font->m_underlinePosition);
		float x1 = (x0 + glyph->advance_x);
		float y1 = (int)(y0 + font->m_underlineThickness);
		float s0 = black->s0;
		float t0 = black->t0;
		float s1 = black->s1;
		float t1 = black->t1;

		Vertex *vtx0 = &vertices[vcount];
		set_glyph_vertex(vtx0[0], (int)x0, y0, 0, s0, t0, c, x0 - ((int)x0), markup.gamma);
		set_glyph_vertex(vtx0[1], (int)x0, y1, 0, s0, t1, c, x0 - ((int)x0), markup.gamma);
		set_glyph_vertex(vtx0[2], (int)x1, y1, 0, s1, t1, c, x1 - ((int)x1), markup.gamma);
		set_glyph_vertex(vtx0[3], (int)x1, y0, 0, s1, t0, c, x1 - ((int)x1), markup.gamma);

		int32_t *idx0 = &indices[icount];
		idx0[0] = vcount + 0;
		idx0[1] = vcount + 1;
		idx0[2] = vcount + 2;
		idx0[3] = vcount + 0;
		idx0[4] = vcount + 2;
		idx0[5] = vcount + 3;

		vcount += 4;
		icount += 6;
	}

	// Overline
	if (markup.overline) {
		Vec4f c = markup.overline_color;
		float x0 = (m_pen.x - kerning);
		float y0 = (int)(m_pen.y + (int)font->m_ascender);
		float x1 = (x0 + glyph->advance_x);
		float y1 = (int)(y0 + (int)font->m_underlineThickness);
		float s0 = black->s0;
		float t0 = black->t0;
		float s1 = black->s1;
		float t1 = black->t1;

		Vertex *vtx0 = &vertices[vcount];
		set_glyph_vertex(vtx0[0], (int)x0, y0, 0, s0, t0, c, x0 - ((int)x0), markup.gamma);
		set_glyph_vertex(vtx0[1], (int)x0, y1, 0, s0, t1, c, x0 - ((int)x0), markup.gamma);
		set_glyph_vertex(vtx0[2], (int)x1, y1, 0, s1, t1, c, x1 - ((int)x1), markup.gamma);
		set_glyph_vertex(vtx0[3], (int)x1, y0, 0, s1, t0, c, x1 - ((int)x1), markup.gamma);

		int32_t *idx0 = &indices[icount];
		idx0[0] = vcount + 0;
		idx0[1] = vcount + 1;
		idx0[2] = vcount + 2;
		idx0[3] = vcount + 0;
		idx0[4] = vcount + 2;
		idx0[5] = vcount + 3;

		vcount += 4;
		icount += 6;
	}

	/* Strikethrough */
	if (markup.strikethrough) {
		Vec4f c = markup.strikethrough_color;
		float x0 = (m_pen.x - kerning);
		float y0 = (int)(m_pen.y + (int)font->m_ascender * .33f);
		float x1 = (x0 + glyph->advance_x);
		float y1 = (int)(y0 + (int)font->m_underlineThickness);
		float s0 = black->s0;
		float t0 = black->t0;
		float s1 = black->s1;
		float t1 = black->t1;

		Vertex *vtx0 = &vertices[vcount];
		set_glyph_vertex(vtx0[0], (int)x0, y0, 0, s0, t0, c, x0 - ((int)x0), markup.gamma);
		set_glyph_vertex(vtx0[1], (int)x0, y1, 0, s0, t1, c, x0 - ((int)x0), markup.gamma);
		set_glyph_vertex(vtx0[2], (int)x1, y1, 0, s1, t1, c, x1 - ((int)x1), markup.gamma);
		set_glyph_vertex(vtx0[3], (int)x1, y0, 0, s1, t0, c, x1 - ((int)x1), markup.gamma);

		int32_t *idx0 = &indices[icount];
		idx0[0] = vcount + 0;
		idx0[1] = vcount + 1;
		idx0[2] = vcount + 2;
		idx0[3] = vcount + 0;
		idx0[4] = vcount + 2;
		idx0[5] = vcount + 3;

		vcount += 4;
		icount += 6;
	}
	{
		// Actual glyph
		Vec4f c = markup.foreground_color;
		float x0 = (m_pen.x + glyph->offset_x);
		float y0 = (int)(m_pen.y + glyph->offset_y);
		float x1 = (x0 + glyph->width);
		float y1 = (int)(y0 - glyph->height);
		float s0 = glyph->s0;
		float t0 = glyph->t0;
		float s1 = glyph->s1;
		float t1 = glyph->t1;

		Vertex *vtx0 = &vertices[vcount];
		set_glyph_vertex(vtx0[0], (int)x0, y0, 0, s0, t0, c, x0 - ((int)x0), markup.gamma);
		set_glyph_vertex(vtx0[1], (int)x0, y1, 0, s0, t1, c, x0 - ((int)x0), markup.gamma);
		set_glyph_vertex(vtx0[2], (int)x1, y1, 0, s1, t1, c, x1 - ((int)x1), markup.gamma);
		set_glyph_vertex(vtx0[3], (int)x1, y0, 0, s1, t0, c, x1 - ((int)x1), markup.gamma);

		int32_t *idx0 = &indices[icount];
		idx0[0] = vcount + 0;
		idx0[1] = vcount + 1;
		idx0[2] = vcount + 2;
		idx0[3] = vcount + 0;
		idx0[4] = vcount + 2;
		idx0[5] = vcount + 3;

		vcount += 4;
		icount += 6;

		addVertices(vertices, vcount, indices, icount, true);
		m_pen.x += glyph->advance_x * (1.0f + markup.spacing);
	}
}

void FTText::addText(const freetype::FTMarkup &markup, const char *text, uint32_t length)
{
	// size_t i;
	const char *prev_character = NULL;

	if (length == 0) {
		length = freetype::utf8_strlen(text);
	}
	if (m_items.size() == 0) {
		m_origin = m_pen;
		m_lineLeft = m_pen.x;
		getRange() = {m_pen, m_pen};
	}
	else {
		if (m_pen.x < m_origin.x) {
			m_origin.x = m_pen.x;
		}
		if (m_pen.y != m_lastPenY) {
			finishLine(false);
		}
	}

	for (auto i = 0; length; i += freetype::utf8_surrogate_len(text + i)) {
		addChar(markup, text + i, prev_character);
		prev_character = text + i;
		length--;
	}
	m_lastPenY = m_pen.y;
}

void FTText::align(Align alignment)
{
	if (m_alignment == alignment) {
		return;
	}
	aux_error(m_alignment != e_align_left, "You can align only once\n");
	m_alignment = alignment;

	if (m_lineStart != m_items.size()) {
		finishLine(false);
	}

	float line_left;
	float line_right;
	float line_center;
	float dx;
	uint32_t line_end;

	auto self_right = getRange().p1.x;
	auto self_center = getRange().center().x;
	auto lines_count = m_lineInfos.size();

	for (size_t i = 0; i < lines_count; ++i) {
		auto &line_info = m_lineInfos.at(i);

		if (i + 1 < lines_count) {
			line_end = m_lineInfos.at(i + 1).line_start;
		}
		else {
			line_end = m_items.size();
		}

		line_right = line_info.range.p1.x;

		if (m_alignment == e_align_right) {
			dx = self_right - line_right;
		}
		else if (m_alignment == e_align_center) {
			line_left = line_info.range.p0.x;
			line_center = (line_left + line_right) / 2;
			dx = self_center - line_center;
		}
		else {
			assert(0);
		}

		dx = roundf(dx);

		auto vertices_view = getVerticesView();
		for (auto j = line_info.line_start; j < line_end; ++j) {
			Vec4i &item = m_items.at(j);
			for (auto k = item.i[0]; k < item.i[0] + item.i[1]; ++k) {
				Vertex &vertex = vertices_view.at(k);
				vertex.x += dx;
			}
		}
	}
}

void FTText::transform(const Mat4f &matrix)
{
	auto vertices_view = getVerticesView();
	for (auto &vertex: vertices_view) {
		Vec3f p = matrix.ortho3(Vec3f(vertex.x, vertex.y, vertex.z));
		vertex.x = p.x;
		vertex.y = p.y;
		vertex.z = p.z;
	}
}

void FTText::finishLine(bool advancePen)
{
	LineInfo line_info;
	line_info.line_start = m_lineStart;
	line_info.range.p0 = {m_lineLeft, m_pen.y + m_lineDescender, 0};
	line_info.range.p1 = {m_pen.x, m_pen.y + m_lineAscender, 0};

	m_lineInfos.push_back(line_info);
	getRange().expand(line_info.range);

	if (advancePen) {
		m_pen.x = m_origin.x;
		m_pen.y += (int)(m_lineDescender);
	}

	m_lineDescender = 0;
	m_lineAscender = 0;
	m_lineStart = m_items.size();
	m_lineLeft = m_pen.x;
}

void FTText::moveLastLine(float dy)
{
	auto vertices_view = getVerticesView();
	for (auto i = m_lineStart; i < m_items.size(); ++i) {
		Vec4i &item = m_items.at(i);
		for (auto j = item.i[0]; j < item.i[0] + item.i[1]; ++j) {
			Vertex &vertex = vertices_view.at(j);
			vertex.y -= dy;
		}
	}
}

void FTText::init(const Attrs &attrs)
{
	// atlas
	{
		auto def_render_mode = attrs.get<uint32_t>("def_render_mode", e_render_default);
		auto size = Vec4i(attrs.get<vec4i_t>("atlas.size", vec4i_t(0, 0, 1, 0)));
		auto *atlas = attrs.get<freetype::FTFontAtlas *>("atlas", nullptr);

		if (ms_globalAtlas) {
			aux_error(atlas != nullptr, "global atlas already defined\n");
			m_atlas = ms_globalAtlas;
			m_isAtlasOwner = false;
		}
		else if (atlas) {
			m_atlas = atlas;
			m_isAtlasOwner = false;
		}
		else {
			if (size.x > 0 && size.y > 0 && size.z > 0) {
				m_atlas->init(Vec4i(size.x, size.y, size.z, 0));
			}
		}
		if ((m_atlas->size().z == 3 && def_render_mode != e_render_lcd)
		    || (m_atlas->size().z != 3 && def_render_mode == e_render_lcd)) {
			aux_error(true, "depth must be 3 in LCD mode (e_render_lcd)\n");
		}
	}

	// painter
	{
		const char *c_path = "painter/freetype/text.us";
		Attrs painter_attrs = {
		        {"path",         c_path},
                        {"a.a_vertex",   3     },
                        {"a.a_texcoord", 2     },
		        {"a.a_color",    4     },
                        {"a.a_shift",    1     },
                        {"a.a_gamma",    1     },
		};
		painter_attrs += attrs;
		GsPainter::init(painter_attrs);
	}

	// shader
	{
		Attrs unif_attrs = {
		        {"u_texture",    &m_atlas->texture().id()},
		        {"u_textscreen", &u_textscreen           },
		        {"u_atlas_size", &u_atlas_size           },
		        {"u_color",      &u_color                },
		};
		addUniforms(unif_attrs);
		u_atlas_size = m_atlas->size();
	}

	// drawcall
	{
		auto &drawcall = getADrawcall();
		drawcall.flags.blend = true;
		drawcall.flags.depth_test = false;
		drawcall.flags.cull_face = false;
		drawcall.blend_func = {
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		};

		drawcall.flags.point_sprite = false;
		drawcall.flags.program_point_size = false;
		drawcall.point_size = 10.0;
	}
	m_state = e_dirty;
	getRange() = {Vec3f(0), Vec3f(0)};
}
#if 1
void FTText::doRender()
{
	if (m_state != e_clean) {
		auto vertices_view = getVerticesView();
		auto &indices = getIndices();
		SpuArray::send(vertices_view.data(), vertices_view.size(), 0);
		SpuArray::send(indices, -1);
		m_state = e_clean;
	}

	GsPainter::doRender();
}

#else
void FTText::doUse(uint32_t id)
{
	if (id == 0) {
		if (m_state != e_clean) {
			auto vertices_view = getVerticesView();
			auto &indices = getIndices();
			SpuArray::send(vertices_view.data(), vertices_view.size(), 0);
			SpuArray::send(indices, -1);
			m_state = e_clean;
		}
	}
	GsPainter::doUse(id);
}
#endif

void FTText::clear()
{
	getVerticesView().clear();
	getIndices().clear();
	getRange() = {Vec3f(0), Vec3f(0)};

	m_state = e_dirty;
	m_items.clear();
	m_lineInfos.clear();
	m_origin = Vec2f(0);
	m_lastPenY = 0;
	m_lineStart = 0;
	m_lineLeft = 0;
	m_lineAscender = 0;
	m_lineDescender = 0;
	m_alignment = e_align_left;
}

Vec4i FTText::atlasSize() const { return m_atlas->size(); }

uint32_t FTText::atlasUsed() const { return m_atlas->used(); }

Recti FTText::getAtlasRegion(const uint32_t width, const uint32_t height)
{
	return m_atlas->getRegion(width, height);
}

void FTText::setAtlasRegion(const Recti &region, const uint8_t *data, const uint32_t stride)
{
	m_atlas->setRegion(region, data, stride);
}

freetype::FTFont *FTText::getFontFromFile(const char *filename, const float fontsize)
{
	return m_atlas->getFontFromFile(filename, fontsize);
}

void FTText::upload() { m_atlas->upload(); }

void FTText::allocGlobalAtlas(const Vec4i &size)
{
	if (!ms_globalAtlas) {
		ms_globalAtlas = new freetype::FTFontAtlas();
	}
	ms_globalAtlas->init(size);
}

void FTText::freeGlobalAtlas()
{
	if (ms_globalAtlas) {
		delete ms_globalAtlas;
		ms_globalAtlas = nullptr;
	}
}
}  // namespace spu::gs_painter
