#pragma once
#include <ssys/ssys.h>

namespace spu::oglplus {
inline std::string replaceText(const char *c_text, const Attrs &attrs)
{
	auto text = std::string(c_text);
	for (auto &attr: attrs) {
		auto symbol = std::string(attr.key().c_str());
		auto value = attr.toString();
		for (auto pos = text.find(symbol); pos != std::string::npos; pos = text.find(symbol, pos + value.size())) {
			text.replace(pos, symbol.size(), value);
		}
	}
	return text;
}
}
