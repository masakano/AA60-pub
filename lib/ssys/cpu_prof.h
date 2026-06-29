//
// CpuProf :
//
#pragma once
#include "ssys.h"

namespace spu {

class CpuProf {
public:
	void setInterval(int32_t interval) { m_interval = interval; }
	void setTitleInterval(int32_t interval) { m_tinterval = interval; }
	void start();
	void record(const char *s);
	void stop();

private:
	struct Point {
		const char *name = nullptr;
		uint32_t dusec = 0;
		uint32_t count = 0;
	};
	std::vector<Point> m_points;
	uint64_t m_pusec = 0;
	uint64_t m_usec = 0;
	uint32_t m_dusec = 0;
	int32_t m_count = 0;
	int32_t m_tcount = 32;
	int32_t m_interval = 0;
	int32_t m_tinterval = 32;
};
}  // namespace spu
