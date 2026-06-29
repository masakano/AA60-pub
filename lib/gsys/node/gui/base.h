//
// Base :
//
#pragma once
#include <gsys/canvas.h>
#include <gsys/node.h>
#include <gsys/painter/sprite.h>
#include <gsys/painter/text.h>

namespace spu {
namespace gs_node::gui {

class Base : public GsNode {
public:
	static constexpr int32_t e_class_depth = GsNode::e_class_depth + 1;

	class WindowModifier {
	public:
		WindowModifier(Base *) {}
		void fold(bool is_immediate);
		void unfold(bool is_immediate);
		void reverse(bool is_immediate);
		void update();
		float rate() const { return m_rate; }
		float speed() const { return m_speed; }
		uint32_t count() const { return m_count; }

	private:
		float m_speed = 0.0;
		float m_rate = 0.0;
		uint32_t m_count = 0;
	};

	static constexpr float c_charwidth = 1.0;
	static constexpr float c_charheight = 2.0;
	static constexpr float c_charscale = 8.0;

	static constexpr hash32_t e_front = "front";
	static constexpr hash32_t e_title = "title";
	static constexpr hash32_t e_disabled = "disabled";
	static constexpr hash32_t e_inactive = "inactive";
	static constexpr hash32_t e_active = "active";
	static constexpr hash32_t e_featured = "feattured";
	static constexpr hash32_t e_background = "background";

	static constexpr hash32_t e_guide = "guide";
	static constexpr hash32_t e_bar = "bar";
	static constexpr hash32_t e_button = "button";

	explicit Base(const char *name = nullptr);
	explicit Base(const Attrs &attrs) : Base() { init(attrs); }

	virtual int32_t getColumns(const char *title = nullptr, Base *parent = nullptr) const;
	virtual void save(File &file) const;
	virtual void load(File &file);

	virtual bool isFocus(const GsCanvas *current = nullptr) const;
	virtual bool isInside(const Vec2f &cursor) const;

	virtual void changeState(const std::vector<const void *> &ptrs, const hash32_t &state);
	virtual std::map<hash32_t, Vec4f> &getColors() { return m_colors; }

	void init(const Attrs &attrs) override;
	void update() override;
	uint32_t lastUpdateCount() const override { return ms_lastUpdateCount; }

	WindowModifier &getWindowModifier() { return m_windowModifier; }
	const WindowModifier &getWindowModifier() const { return m_windowModifier; }

	Range2f windowRange() const;
	Range2f scissorRange(const GsCanvas *current = nullptr) const;
	Range2f headerRange() const;

	static SpuGesture *getGesture();
	static std::string padstr(const std::string &item, int32_t length);
	static std::vector<std::string> normalizestr(
	        const std::vector<const char *> &items, const int32_t indent, const int32_t length);
	static bool grab();
	static bool ungrab();

protected:
	template<class value_t> struct State {
		std::string text;
		value_t *value_ptr = nullptr;
		hash32_t state = "active"_h32;
		bool is_featured = false;
	};

	struct Graphic {
		hash32_t name;
		GsDrawcall drawcall;
		std::vector<gs_painter::Sprite::Vertex> vertices;
	};

	struct TextGraphic {
		hash32_t name;
		std::vector<gs_painter::Text::Vertex> vertices;
	};

	WindowModifier m_windowModifier;
	inline static uint32_t ms_lastUpdateCount;

	void doRender() override;

	Graphic &getGraphic(const hash32_t &name);
	Graphic &newGraphic(const hash32_t &name);

	Mat4f getNodefrag(const GsCanvas *current = nullptr) const;
	Vec2f getCursor(const int16_t icursor[2]) const;

	std::vector<Graphic> &getGraphics() { return m_graphics; }
	uint32_t count() const { return m_count; }

	template<class S, class value_t> void bakeTextInternal(const std::vector<S> &states)
	{
		static_assert(std::is_base_of<State<value_t>, S>::value, "not derived from Base::State");
		clearText();
		for (auto &s: states) {
			auto state = s.state == e_disabled ? e_disabled : e_active;  // force to active
			setTextColor(getColors()[state]);
			putText(s.text);
		}
		updateText();
	}
	template<class S, class value_t>
	void changeStateInternal(
	        std::vector<S> &states, const std::vector<const void *> &ptrs, const hash32_t &state)
	{
		static_assert(std::is_base_of<State<value_t>, S>::value, "not derived from Base::State");
		for (auto &s: states) {
			if (s.state != e_featured) {  // don't change when featured
				for (const auto &ptr: ptrs) {
					if (s.value_ptr == ptr) {
						s.state = state;
					}
				}
			}
		}
		bakeTextInternal<S, value_t>(states);
	}

private:
	std::map<hash32_t, Vec4f> m_colors;
	std::vector<Graphic> m_graphics;
	TextGraphic m_textGraphic;
	std::vector<int32_t> m_columns;
	uint32_t m_count = 0;

	inline static gs_painter::Text *ms_text = nullptr;
	inline static gs_painter::Sprite *ms_sprite = nullptr;
	inline static SpuGesture ms_gesture;

	void clearText();
	void setTextColor(const Vec4f &color);
	void putText(const std::string &text);
	void updateText();

	friend ClassCreator<Base>;
	static void startup(const Attrs &);
	static void shutdown();
	static ClassCreator<Base> ms_classCreator;
};
}  // namespace gs_node::gui
}  // namespace spu
