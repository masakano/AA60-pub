//
// KeyValue :
//
#pragma once
#include <gsys/node.h>

namespace spu::gs_node::gui {

class KeyValue {
public:
	using pair_t = std::pair<std::string, std::string>;

	std::string m_title;  // enclosed by '[' and ']'
	std::vector<std::vector<pair_t>> m_lines;

	KeyValue(const GsNode *node)
	{
		m_title = "[";
		m_title += peeloff_string(node->name());
		while ((node = node->getParent())) {
			m_title += std::string("@") + peeloff_string(node->name());
		}
		m_title += "]";
	}

	void save(File &file)
	{
		file.printf("%s\n", m_title.c_str());
		ColumnAligner aligner("    ", " ");
		for (auto &line: m_lines) {
			std::vector<std::string> items;
			for (auto &pair: line) {
				auto item = pair.first + "=" + pair.second + ",";
				items.emplace_back(item);
			}
			aligner.puts(items);
		}
		auto aligned_lines = aligner.flush();
		for (auto &line: aligned_lines) {
			file.printf("%s", line.c_str());
		}
		file.printf("\n");
	}

	void load(File &file)
	{
		file.rewind();

		auto is_active = false;
		std::string input_line;
		while (file.getline(input_line)) {
			input_line = peeloff_string(input_line);
			if (input_line.empty()) {
				continue;
			}
			if (input_line[0] == '[') {
				is_active = input_line.find(m_title) == 0 ? true : false;
			}
			else if (is_active) {
				auto tokens = extract_from_string(input_line, ",");
				std::vector<pair_t> line;
				for (auto &token: tokens) {
					if (!token.empty()) {
						auto kv = extract_from_string(token, "=");
						assert(kv.size() == 2);
						line.emplace_back(kv[0], kv[1]);
					}
				}
				if (!line.empty()) {
					m_lines.push_back(line);
				}
			}
		}
	}

	void add(int32_t lineno, const std::string &key, const std::string &value)
	{
		if (int32_t(m_lines.size()) <= lineno) m_lines.resize(lineno + 1);
		m_lines[lineno].emplace_back(key, peeloff_string(value));
	}

	void add(int32_t lineno, const std::string &key, const float value)
	{
		add(lineno, key, std::to_string(value));
	}

	void add(int32_t lineno, const std::string &key, const Vec4f &value)
	{
		add(lineno, key, value.to_string("%f %f %f %f").c_str());
	}

	bool get(int lineno, const std::string &key, std::string &value)
	{
		auto is_found = false;
		for (auto &pair: m_lines.at(lineno)) {
			if (pair.first == key) {
				value = pair.second;
				is_found = true;
			}
		}
		return is_found;
	}

	bool get(int lineno, const std::string &key, float &value)
	{
		std::string value_str;
		auto is_found = get(lineno, key, value_str);
		if (is_found) value = std::stof(value_str);
		return is_found;
	}

	bool get(int lineno, const std::string &key, Vec4f &value)
	{
		std::string value_str;
		auto is_found = get(lineno, key, value_str);
		if (is_found) {
			float x, y, z, w;
			sscanf(value_str.c_str(), "%f %f %f %f", &x, &y, &z, &w);
			value.x = x, value.y = y, value.z = z, value.w = w;
		}
		return is_found;
	}
};
}  // namespace spu::gs_node::gui
