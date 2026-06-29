//
// Value :
//
#pragma once

#include <gsys/node.h>
#include <stack>

namespace spu::gs_painter {
class FTWriter;
}

namespace spu::gs_node::dui {

// clang-format off
inline Vec2f centerOf    (const Range2f &r) { return {r.center().x, r.center().y};}
inline Vec2f toprOf      (const Range2f &r) { return {r.center().x, r.p1.y      };}
inline Vec2f bottomOf    (const Range2f &r) { return {r.center().x, r.p0.y      };}
inline Vec2f leftOf      (const Range2f &r) { return {r.p0.x,       r.center().y};}
inline Vec2f rightOf     (const Range2f &r) { return {r.p1.x,       r.center().y};}
inline Vec2f upperLeftOf (const Range2f &r) { return {r.p0.x,       r.p1.y      };}
inline Vec2f lowerLeftOf (const Range2f &r) { return {r.p0.x,       r.p0.y      };}
inline Vec2f upperRightOf(const Range2f &r) { return {r.p1.x,       r.p1.y      };}
inline Vec2f lowerRightOf(const Range2f &r) { return {r.p1.x,       r.p0.y      };}
// clang-format on

class Value {
public:
	Value() = default;

	Value(void *ref) { init(ref); }
	void init(void *ref) { m_ptr = (Union *)ref; }
	bool empty() const { return m_ptr == nullptr; }

	friend bool operator==(const Value &v0, const Value &v1) { return v0.m_ptr == v1.m_ptr; }

	Value operator=(int32_t i)
	{
		m_ptr->i = i;
		return *this;
	}
	Value operator=(float f)
	{
		m_ptr->f = f;
		return *this;
	}

	operator int32_t() const { return m_ptr->i; }
	operator float() const { return m_ptr->f; }

private:
	union Union {
		int32_t i;
		float f;
	};
	Union *m_ptr = nullptr;
};

class DuiNode : public GsNode {
public:
	static constexpr float c_fontSize = 20.0;
	static constexpr float c_lineHeight = c_fontSize * 1.5;

	enum {
		e_none = -3,    // appearance only
		e_active = -2,  // appearance only
		e_inactive = -1,
		e_off = 0,
		e_on = 1,

	};
	enum {
		e_stick_left = 0,
		e_stick_center,
		e_stick_right,
	};

	struct InsideState {
		bool curr = 0;
		bool anchor = 0;
	};

	explicit DuiNode(const char *name = nullptr) : GsNode(name)
	{
		getARange() = {ezero(), ezero()};
		setProperty(e_lazy, 0);
		m_value.init(&m_heap);
	}

	explicit DuiNode(const Attrs &attrs) : DuiNode() { init(attrs); }

	virtual void setColor(const Vec4f &color, uint32_t mask = 0xffff);

	void setBorder(const Vec2f &border) { m_border = border; }
	void setHighlight(int32_t highlight) { m_highlight = highlight; }
	Value &getValue() { return m_value; }

	const Value &getValue() const { return m_value; }
	const SpuTexture &texture() const { return m_texture; }
	const Vec4f &color() const { return m_color; }
	const Vec2f &border() const { return m_border; }
	const InsideState &insideState() const { return m_insideState; }

	void add(DuiNode *element, bool is_expand = true);
	void pile(DuiNode *element, const DuiNode *upper = nullptr, float indent = c_fontSize * 0.5);

	void move(const Vec2f &delta);
	void squash();

	void init(const Attrs &attrs) override;
	void update() override;

	static void startup();
	static void shutdown();

protected:
	inline static SpuGesture ms_gesture;
	inline static Rectf ms_viewport;

	void doRender() override;
	void doDebugRender() override;
	float highlightRate(float active_rate) const;

private:
	InsideState m_insideState;
	Value m_value;
	SpuTexture m_texture;
	Vec4f m_color = eone<Vec4f>();
	Vec2f m_border = Vec2f(0.0);
	int32_t m_heap;
	int32_t m_highlight = e_none;

	inline static SpuShader ms_shader;
	inline static SpuArray ms_array;

	inline static Mat4f u_nodescreen;
	inline static Vec4f u_color;
	inline static Vec2f u_border;
	inline static uint32_t u_texture = 0;
	inline static uint32_t u_debug = 0;

	void coreDraw();
};

class Container : public DuiNode {
public:
	explicit Container(const char *name = nullptr) : DuiNode(name) {}
	explicit Container(const Attrs &attrs) : Container() { init(attrs); }

	void setCondition(const Value &refvar, int32_t condvar);
	bool relink(DuiNode *element);
	bool remove(DuiNode *element);
	void setHotNode(DuiNode *element) { m_hotDuiNode = element; }
	const DuiNode *hotNode() const { return m_hotDuiNode; }

	void update() override;

protected:
	void doRender() override;

private:
	friend class Popup;
	DuiNode *m_hotDuiNode = nullptr;
	Value m_refvar;
	int32_t m_condvar = 0;
};

class Text : public DuiNode {
public:
	struct Vertex {
		float x, y, u, v;
		uint8_t r, g, b, a;
	};

