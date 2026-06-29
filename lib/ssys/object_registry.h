//
// ObjectRegistry :
//
#pragma once
#include "ssys.h"

namespace spu {

template<class base_t> class ObjectRegistry {
public:
	using creator_t = base_t *(*)(const char *);

	template<class app_t> class Creator {
	public:
		Creator(const std::string &name) { addCreator(name, create); }

	private:
		static base_t *create(const char *name) { return new app_t(name); }
	};

	static base_t *create(const char *name)
	{
		if (ms_creators().find(name) == ms_creators().end()) {
			aux_printf("%s: no such a sandbox object. candidates are:\n", name);
			printAppNames();
			aux_abort();
		}
		return ms_creators()[name](name);
	}

	static std::vector<std::string> appNames()
	{
		std::vector<std::string> names;
		for (auto &creator: ms_creators()) {
			names.push_back(creator.first);
		}
		return names;
	}

	static void printAppNames()
	{
		auto names = appNames();
		auto max_width = 0;
		for (auto &name: names) {
			max_width = std::max(max_width, int32_t(name.length()));
		}

		std::string line = "    ";
		auto term_column = get_term_column();
		auto max_line_width = size_t(std::max(term_column - 1, 0));
		for (auto &name: names) {
			if (line.length() + size_t(max_width) > max_line_width) {
				aux_printf("%s\n", line.c_str());
				line = "    ";
			}
			line += name;
			line += std::string(max_width - int32_t(name.length()) + 1, ' ');
		}
		if (line.length() > 4) {
			aux_printf("%s\n", line.c_str());
		}
	}
	static void addCreator(const std::string &name, creator_t creator) { ms_creators()[name] = creator; }

	// need implementation to local cpp
	static std::map<const std::string, creator_t> &ms_creators();
	/* (example)
	static std::map<const std::string, creator_t> &ms_creators();
	{
	        static std::map<const std::string, typename ObjectRegistry<base_t>::creator_t> v;
	        return v;
	}
	*/
};
}  // namespace spu
