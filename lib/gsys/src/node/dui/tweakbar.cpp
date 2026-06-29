//
// Tweakbar :
//
#include <algorithm>
#include <cstdio>
#include <gsys/node/dui.h>
#include <smath/color_chart.h>

namespace spu::gs_node::dui {
namespace {

const float c_padFactor = 0.33;
// const uint32_t c_nLine  = 22;

const Vec4f c_white = {1.0, 1.0, 1.0, 1.0};
const Vec4f c_yellow = {1.0, 0.8, 0.3, 1.0};
const Vec4f c_darkyellow = {0.5, 0.4, 0.15, 1.0};
const Vec4f c_blue = {0.3, 0.8, 1.0, 1.0};
const Vec4f c_limegreen = {0.3, 1.0, 0.8, 1.0};
}  // namespace

void Tweakbar::init(const Attrs &attrs)
{
	attrs.peek("text", "use 'title' instead");

	auto *title = attrs.get("title", "no title");
	auto *window = attrs.get<Window *>("window", nullptr);
	auto ox = attrs.get("ox", 0);
	auto oy = attrs.get("oy", 0);
	auto sx = attrs.get("sx", 320);

	// tweakbar
	{
		auto span = Vec2f(sx, 0);  // start with zero
		auto color = Vec4f(0.5, 0.5, 1.0, 1.0);

		Attrs attrs = {
		        {"texture_path", "frame_thin.dds"},
		        {"span",         span            },
		        {"color",        color           },
		        {"border",       8.0             },
		};
		Container::init(attrs);
		m_current = this;

		auto to = upperLeftOf(window->getARange()) + Vec2f(ox, oy);
		Container::move(to);

		window->add(this);
	}

	// tweaktab
	{
		// auto height = span().y;
		auto button_span = span() - Vec2f(c_fontSize, 0);
		auto element_span = Vec2f(c_lineHeight * 0.5);

		Attrs off_node_attrs = {
		        {"span",         element_span    },
		        {"texture_path", "arrow_blue.dds"},
		};

		Attrs on_node_attrs = {
		        {"span",         element_span         },
		        {"texture_path", "arrow_blue_left.dds"},
		};

		Attrs text_attrs = {
		        {"text",         title                },
		        {"span",         button_span          },
		        {"texture_path", "frame_thin.dds"     },
		        {"border",       8.0                  },
		        {"text_color",   color_by_name("lime")},
		        {"align_h",      Text::e_center       },
		        {"align_v",      Text::e_center       },
		};

		auto text = new Text(text_attrs);
		auto off_node = new DuiNode(off_node_attrs);
		auto on_node = new DuiNode(on_node_attrs);

		Attrs attrs = {
		        {"type",     Button::e_check},
                        {"span",     button_span    },
                        {"text",     text           },
		        {"border",   12.0           },
                        {"off_node", off_node       },
                        {"on_node",  on_node        },
		};
		m_tweaktab = new Button(attrs);
		m_tweaktab->getValue() = true;

		auto range = Range2f(Vec2f(0), button_span);
		auto from = upperLeftOf(range);
		auto to = upperLeftOf(getARange());

		m_tweaktab->move(to - from);
		add(m_tweaktab);
	}
}

Vec2f Tweakbar::span() const { return {m_current->getARange().span().x - c_fontSize, c_lineHeight}; }

void Tweakbar::doRender()
{
	if (int32_t(m_tweaktab->getValue())) {
		Container::doRender();
	}
	else {
		m_tweaktab->render();  // tweaktab only
	}
}

void Tweakbar::check(State state)
{
	aux_error(m_state != state, "switch-case mismatch (current=%d disired=%d)\n", m_state, state);
}

void Tweakbar::beginSwitch(const Value &refvar)
{
	check(e_none);
	m_refvar = refvar;
	m_state = e_inSwitch;
}

void Tweakbar::beginCase(int32_t condvar)
{
	check(e_inSwitch);
	auto span = Vec2f(this->span().x, c_fontSize * 0.5);  // add top margin
	auto color = Vec4f(1.0, 1.0, 1.0, 0.2);

	Attrs attrs = {
	        {"span",         span            },
	        {"texture_path", "frame_thin.dds"},
	        {"color",        color           },
	        {"border",       12.0            },
	};

	auto *container = new Container(attrs);
	container->setCondition(m_refvar, condvar);

	if (m_bottom == nullptr) {
		m_bottom = dynamic_cast<DuiNode *>(getChildren().back());  // tricky
	}
	m_current = container;
	m_state = e_inCase;
}

void Tweakbar::endCase()
{
	check(e_inCase);

	pile(m_current, m_bottom, 0);
	m_current = this;
	m_state = e_inSwitch;
}

void Tweakbar::endSwitch()
{
	check(e_inSwitch);
	m_bottom = nullptr;
	m_state = e_none;
}

void Tweakbar::addPadding(float multiplier)
{
	auto &range = m_current->getARange();
	range.p0.y -= multiplier * c_padFactor * c_lineHeight;
}

void Tweakbar::addLabel(const char *text, const Attrs &attrs)
{
	Attrs label_attrs = {
	        {"text",       text          },
	        {"align_h",    Text::e_left  },
	        {"align_v",    Text::e_center},
	        {"text_color", c_yellow      },
	};
	m_current->pile(new Text(label_attrs + attrs));
}

Value &Tweakbar::addEnum(const char *name, int32_t *var, std::vector<const char *> names)
{
	addLabel(name);

	Attrs attrs = {
	        {"span", Vec2f(span().x - c_fontSize, 0)},
	};
	auto *container = new Container(attrs);

	container->getValue().init(var);

	for (auto i = 0u; i < names.size(); i++) {
		auto *button = makeButton(names[i], Button::e_radio, i);
		container->pile(button, container, 0);
	}
	m_current->pile(container);
	return container->getValue();
}

Value &Tweakbar::addMenu(const char *name, int32_t *var, std::vector<const char *> names)
{
	Button *button;
	{
		auto button_span = span();

		Attrs text_attrs = {
		        {"text",    name          },
		        {"align_h", Text::e_center},
		        {"align_v", Text::e_center},
		};

		Attrs off_node_attrs = {
		        {"span",         button_span     },
		        {"texture_path", "frame_thin.dds"},
		        {"color",        c_white         },
		        {"border",       4.0             },
		};

		Attrs on_node_attrs = {
		        {"span",         button_span     },
		        {"texture_path", "frame_thin.dds"},
		        {"color",        c_white         },
		        {"border",       4.0             },
		};

		auto text = new Text(text_attrs);
		auto off_node = new DuiNode(off_node_attrs);
		auto on_node = new DuiNode(on_node_attrs);

		Attrs button_attrs = {
		        {"type",     int32_t(Button::e_push)},
		        {"text",     text                   },
		        {"off_node", off_node               },
		        {"on_node",  on_node                },
		};
		button = new Button(button_attrs);
	}

	Button *popper;
	{
		// const auto popper_span = Vec2f(span().y * 0.5F);
		const auto popper_span = Vec2f(c_lineHeight * 0.5);

		Attrs off_node_attrs = {
		        {"span",         popper_span          },
		        {"texture_path", "arrow_blue_left.dds"},
		};
		Attrs on_node_attrs = {
		        {"span",         popper_span             },
		        {"texture_path", "arrow_pressed_down.dds"},
		};

		auto off_node = new DuiNode(off_node_attrs);
		auto on_node = new DuiNode(on_node_attrs);

		Attrs popper_attrs = {
		        {"type",     Button::e_check},
		        {"off_node", off_node       },
		        {"on_node",  on_node        },
		};
		popper = new Button(popper_attrs);
	}

	Container *contents;
	{
		auto contents_span = Vec2f(span().x, 0);  // start with zero
		Attrs contents_attrs = {
		        {"span",         contents_span           },
		        {"texture_path", "info_text_box_thin.dds"},
		        {"color",        c_white                 },
		        {"border",       4.0                     },
		};

		contents = new Container(contents_attrs);

		for (auto &name: names) {
			auto element_span = span();
			auto off_node_color = Vec4f(c_white, 0.0);
			auto on_node_color = Vec4f(c_darkyellow, 0.5);

			Attrs text_attrs = {
			        {"text",    name          },
			        {"align_h", Text::e_center},
			        {"align_v", Text::e_center},
			};

			Attrs off_node_attrs = {
			        {"span",         element_span                     },
			        {"texture_path", "icon_button_highlight_small.dds"},
			        {"color",        off_node_color                   },
			        {"border",       8.0                              },
			};

			Attrs on_node_attrs = {
			        {"span",         element_span                     },
			        {"texture_path", "icon_button_highlight_small.dds"},
			        {"color",        on_node_color                    },
			        {"border",       8.0                              },
			};

			auto *text = new Text(text_attrs);
			auto *off_node = new DuiNode(off_node_attrs);
			auto *on_node = new DuiNode(on_node_attrs);

			Attrs attrs = {
			        {"type",     Button::e_slide  },
                                {"enum",     &name - &names[0]},
                                {"text",     text             },
			        {"off_node", off_node         },
                                {"on_node",  on_node          },
			};
			auto *button = new Button(attrs);

			auto contents_range = contents->getARange();
			auto button_range = button->getARange();

			auto from = upperLeftOf(button_range);
			auto to = lowerLeftOf(contents_range);
			button->move(to - from);
			contents->add(button);
		}
		contents->getValue().init(var);
	}
	auto *popup = new Popup(contents, button, popper);

	m_current->pile(popup);
	return popup->getValue();
}

Value &Tweakbar::addButton(const char *name, int32_t *var, bool is_push_button)
{
	auto *button = makeButton(name, is_push_button ? Button::e_push : Button::e_check, 1);
	button->getValue().init(var);
	m_current->pile(button);
	return button->getValue();
}

Value &Tweakbar::addSlider(const char *name, float *var, float min, float max)
{
	auto *slider = makeSlider(var, min, max);
	slider->setName(name);

	// valuetext
	const auto *format = ((max > 100) ? "%.1f" : ((max > 10) ? "%.2f" : "%.3f"));
	auto *valuetext = makeValueText(var, name, format);
	m_current->pile(valuetext);
	m_current->pile(slider);
	return slider->getValue();
}

Button *Tweakbar::makeButton(const char *name, int32_t type, uint32_t enum_value)
{
	Attrs off_node_attrs;
	Attrs on_node_attrs;

	switch (type) {
	case Button::e_push: {
		auto push_span = span();
		off_node_attrs = {
		        {"span",         push_span           },
		        {"texture_path", "btn_round_blue.dds"},
		        {"border",       8.0                 },
		};

		on_node_attrs = {
		        {"span",         push_span              },
		        {"texture_path", "btn_round_pressed.dds"},
		        {"border",       8.0                    },
		};
		break;
	}
	case Button::e_check: {
		auto check_span = Vec2f(c_lineHeight);
		off_node_attrs = {
		        {"span",         check_span        },
		        {"texture_path", "btn_box_blue.dds"},
		};
		on_node_attrs = {
		        {"span",         check_span             },
		        {"texture_path", "btn_box_pressed_x.dds"},
		};
		break;
	}
	case Button::e_radio: {
		auto radio_span = Vec2f(c_lineHeight * 0.5f);
		off_node_attrs = {
		        {"span",         radio_span          },
		        {"texture_path", "button_top_row.dds"},
		};
		on_node_attrs = {
		        {"span",         radio_span                  },
		        {"texture_path", "button_top_row_pressed.dds"},
		};
		break;
	}
	default: assert(0);
	}

	auto align_h = type == Button::e_push ? Text::e_center : Text::e_left;
	Attrs text_attrs = {
	        {"text",    name          },
	        {"align_h", align_h       },
	        {"align_v", Text::e_center},
	};

	auto *text = new Text(text_attrs);
	auto element_span = span();
	auto *off_node = new DuiNode(off_node_attrs);
	auto *on_node = new DuiNode(on_node_attrs);

	Attrs attrs = {
	        {"type",     type        },
                {"enum",     enum_value  },
                {"span",     element_span},
	        {"text",     text        },
                {"off_node", off_node    },
                {"on_node",  on_node     },
	};
	return new Button(attrs);
}

Slider *Tweakbar::makeSlider(float *var, float min, float max)
{
	auto bar_span = Vec2f(span().x, c_lineHeight * 0.5);
	auto thumb_span = Vec2f(c_lineHeight * 0.5, c_lineHeight * 1.25);

	Attrs emptybar_attrs = {
	        {"span",         bar_span          },
	        {"texture_path", "slider_empty.dds"},
	        {"color",        c_white           },
	        {"border",       4.0               },
	};
	Attrs fullbar_attrs = {
	        {"span",         bar_span         },
	        {"texture_path", "slider_full.dds"},
	        {"color",        c_yellow         },
	        {"border",       4.0              },
	};
	Attrs thumb_attrs = {
	        {"span",         thumb_span        },
	        {"texture_path", "slider_thumb.dds"},
	};

	auto *emptybar = new DuiNode(emptybar_attrs);
	auto *fullbar = new DuiNode(fullbar_attrs);
	auto *thumb = new DuiNode(thumb_attrs);

	Attrs attrs = {
	        {"span",     bar_span},
                {"min",      min     },
                {"max",      max     },
	        {"emptybar", emptybar},
                {"fullbar",  fullbar },
                {"thumb",    thumb   },
	};
	auto *slider = new Slider(attrs);
	slider->getValue().init(var);
	return slider;
}

ValueText *Tweakbar::makeValueText(float *var, const char *name, const char *format)
{
	Attrs attrs = {
	        {"span",             span()        },
	        {"format",           format        },
	        {"text",             name          },
	        {"align_h",          Text::e_left  },
	        {"align_v",          Text::e_center}, // need check
	        {"text_color",       c_limegreen   },
	        {"value.text",       "0"           },
	        {"value.align_h",    Text::e_right },
	        {"value.align_v",    Text::e_center}, // need check
	        {"value.text_color", c_blue        },
	};

	auto *valuetext = new ValueText(attrs);
	valuetext->getValue().init(var);
	return valuetext;
}
}  // namespace spu::gs_node::dui
