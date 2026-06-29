//
// ParamsFunc :
//
#pragma once

#include "spu_renderstate_lib.h"

namespace spu::libspu::spu_renderstate {

#define setScalar(val, prev, func)      \
	{                               \
		if ((val) != *(prev)) { \
			*(prev) = val;  \
			F(func, val);   \
			return 1;       \
		}                       \
		return 0;               \
	}

template<class T> struct multivec {
	T v[16];

	multivec() { memset(v, -1, sizeof(v)); }

	int32_t cache(int32_t chan, const void *curr)
	{
		if (chan == -1) {  // all channel
			if (spu_renderstate::cache(&v[0], curr)) {
				for (auto i = 1; i < 16; i++) {
					v[i] = v[0];
				}
				return 1;
			}
			return 0;
		}
		return spu_renderstate::cache(&v[chan], curr);
	}
};

vec4 s_clear_color = {
        {-1, -1, -1, -1}
};
vec4 s_blend_color = {
        {-1, -1, -1, -1}
};
vec4 s_viewport = {
        {-1, -1, -1, -1}
};
vec4 s_scissor = {
        {-1, -1, -1, -1}
};
vec2 s_poly_offset = {
        {0, 0}
};
vec2 s_line_stipple = {
        {-1, -1}
};
float s_min_sample_shading = 0;
float s_clear_depth = std::numeric_limits<float>::max();
float s_line_width = 1.0;
float s_point_size = 1.0;
uint32_t s_depth_func = ~0u;
uint32_t s_cull_face = ~0u;
uint32_t s_clear_stencil = 0;

struct {
	int32_t i[12];
} s_stencil_func = {
        {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
};  // {int

multivec<vec4> s_blend_func;
multivec<vec2> s_blend_eq;
multivec<uint32_t> s_write_mask;

class ParamsFunc {
public:
	virtual bool is_deep() = 0;
	virtual bool use(value_t val) = 0;
	virtual value_t get(value_t val) = 0;
	virtual void report() = 0;
	virtual ~ParamsFunc() = default;
};

class PFlags : public ParamsFunc {
public:
	PFlags(const Attrs &attrs) : m_flags(attrs.select("flags.")) {}

	bool is_deep() override { return false; }
	bool use(value_t val) override { return m_flags.use(val.u); }
	value_t get(value_t) override { return value_t(m_flags.get()); }
	void report() override { m_flags.report("flags."); }

private:
	Flags m_flags;
};

class PBlendFuncChannel : public ParamsFunc {
public:
	PBlendFuncChannel() = default;
	bool is_deep() override { return false; }

	bool use(value_t val) override
	{
		if (val.s != s_blend_func_channel) {
			aux_error(val.s < -1 || val.s >= 16, "invalid blend func channel (%d)\n", val.s);
			s_blend_func_channel = val.s;
			return true;
		}
		return false;
	}

	value_t get(value_t) override { return value_t(s_blend_func_channel); }

	void report() override { printInt("blend_func_channel", s_blend_func_channel); }
};

class PWriteMask : public ParamsFunc {
public:
	PWriteMask() { get(value_t(nullptr)); }

	bool is_deep() override { return false; }

	bool use(value_t val) override
	{
		auto chan = s_blend_func_channel;

		if (s_write_mask.cache(chan, &val.u) != 0) {
			auto rmask = ((val.u >> 0) & 0x1) != 0u;
			auto gmask = ((val.u >> 1) & 0x1) != 0u;
			auto bmask = ((val.u >> 2) & 0x1) != 0u;
			auto amask = ((val.u >> 3) & 0x1) != 0u;
			auto zmask = ((val.u >> 4) & 0x1) != 0u;
			if (chan == -1) {
				F(glColorMask, rmask, gmask, bmask, amask);
				F(glDepthMask, zmask);
			}
			else {
				F(glColorMaski, chan, rmask, gmask, bmask, amask);
				F(glDepthMask, zmask);
			}
			return true;
		}
		return false;
	}

	value_t get(value_t) override
	{
		// channel #0 only
		GLboolean cm[4];
		GLboolean dm[1];

		F(glGetBooleanv, GL_COLOR_WRITEMASK, cm);
		F(glGetBooleanv, GL_DEPTH_WRITEMASK, dm);

		s_write_mask.v[0] = 0;
		s_write_mask.v[0] |= ((cm[0] == GL_TRUE) ? 1 : 0) << 0;
		s_write_mask.v[0] |= ((cm[1] == GL_TRUE) ? 1 : 0) << 1;
		s_write_mask.v[0] |= ((cm[2] == GL_TRUE) ? 1 : 0) << 2;
		s_write_mask.v[0] |= ((cm[3] == GL_TRUE) ? 1 : 0) << 3;
		s_write_mask.v[0] |= ((dm[0] == GL_TRUE) ? 1 : 0) << 4;

		return value_t(s_write_mask.v[0]);
	}

	void report() override
	{
		// channel #0 only
		aux_printf(
		        "\t%-24s : [%08x] (r,g,b,a,z)=(%d,%d,%d,%d,%d)\n", "write_mask", s_write_mask.v[0],
		        (s_write_mask.v[0] >> 0) & 0x1, (s_write_mask.v[0] >> 1) & 0x1,
		        (s_write_mask.v[0] >> 2) & 0x1, (s_write_mask.v[0] >> 3) & 0x1,
		        (s_write_mask.v[0] >> 4) & 0x1);
	}
};

class PBlendFunc : public ParamsFunc {
public:
	PBlendFunc() { get(value_t(nullptr)); }
	bool is_deep() override { return true; }

	bool use(value_t val) override
	{
		auto chan = s_blend_func_channel;
		const auto *p = (const int32_t *)val.p;
		if (s_blend_func.cache(chan, p) != 0) {
			if (chan == -1) {
				F(glBlendFuncSeparate, p[0], p[1], p[2], p[3]);
			}
			else {
#ifdef MAINTENANCE
				aux_printf("glBlendFunc: chan=%d\n", chan);
				aux_printf(
				        "    p0: %04x %s\n", ((int32_t *)p)[0],
				        opengl_const(s_blend_func.v[chan].i[0]));
				aux_printf(
				        "    p1: %04x %s\n", ((int32_t *)p)[1],
				        opengl_const(s_blend_func.v[chan].i[1]));
				aux_printf(
				        "    p2: %04x %s\n", ((int32_t *)p)[2],
				        opengl_const(s_blend_func.v[chan].i[2])) g;
				aux_printf(
				        "    p3: %04x %s\n", ((int32_t *)p)[3],
				        opengl_const(s_blend_func.v[chan].i[3]));
#endif
				F(glBlendFuncSeparatei, chan, p[0], p[1], p[2], p[3]);
			}
			return true;
		}
		return false;
	}

	value_t get(value_t val) override
	{
		// channel #0 only
		s_blend_func.v[0].i[0] = getInt(GL_BLEND_SRC_RGB);
		s_blend_func.v[0].i[1] = getInt(GL_BLEND_DST_RGB);
		s_blend_func.v[0].i[2] = getInt(GL_BLEND_SRC_ALPHA);
		s_blend_func.v[0].i[3] = getInt(GL_BLEND_DST_ALPHA);
		return copyDeep(val, s_blend_func.v[0]);
	}

	void report() override
	{
		auto chan = s_blend_func_channel;
		if (chan < 0) {
			chan = 0;  // chanel #0 only
		}
		printEnum("blend_func.sc", s_blend_func.v[chan].i[0]);
		printEnum("blend_func.dc", s_blend_func.v[chan].i[1]);
		printEnum("blend_func.sa", s_blend_func.v[chan].i[2]);
		printEnum("blend_func.da", s_blend_func.v[chan].i[3]);
	}
};

class PBlendEq : public ParamsFunc {
public:
	PBlendEq() { get(value_t(nullptr)); }
	bool is_deep() override { return true; }

	bool use(value_t val) override
	{
		auto chan = s_blend_func_channel;
		const auto *p = (const int32_t *)val.p;

		if (s_blend_eq.cache(chan, p) != 0) {
			if (chan == -1) {
				F(glBlendEquationSeparate, p[0], p[1]);
			}
			else {
				F(glBlendEquationSeparatei, chan, p[0], p[1]);
			}
			return true;
		}
		return false;
	}

	value_t get(value_t val) override
	{
		// channel #0 only
		s_blend_eq.v[0].i[0] = getInt(GL_BLEND_EQUATION_RGB);
		s_blend_eq.v[0].i[1] = getInt(GL_BLEND_EQUATION_ALPHA);
		return copyDeep(val, s_blend_eq.v[0]);
	}

	void report() override
	{
		// channel #0 only
		auto chan = s_blend_func_channel;
		if (chan < 0) {
			chan = 0;  // chanel #0 only
		}
		printEnum("blend_eq.c", s_blend_eq.v[chan].i[0]);
		printEnum("blend_eq.a", s_blend_eq.v[chan].i[1]);
	}
};

class PStencilFunc : public ParamsFunc {
public:
	PStencilFunc() { get(value_t(nullptr)); }
	bool is_deep() override { return true; }

	static void set_stencils(const int32_t *params)
	{
		GLenum f_func = params[0];
		int32_t f_ref = params[1];
		uint32_t f_mask = params[2];
		GLenum f_sfail = params[3];
		GLenum f_dfail = params[4];
		GLenum f_pass = params[5];

		GLenum b_func = params[6];
		int32_t b_ref = params[7];
		uint32_t b_mask = params[8];
		GLenum b_sfail = params[9];
		GLenum b_dfail = params[10];
		GLenum b_pass = params[11];

		F(glStencilFuncSeparate, GL_FRONT, f_func, f_ref, f_mask);
		F(glStencilFuncSeparate, GL_BACK, b_func, b_ref, b_mask);

		F(glStencilOpSeparate, GL_FRONT, f_sfail, f_dfail, f_pass);
		F(glStencilOpSeparate, GL_BACK, b_sfail, b_dfail, b_pass);
	}

	bool use(value_t val) override
	{
		const auto *p = (const int32_t *)val.p;
		if ((p) && p[0] != 0 && cache(&s_stencil_func, p)) {
			set_stencils(p);
			return true;
		}
		return false;
	}

	value_t get(value_t val) override
	{
		// front
		s_stencil_func.i[0] = getInt(GL_STENCIL_FUNC);
		s_stencil_func.i[1] = getInt(GL_STENCIL_REF);
		s_stencil_func.i[2] = getInt(GL_STENCIL_VALUE_MASK);
		s_stencil_func.i[3] = getInt(GL_STENCIL_FAIL);
		s_stencil_func.i[4] = getInt(GL_STENCIL_PASS_DEPTH_FAIL);
		s_stencil_func.i[5] = getInt(GL_STENCIL_PASS_DEPTH_PASS);

		// back (same as front)
		s_stencil_func.i[6] = getInt(GL_STENCIL_FUNC);
		s_stencil_func.i[7] = getInt(GL_STENCIL_REF);
		s_stencil_func.i[8] = getInt(GL_STENCIL_VALUE_MASK);
		s_stencil_func.i[9] = getInt(GL_STENCIL_BACK_FAIL);
		s_stencil_func.i[10] = getInt(GL_STENCIL_BACK_PASS_DEPTH_FAIL);
		s_stencil_func.i[11] = getInt(GL_STENCIL_BACK_PASS_DEPTH_PASS);
		return copyDeep(val, s_stencil_func);
	}

	void report() override
	{
		printEnum("stencil_func.f_func", s_stencil_func.i[0]);
		printInt("stencil_func.f_ref", s_stencil_func.i[1]);
		printInt("stencil_func.f_mask", s_stencil_func.i[2]);
		printEnum("stencil_func.f_sfail", s_stencil_func.i[3]);
		printEnum("stencil_func.f_dfail", s_stencil_func.i[4]);
		printEnum("stencil_func.f_pass", s_stencil_func.i[5]);

		printEnum("stencil_func.b_func", s_stencil_func.i[6]);
		printInt("stencil_func.b_ref", s_stencil_func.i[7]);
		printInt("stencil_func.b_mask", s_stencil_func.i[8]);
		printEnum("stencil_func.b_sfail", s_stencil_func.i[9]);
		printEnum("stencil_func.b_dfail", s_stencil_func.i[10]);
		printEnum("stencil_func.b_pass", s_stencil_func.i[11]);
	}
};

class PDepthFunc : public ParamsFunc {
public:
	PDepthFunc() { get(value_t(nullptr)); }
	bool is_deep() override { return false; }

	bool use(value_t val) override
	{
		if (val.u == 0) {
			return false;  // need check
		}
		setScalar(val.u, &s_depth_func, glDepthFunc);
	}

	value_t get(value_t) override { return value_t(s_depth_func = getInt(GL_DEPTH_FUNC)); }

	void report() override { printEnum("depth_func", s_depth_func); }
};

class PCullFace : public ParamsFunc {
public:
	PCullFace() { get(value_t(nullptr)); }
	bool is_deep() override { return false; }

	bool use(value_t val) override
	{
		if (val.u == 0) {
			return false;  // need check
		}
		setScalar(val.u, &s_cull_face, glCullFace);
	}

	value_t get(value_t) override { return value_t(s_cull_face = getInt(GL_CULL_FACE_MODE)); }

	void report() override { printEnum("cull_face", s_cull_face); }
};

class PLineWidth : public ParamsFunc {
public:
	PLineWidth() { get(value_t(nullptr)); }
	bool is_deep() override { return false; }

	bool use(value_t val) override { setScalar(val.f, &s_line_width, glLineWidth); }
	value_t get(value_t) override { return value_t(s_line_width = getFloat(GL_LINE_WIDTH)); }
	void report() override { printFloat("line_width", s_line_width); }
};

class PPointSize : public ParamsFunc {
public:
	PPointSize() { get(value_t(nullptr)); }
	bool is_deep() override { return false; }

	bool use(value_t val) override { setScalar(val.f, &s_point_size, glPointSize); }
	value_t get(value_t) override { return value_t(s_point_size = getFloat(GL_POINT_SIZE)); }
	void report() override { printFloat("point_size", s_point_size); }
};

class PPolyOffset : public ParamsFunc {
public:
	PPolyOffset() { get(value_t(nullptr)); }
	bool is_deep() override { return true; }

	bool use(value_t val) override
	{
		const auto *p = (const float *)val.p;
		if (cache(&s_poly_offset, p)) {
			F(glPolygonOffset, p[0], p[1]);
			return true;
		}
		return false;
	}

	value_t get(value_t val) override
	{
		s_poly_offset.f[0] = getFloat(GL_POLYGON_OFFSET_FACTOR);
		s_poly_offset.f[1] = getFloat(GL_POLYGON_OFFSET_UNITS);
		return copyDeep(val, s_poly_offset);
	}

	void report() override
	{
		printFloat("poly_offset.f", s_poly_offset.f[0]);
		printFloat("poly_offset.u", s_poly_offset.f[1]);
	}
};

class PLineStipple : public ParamsFunc {
public:
	PLineStipple() { get(value_t(nullptr)); }
	bool is_deep() override { return true; }

	bool use(value_t val) override
	{
		const auto *p = (const int32_t *)val.p;
		if (cache(&s_line_stipple, p)) {
			F(glLineStipple, p[0], p[1]);
			return true;
		}
		return false;
	}

	value_t get(value_t val) override
	{
		s_line_stipple.i[0] = getInt(GL_LINE_STIPPLE_REPEAT);
		s_line_stipple.i[1] = getInt(GL_LINE_STIPPLE_PATTERN);
		return copyDeep(val, s_line_stipple);
	}

	void report() override
	{
		printFloat("line_stipple.f", s_line_stipple.i[0]);
		printFloat("line_stipple.p", s_line_stipple.i[1]);
	}
};

class PBlendColor : public ParamsFunc {
public:
	PBlendColor() { get(value_t(nullptr)); }
	bool is_deep() override { return true; }

	bool use(value_t val) override
	{
		const auto *p = (const float *)val.p;
		if (cache(&s_blend_color, p)) {
			F(glBlendColor, p[0], p[1], p[2], p[3]);
			return true;
		}
		return false;
	}

	value_t get(value_t val) override
	{
		s_blend_color = getFloat4(GL_BLEND_COLOR);
		return copyDeep(val, s_blend_color);
	}

	void report() override { printFloat4("blend_color", s_blend_color); }
};

class PViewport : public ParamsFunc {
public:
	PViewport() { get(value_t(nullptr)); }
	bool is_deep() override { return true; }

	bool use(value_t val) override
	{
		const auto *p = (const int32_t *)val.p;
		if (cache(&s_viewport, p)) {
			F(glViewport, p[0], p[1], p[2], p[3]);
			return true;
		}
		return false;
	}

	value_t get(value_t val) override
	{
		s_viewport = getInt4(GL_VIEWPORT);
		return copyDeep(val, s_viewport);
	}

	void report() override { printInt4("viewport", s_viewport); }
};

class PScissor : public ParamsFunc {
public:
	PScissor() { get(value_t(nullptr)); }
	bool is_deep() override { return true; }

	bool use(value_t val) override
	{
		const auto *p = (const int32_t *)(val.p);
		if (cache(&s_scissor, p)) {
			F(glScissor, p[0], p[1], p[2], p[3]);
			return true;
		}
		return false;
	}

	value_t get(value_t val) override
	{
		s_scissor = getInt4(GL_SCISSOR_BOX);
		return copyDeep(val, s_scissor);
	}

	void report() override { printInt4("scissor", s_scissor); }
};

class PMinSampleShading : public ParamsFunc {
public:
	PMinSampleShading() = default;
	bool is_deep() override { return false; }

	bool use(value_t val) override
	{
		if (val.f != s_min_sample_shading) {
			s_min_sample_shading = val.f;
			if (s_min_sample_shading > 1.0) {
				s_min_sample_shading = 1.0;
			}
			if (s_min_sample_shading < 0.0) {
				s_min_sample_shading = 0.0;
			}
			glMinSampleShading(s_min_sample_shading);
			return true;
		}
		return false;
	}

	value_t get(value_t) override { return value_t(s_min_sample_shading); }

	void report() override { printFloat("min_sample_shadiing", s_min_sample_shading); }
};

struct ParamsFuncArg {
	ParamsFunc *f;
	void *a;
};

inline std::vector<ParamsFuncArg> createParamsFuncs(const Attrs &attrs)
{
	std::vector<ParamsFuncArg> funcs;
	void *arg = nullptr;

	attrs.peek("clear_color", "deprecated");
	attrs.peek("clear_depth", "deprecated");
	attrs.peek("clear_stencil", "deprecated");

	// probe only
	if ((attrs.get("probe", 0))) {
		auto probe_attrs = attrs;
		probe_attrs.emplace_back("flags.probe", 1);
		auto *func = new PFlags(probe_attrs);
		printf("    func = %p\n", (void *)func);
		delete func;
	}

	// must bet thehighest priority
	if ((arg = attrs.get<void *>("blend_func_channel", nullptr))) {
		auto *func = new PBlendFuncChannel();
		funcs.push_back({func, arg});
	}
	if ((arg = attrs.get<void *>("flags", nullptr))) {  // force if proobe
		auto *func = new PFlags(attrs);
		funcs.push_back({func, arg});
	}
	if ((arg = attrs.get<void *>("write_mask", nullptr))) {
		auto *func = new PWriteMask();
		funcs.push_back({func, arg});
	}
	if ((arg = attrs.get<void *>("blend_func", nullptr))) {
		auto *func = new PBlendFunc();
		funcs.push_back({func, arg});
	}
	if ((arg = attrs.get<void *>("blend_eq", nullptr))) {
		auto *func = new PBlendEq();
		funcs.push_back({func, arg});
	}
	if ((arg = attrs.get<void *>("depth_func", nullptr))) {
		auto *func = new PDepthFunc();
		funcs.push_back({func, arg});
	}
	if ((arg = attrs.get<void *>("stencil_func", nullptr))) {
		auto *func = new PStencilFunc();
		funcs.push_back({func, arg});
	}
	if ((arg = attrs.get<void *>("cull_face", nullptr))) {
		auto *func = new PCullFace();
		funcs.push_back({func, arg});
	}
	if ((arg = attrs.get<void *>("line_width", nullptr))) {
		auto *func = new PLineWidth();
		funcs.push_back({func, arg});
	}
	if ((arg = attrs.get<void *>("point_size", nullptr))) {
		auto *func = new PPointSize();
		funcs.push_back({func, arg});
	}
	if ((arg = attrs.get<void *>("blend_color", nullptr))) {
		auto *func = new PBlendColor();
		funcs.push_back({func, arg});
	}
	if ((arg = attrs.get<void *>("viewport", nullptr))) {
		auto *func = new PViewport();
		funcs.push_back({func, arg});
	}
	if ((arg = attrs.get<void *>("scissor", nullptr))) {
		auto *func = new PScissor();
		funcs.push_back({func, arg});
	}
	if ((arg = attrs.get<void *>("poly_offset", nullptr))) {
		auto *func = new PPolyOffset();
		funcs.push_back({func, arg});
	}
	if ((arg = attrs.get<void *>("line_stipple", nullptr))) {
		auto *func = new PLineStipple();
		funcs.push_back({func, arg});
	}
	if ((arg = attrs.get<void *>("min_sample_shading", nullptr))) {
		auto *func = new PMinSampleShading();
		funcs.push_back({func, arg});
	}
	return funcs;
}

}  // namespace spu::libspu::spu_renderstate
