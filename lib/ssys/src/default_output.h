//
//
//
#pragma once
#include <ssys/ssys.h>
#include <deque>

namespace spu {
namespace {

constexpr const char *c_ansi_reset = "\x1b[0m";
constexpr const char *c_ansi_fg_black = "\x1b[30m";
constexpr const char *c_ansi_fg_red = "\x1b[31m";
constexpr const char *c_ansi_fg_green = "\x1b[32m";
constexpr const char *c_ansi_fg_yellow = "\x1b[33m";
constexpr const char *c_ansi_fg_blue = "\x1b[34m";
constexpr const char *c_ansi_fg_magenta = "\x1b[35m";
constexpr const char *c_ansi_fg_cyan = "\x1b[36m";
constexpr const char *c_ansi_fg_white = "\x1b[37m";
constexpr const char *c_ansi_bg_black = "\x1b[40m";
constexpr const char *c_ansi_bg_red = "\x1b[41m";
constexpr const char *c_ansi_bg_green = "\x1b[42m";
constexpr const char *c_ansi_bg_yellow = "\x1b[43m";
constexpr const char *c_ansi_bg_blue = "\x1b[44m";
constexpr const char *c_ansi_bg_magenta = "\x1b[45m";
constexpr const char *c_ansi_bg_cyan = "\x1b[46m";
constexpr const char *c_ansi_bg_white = "\x1b[47m";

uint32_t get_repeat_count(const char *msg)
{
	constexpr uint32_t c_history_size = 64;
	static std::deque<uint32_t> s_histories;

	auto hash = hash32_t(msg).value();
	auto repeat_count = 0;
	for (auto &h: s_histories) {
		if (h == hash) repeat_count++;
	}
	if (s_histories.size() == c_history_size) {
		s_histories.pop_front();
	}
	s_histories.push_back(hash);
	return repeat_count;
}

void default_output(int level, const char *time, const char *label, const char *msg)
{
	constexpr uint32_t c_max_repeat_count = 4;
	auto repeat_count = 0;

	if (level < 3 && *label) {
		if ((repeat_count = get_repeat_count(msg)) >= int(c_max_repeat_count)) {
			return;
		}
	}

	auto column = 0;
	if (msg == nullptr || *msg == 0) {
		return;
	}
	if (label && *label) {
		if (time && *time) {
			fputs(c_ansi_fg_green, stdout);
			fputs("[", stdout);
			fputs(time, stdout);
			fputs("] ", stdout);
			column += strlen(time) + 4;
		}

		switch (level) {
		case 0: fputs(c_ansi_fg_yellow, stdout); break;
		case 1: fputs(c_ansi_fg_yellow, stdout); break;
		case 2: fputs(c_ansi_fg_yellow, stdout); break;
		case 3: fputs(c_ansi_fg_cyan, stdout); break;
		case 4: fputs(c_ansi_fg_red, stdout); break;
		default: break;
		}
		fputs("[", stdout);
		fputs(label, stdout);
		fputs("] ", stdout);
		column += strlen(label) + 4;

		fputs(c_ansi_fg_red, stdout);
		fputs(repeat_count == c_max_repeat_count - 1 ? "*" : " ", stdout);
		column += 2;
		fputs(c_ansi_reset, stdout);
	}
	auto pretty_msg = pretty_string(msg, -column);
	fputs(pretty_msg.c_str(), stdout);
}
}  // namespace
}  // namespace spu
