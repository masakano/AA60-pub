//
// LevensteinDistance :
//
#pragma once
#include <ssys/ssys.h>

namespace spu {

// https://zenn.dev/uu/articles/17d9839bccacd9
class LevensteinDistance {
public:
	int32_t compare(const std::string &x, const std::string y)
	{
		const auto nx = int32_t(x.size());
		const auto ny = int32_t(y.size());

		assert(nx < c_line_size - 1);
		assert(ny < c_line_size - 1);

		for (auto j = 1; j <= nx; j++) {
			LP[j][0] = j;
		}
		for (auto k = 1; k <= ny; k++) {
			LP[0][k] = k;
		}
		for (auto j = 1; j <= nx; j++) {
			for (auto k = 1; k <= ny; k++) {
				auto m = std::min(LP[j - 1][k] + 1, LP[j][k - 1] + 1);
				if (x[j - 1] == y[k - 1]) {
					m = std::min(m, LP[j - 1][k - 1]);
					LP[j][k] = m;
				}
				else {
					m = std::min(m, LP[j - 1][k - 1] + 1);
					LP[j][k] = m;
				}
			}
		}
		return LP[nx][ny];
	}

private:
	static constexpr int32_t c_line_size = 256;
	int32_t LP[c_line_size][c_line_size] = {};
};
}  // namespace spu