	enum {
		e_left = 0,
		e_right,
		e_center,
		e_top,
		e_bottom,
	};

	enum {
		e_white = 001,
		e_gray = 002,
		e_black = 003,
		e_red = 004,
		e_green = 005,
		e_blue = 006,
		e_preset = 007,
		e_normal = 020,
		e_bold = 021,
	};

	explicit Text(const char *name = nullptr);
	explicit Text(const Attrs &attrs) : Text() { init(attrs); }
	~Text();

	void init(const Attrs &attrs) override;

	void setText(const std::string &text);
	void setAlignH(int32_t align_h);
	void setAlignV(int32_t align_v);
	void setTextColor(const Vec4f &color, uint32_t mask = 0xffff);
	void bake();

	const std::string &text() const { return m_text; }
	int32_t alignH() const { return m_alignH; }
	int32_t alignV() const { return m_alignV; }

protected:
	void doRender() override;

private:
	std::string m_text;
	gs_painter::FTWriter *m_ftwriter = nullptr;

	Range2f m_textRange;
	Vec4f m_textColor = eone<Vec4f>();
	int32_t m_alignH = e_right;
	int32_t m_alignV = e_top;
	bool m_isCached = false;

	void add(
	        std::vector<Vertex> &vertices, const Vec2f &p, const Vec2f &t, const Vec2f &d,
	        const Vec4f &color);

	Range2f alignText();
};

class ValueText : public Text {
public:
	explicit ValueText(const char *name = nullptr) : Text(name) {}
	explicit ValueText(const Attrs &attrs) : ValueText() { init(attrs); }
	void init(const Attrs &attrs) override;

protected:
	void doRender() override;

private:
	float m_prevValue;
	std::string m_format = "%.1f";
	Text *m_value = nullptr;
};

class Button : public DuiNode {
public:
	enum {
		e_push = 1,
		e_slide,
		e_radio,
		e_check,
	};

	explicit Button(const char *name = nullptr /*"dui::Button"*/) : DuiNode(name) {}
	explicit Button(const Attrs &attrs) : Button() { init(attrs); }

	void setEnum(const int32_t enum_value) { m_enum = enum_value; }
	Text *text() const { return m_text; }

	void init(const Attrs &attrs) override;
	void setColor(const Vec4f &color, uint32_t mask = 0xffff) override;
	void update() override;

protected:
	void doRender() override;

private:
	Text *m_text = nullptr;
	DuiNode *m_onNode = nullptr;
	DuiNode *m_offNode = nullptr;
	int32_t m_type = e_radio;
	int32_t m_enum = 1;  // on
};

class Slider : public DuiNode {
public:
	explicit Slider(const char *name = nullptr) : DuiNode(name) {}
	explicit Slider(const Attrs &attrs) : Slider() { init(attrs); }

	void setValue(float value);
	void setColor(const Vec4f &color, uint32_t mask = 0xffff) override;
	void update() override;
	void init(const Attrs &attrs) override;

protected:
	void doRender() override;

private:
	DuiNode *m_emptybar = nullptr;
	DuiNode *m_fullbar = nullptr;
	DuiNode *m_thumb = nullptr;

	Range1f m_valueRange = {0.0, 100.0};
	float m_curr = 0;
	float m_prev = huge();
	void updateBar(float value);
};

class Popup : public Container {
public:
	explicit Popup(const char *name = nullptr) : Container(name) {}

	Popup(Container *contants, Button *button, Button *popper_button);
	void update() override;

private:
	Button *m_button = nullptr;
	Button *m_popper = nullptr;
	Container *m_contents = nullptr;
};

class Window : public Container {
public:
	Window(float width, float height);
	void update() override;

protected:
	void doRender() override;
};

class Tweakbar : public Container {
public:
	explicit Tweakbar(const char *name = nullptr) : Container(name) {}
	explicit Tweakbar(const Attrs &attrs) : Tweakbar() { init(attrs); }

	void init(const Attrs &attrs) override;

	void beginSwitch(const Value &refvar);
	void beginCase(int32_t condvar);

	void endCase();
	void endSwitch();
	void addPadding(float multiplier = 1);
	void addLabel(const char *text, const Attrs &attrs = Attrs());

	Value &addButton(const char *name, int32_t *var, bool is_push_button = false);
	Value &addSlider(const char *name, float *var, float min, float max);
	Value &addEnum(const char *name, int32_t *var, std::vector<const char *> names);
	Value &addMenu(const char *name, int32_t *var, std::vector<const char *> names);

	Vec2f span() const;

private:
	enum State {
		e_none = 0,
		e_inSwitch,
		e_inCase,
	};

	Button *m_tweaktab = nullptr;
	Value m_refvar;
	State m_state = e_none;

	Container *m_current = nullptr;
	DuiNode *m_bottom = nullptr;

	void check(State state);
	Button *makeButton(const char *name, int32_t type = Button::e_check, uint32_t enum_value = 1);
	Slider *makeSlider(float *var, float min, float max);
	ValueText *makeValueText(float *var, const char *name, const char *format = ".1f");
	void doRender() override;
};
}  // namespace spu::gs_node::dui
