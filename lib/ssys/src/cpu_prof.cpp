//
// CpuProf :
//
#include <ssys/cpu_prof.h>

namespace spu {

void CpuProf::start()
{
	if (m_interval > 0) {
		m_pusec = m_usec = get_microsec();
		// m_enterCount++;
	}
}

void CpuProf::record(const char *s)
{
	if (/*m_enterCount > 0 && */ m_interval > 0) {
		auto det = [&s](const Point &p) { return strcmp(s, p.name) == 0; };
		auto it = vector_find_if(m_points, det);
		auto usec = get_microsec();

		if (it == end(m_points)) {
			Point p;
			p.name = s;
			p.dusec = usec - m_pusec;
			p.count = 1;
			m_points.push_back(p);
		}
		else {
			it->dusec += usec - m_pusec;
			it->count++;
		}
		m_pusec = usec;
	}
}

void CpuProf::stop()
{
	if (m_interval > 0) {
		/*
		if (--m_enterCount > 0) {
		        return;
		}
		*/
		m_dusec += get_microsec() - m_usec;
		if (++m_count >= m_interval) {
			if (++m_tcount >= m_tinterval) {
				std::vector<std::vector<std::string>> title_list;
				size_t n = 0;
				for (auto point: m_points) {
					title_list.push_back(
					        extract_from_string(std::string(point.name), " ."));
					n = std::max(n, title_list.back().size());
				}

				for (auto i = 0u; i < (m_points.size() + 1) * 10; i++) {
					aux_printf("-");
				}
				aux_printf("\n");

				for (auto i = 0u; i < n; i++) {
					for (auto &titles: title_list) {
						if (i < titles.size()) {
							aux_printf("%9s ", titles[i].c_str());
						}
						else {
							aux_printf("%9s ", "");
						}
					}
					aux_printf("\n");
				}
				for (auto i = 0u; i < (m_points.size() + 1) * 10; i++) {
					aux_printf("-");
				}
				aux_printf("\n");
				m_tcount = 0;
			}
			for (auto &point: m_points) {
				aux_printf("%9d ", point.count > 0 ? point.dusec / point.count : 0);
			}
			aux_printf(":%9d\n", m_dusec / m_count);

			for (auto &point: m_points) {
				point.count = 0;
				point.dusec = 0;
			}
			m_count = 0;
			m_dusec = 0;
		}
	}
}
}  // namespace spu
