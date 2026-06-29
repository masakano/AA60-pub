//
// Cube :
//
#pragma once
#include <spu++/spu++.h>

namespace spu {
class Cube {
public:
	Cube(float u = 0.25)
	{
		const uint16_t indices[] = {0, 1, 2, 2, 1, 3, 2, 3, 4, 4, 3, 5, 4, 5, 6, 6, 5, 7,
		                            6, 7, 0, 0, 7, 1, 6, 0, 2, 2, 4, 6, 7, 5, 3, 7, 3, 1};

		const float positions[] = {
		        -u, -u, -u, -u, +u, -u, +u, -u, -u, +u, +u, -u,
		        +u, -u, +u, +u, +u, +u, -u, -u, +u, -u, +u, +u,
		};

		Attrs attr = {
		        {"a.0", 3},
		};
		m_arrayId = spu_array_new(attr);
		spu_array_send(m_arrayId, positions, 8);
		spu_array_send(m_arrayId, indices, 36, -1, 2);
	}
	virtual ~Cube() { spu_array_delete(m_arrayId); }

	void draw() { spu_array_draw(m_arrayId, GL_TRIANGLES, 0, 36); }

private:
	uint32_t m_arrayId;
};
}  // namespace spu
