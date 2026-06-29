
namespace spu::oglplus::shapes {

inline const Mat4f &Cage::face_mat(uint32_t face)
{
	assert(face < 6);
	using M = Mat4f;

	// clang-format off
	static M m[6] = {
		M( 0,  0,  1,  0,  0, -1,  0,  0,  1,  0,  0, 0, 0,0,0,1),  //[0]+x
		M( 0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, 0, 0,0,0,1),  //[1]-x
		M(-1,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0, 0, 0,0,0,1),  //[2]+y
		M(-1,  0,  0,  0,  0,  0, -1,  0,  0, -1,  0, 0, 0,0,0,1),  //[3]-y
		M( 1,  0,  0,  0,  0,  1,  0,  0,  0,  0,  1, 0, 0,0,0,1),  //[4]+z
		M(-1,  0,  0,  0,  0,  1,  0,  0,  0,  0, -1, 0, 0,0,0,1)   //[5]-z
	};
	// clang-format on

	return m[face];
}

inline uint32_t Cage::vert_count() const
{
	return 6 * 8 + (m_divs.z - 1) * 2 * 4 * 2 + (m_divs.x - 1) * 4 * 4 * 2 +

	       (m_divs.y - 1) * m_divs.z * 2 * 4 * 2 + (m_divs.z - 1) * m_divs.x * 2 * 4 * 2
	     + (m_divs.y - 1) * m_divs.x * 2 * 4 * 2 +

	       (m_divs.x * m_divs.y) * 32 + (m_divs.x * m_divs.z) * 32 + (m_divs.y * m_divs.z) * 32;
}

inline uint32_t Cage::index_count() const
{
	return 6 * 11 + (m_divs.z - 1) * 2 * 5 * 2 + (m_divs.x - 1) * 4 * 5 * 2 +

	       (m_divs.y - 1) * m_divs.z * 2 * 5 * 2 + (m_divs.z - 1) * m_divs.x * 2 * 5 * 2
	     + (m_divs.y - 1) * m_divs.x * 2 * 5 * 2 +

	       (m_divs.x * m_divs.y) * 40 + (m_divs.x * m_divs.z) * 40 + (m_divs.y * m_divs.z) * 40;
}

inline uint32_t Cage::positions(std::vector<float> &dest) const
{
	dest.resize(vert_count() * 3);
	auto p = dest.begin();

	for (auto f = 0; f != 6; ++f) {
		float sx = face_size(f, 0);
		float sy = face_size(f, 1);
		float sz = face_size(f, 2);

		float bx = face_barw(f, 0);
		float by = face_barw(f, 1);
		float bz = face_barw(f, 1);

		p = write(p, face_vec(f, Vec3f(+bx - sx, +by - sy, sz)));
		p = write(p, face_vec(f, Vec3f(-sx, -sy, sz)));
		p = write(p, face_vec(f, Vec3f(+bx - sx, -by + sy, sz)));
		p = write(p, face_vec(f, Vec3f(-sx, +sy, sz)));
		p = write(p, face_vec(f, Vec3f(-bx + sx, -by + sy, sz)));
		p = write(p, face_vec(f, Vec3f(+sx, +sy, sz)));
		p = write(p, face_vec(f, Vec3f(-bx + sx, +by - sy, sz)));
		p = write(p, face_vec(f, Vec3f(+sx, -sy, sz)));

		auto dx = face_divs(f, 0);
		auto dy = face_divs(f, 1);

		auto hx = (2 * sx - bx * (dx + 1)) / dx;
		auto hy = (2 * sy - by * (dy + 1)) / dy;

		auto xo = -sx + bx;
		for (auto x = 1u; x != dx; ++x) {
			xo += hx;
			p = write(p, face_vec(f, Vec3f(+bx + xo, +by - sy, sz)));
			p = write(p, face_vec(f, Vec3f(+xo, +by - sy, sz)));
			p = write(p, face_vec(f, Vec3f(+bx + xo, -by + sy, sz)));
			p = write(p, face_vec(f, Vec3f(+xo, -by + sy, sz)));
			xo += bx;
		}

		xo = -sx + bx;
		for (auto x = 1u; x != dx; ++x) {
			xo += hx;
			p = write(p, face_vec(f, Vec3f(+xo, +by - sy, sz - bz)));
			p = write(p, face_vec(f, Vec3f(+bx + xo, +by - sy, sz - bz)));
			p = write(p, face_vec(f, Vec3f(+xo, -by + sy, sz - bz)));
			p = write(p, face_vec(f, Vec3f(+bx + xo, -by + sy, sz - bz)));
			xo += bx;
		}

		auto yo = -sy + by;
		for (auto y = 1u; y != dy; ++y) {
			yo += hy;
			xo = -sx;
			for (auto x = 0u; x != dx; ++x) {
				xo += bx;
				p = write(p, face_vec(f, Vec3f(+xo, +yo, sz)));
				p = write(p, face_vec(f, Vec3f(+xo, +by + yo, sz)));
				p = write(p, face_vec(f, Vec3f(+hx + xo, +yo, sz)));
				p = write(p, face_vec(f, Vec3f(+hx + xo, +by + yo, sz)));
				xo += hx;
			}
			yo += by;
		}

		yo = -sy + by;
		for (auto y = 1u; y != dy; ++y) {
			yo += hy;
			xo = -sx;
			for (auto x = 0u; x != dx; ++x) {
				xo += bx;
				p = write(p, face_vec(f, Vec3f(+hx + xo, +yo, sz - bz)));
				p = write(p, face_vec(f, Vec3f(+hx + xo, +by + yo, sz - bz)));
				p = write(p, face_vec(f, Vec3f(+xo, +yo, sz - bz)));
				p = write(p, face_vec(f, Vec3f(+xo, +by + yo, sz - bz)));
				xo += hx;
			}
			yo += by;
		}

		yo = -sy + by;
		for (auto y = 0u; y != dy; ++y) {
			xo = -sx + bx;
			for (auto x = 0u; x != dx; ++x) {
				p = write(p, face_vec(f, Vec3f(+xo, +yo, -bz + sz)));
				p = write(p, face_vec(f, Vec3f(+xo, +yo, +sz)));
				p = write(p, face_vec(f, Vec3f(+xo, +hy + yo, -bz + sz)));
				p = write(p, face_vec(f, Vec3f(+xo, +hy + yo, +sz)));

				p = write(p, face_vec(f, Vec3f(+xo, +hy + yo, -bz + sz)));
				p = write(p, face_vec(f, Vec3f(+xo, +hy + yo, +sz)));
				p = write(p, face_vec(f, Vec3f(+hx + xo, +hy + yo, -bz + sz)));
				p = write(p, face_vec(f, Vec3f(+hx + xo, +hy + yo, +sz)));

				p = write(p, face_vec(f, Vec3f(+hx + xo, +hy + yo, -bz + sz)));
				p = write(p, face_vec(f, Vec3f(+hx + xo, +hy + yo, +sz)));
				p = write(p, face_vec(f, Vec3f(+hx + xo, +yo, -bz + sz)));
				p = write(p, face_vec(f, Vec3f(+hx + xo, +yo, +sz)));

				p = write(p, face_vec(f, Vec3f(+hx + xo, +yo, -bz + sz)));
				p = write(p, face_vec(f, Vec3f(+hx + xo, +yo, +sz)));
				p = write(p, face_vec(f, Vec3f(+xo, +yo, -bz + sz)));
				p = write(p, face_vec(f, Vec3f(+xo, +yo, +sz)));
				xo += hx + bx;
			}
			yo += hy + by;
		}
	}
	assert(p == dest.end());
	return 3;
}

inline uint32_t Cage::normals(std::vector<float> &dest) const
{
	dest.resize(vert_count() * 3);
	auto p = dest.begin();

	for (auto f = 0; f != 6; ++f) {
		auto m = face_mat(f);
		auto t = Vec3f(m.c[0].x, m.c[1].x, m.c[2].x);
		auto b = Vec3f(m.c[0].y, m.c[1].y, m.c[2].y);
		auto n = Vec3f(m.c[0].z, m.c[1].z, m.c[2].z);

		for (auto v = 0; v != 8; ++v) {
			p = write(p, n);
		}

		auto dx = face_divs(f, 0);
		for (auto s = 0; s != 2; ++s) {
			auto sig = s != 0 ? -1.0 : 1.0;
			for (auto x = 1u; x != dx; ++x) {
				for (auto v = 0; v != 4; ++v) {
					p = write(p, n * sig);
				}
			}
		}

		auto dy = face_divs(f, 1);
		for (auto s = 0; s != 2; ++s) {
			auto sig = s != 0 ? -1.0 : 1.0;
			for (auto y = 1u; y != dy; ++y) {
				for (auto x = 0u; x != dx; ++x) {
					for (auto v = 0; v != 4; ++v) {
						p = write(p, n * sig);
					}
				}
			}
		}

		for (auto y = 0u; y != dy; ++y) {
			for (auto x = 0u; x != dx; ++x) {
				for (auto s = 0; s != 4; ++s) {
					p = write(p, t);
				}
				for (auto s = 0; s != 4; ++s) {
					p = write(p, -b);
				}
				for (auto s = 0; s != 4; ++s) {
					p = write(p, -t);
				}
				for (auto s = 0; s != 4; ++s) {
					p = write(p, b);
				}
			}
		}
	}
	assert(p == dest.end());
	return 3;
}

inline uint32_t Cage::tangents(std::vector<float> &dest) const
{
	dest.resize(vert_count() * 3);
	auto p = dest.begin();

	for (auto f = 0; f != 6; ++f) {
		auto m = face_mat(f);
		auto t = Vec3f(m.c[0].x, m.c[1].x, m.c[2].x);
		auto n = Vec3f(m.c[0].z, m.c[1].z, m.c[2].z);

		for (auto v = 0; v != 8; ++v) {
			p = write(p, t);
		}

		auto dx = face_divs(f, 0);
		for (auto s = 0; s != 2; ++s) {
			auto sig = s != 0 ? -1.0 : 1.0;
			for (auto x = 1u; x != dx; ++x) {
				for (auto v = 0; v != 4; ++v) {
					p = write(p, t * sig);
				}
			}
		}

		auto dy = face_divs(f, 1);
		for (auto s = 0; s != 2; ++s) {
			auto sig = s != 0 ? -1.0 : 1.0;
			for (auto y = 1u; y != dy; ++y) {
				for (auto x = 0u; x != dx; ++x) {
					for (auto v = 0; v != 4; ++v) {
						p = write(p, t * sig);
					}
				}
			}
		}

		for (auto y = 0u; y != dy; ++y) {
			for (auto x = 0u; x != dx; ++x) {
				for (auto s = 0; s != 16; ++s) {
					p = write(p, -n);
				}
			}
		}
	}
	assert(p == dest.end());
	return 3;
}

inline uint32_t Cage::texCoordinates(std::vector<float> &dest) const
{
	dest.resize(vert_count() * 3);
	auto p = dest.begin();

	for (auto fi = 0; fi != 6; ++fi) {
		auto f = double(fi);
		auto z = 0.0;
		auto o = 1.0;

		auto bx = 0.5 * face_barw(f, 0) / face_size(f, 0);
		auto by = 0.5 * face_barw(f, 1) / face_size(f, 1);

		p = write(p, face_vec(f, Vec3f(+bx + z, +by + z, f)));
		p = write(p, face_vec(f, Vec3f(+z, +z, f)));
		p = write(p, face_vec(f, Vec3f(+bx + z, -by + o, f)));
		p = write(p, face_vec(f, Vec3f(+z, +o, f)));
		p = write(p, face_vec(f, Vec3f(-bx + o, -by + o, f)));
		p = write(p, face_vec(f, Vec3f(+o, +o, f)));
		p = write(p, face_vec(f, Vec3f(-bx + o, +by + z, f)));
		p = write(p, face_vec(f, Vec3f(+o, +z, f)));

		auto dx = face_divs(f, 0);
		auto dy = face_divs(f, 1);

		auto hx = (1.0 - bx * (dx + 1)) / dx;
		auto hy = (1.0 - by * (dy + 1)) / dy;

		auto xo = bx;
		for (auto x = 1u; x != dx; ++x) {
			xo += hx;
			p = write(p, face_vec(f, Vec3f(+bx + xo, +by + z, f)));
			p = write(p, face_vec(f, Vec3f(+xo, +by + z, f)));
			p = write(p, face_vec(f, Vec3f(+bx + xo, -by + o, f)));
			p = write(p, face_vec(f, Vec3f(+xo, -by + o, f)));
			xo += bx;
		}

		xo = bx;
		for (auto x = 1u; x != dx; ++x) {
			xo += hx;
			p = write(p, face_vec(f, Vec3f(+xo, +by + z, f)));
			p = write(p, face_vec(f, Vec3f(+bx + xo, +by + z, f)));
			p = write(p, face_vec(f, Vec3f(+xo, -by + o, f)));
			p = write(p, face_vec(f, Vec3f(+bx + xo, -by + o, f)));
			xo += bx;
		}

		auto yo = by;
		for (auto y = 1u; y != dy; ++y) {
			yo += hy;
			xo = z;
			for (auto x = 0u; x != dx; ++x) {
				xo += bx;
				p = write(p, face_vec(f, Vec3f(+xo, +yo, f)));
				p = write(p, face_vec(f, Vec3f(+xo, +by + yo, f)));
				p = write(p, face_vec(f, Vec3f(+hx + xo, +yo, f)));
				p = write(p, face_vec(f, Vec3f(+hx + xo, +by + yo, f)));
				xo += hx;
			}
			yo += by;
		}

		yo = by;
		for (auto y = 1u; y != dy; ++y) {
			yo += hy;
			xo = z;
			for (auto x = 0u; x != dx; ++x) {
				xo += bx;
				p = write(p, face_vec(f, Vec3f(+hx + xo, +yo, f)));
				p = write(p, face_vec(f, Vec3f(+hx + xo, +by + yo, f)));
				p = write(p, face_vec(f, Vec3f(+xo, +yo, f)));
				p = write(p, face_vec(f, Vec3f(+xo, +by + yo, f)));
				xo += hx;
			}
			yo += by;
		}

		yo = by;
		for (auto y = 0u; y != dy; ++y) {
			xo = bx;
			for (auto x = 0u; x != dx; ++x) {
				p = write(p, face_vec(f, Vec3f(+xo, +yo, f)));
				p = write(p, face_vec(f, Vec3f(+xo, +yo, f)));
				p = write(p, face_vec(f, Vec3f(+xo, +hy + yo, f)));
				p = write(p, face_vec(f, Vec3f(+xo, +hy + yo, f)));

				p = write(p, face_vec(f, Vec3f(+xo, +hy + yo, f)));
				p = write(p, face_vec(f, Vec3f(+xo, +hy + yo, f)));
				p = write(p, face_vec(f, Vec3f(+hx + xo, +hy + yo, f)));
				p = write(p, face_vec(f, Vec3f(+hx + xo, +hy + yo, f)));

				p = write(p, face_vec(f, Vec3f(+hx + xo, +hy + yo, f)));
				p = write(p, face_vec(f, Vec3f(+hx + xo, +hy + yo, f)));
				p = write(p, face_vec(f, Vec3f(+hx + xo, +yo, f)));
				p = write(p, face_vec(f, Vec3f(+hx + xo, +yo, f)));

				p = write(p, face_vec(f, Vec3f(+hx + xo, +yo, f)));
				p = write(p, face_vec(f, Vec3f(+hx + xo, +yo, f)));
				p = write(p, face_vec(f, Vec3f(+xo, +yo, f)));
				p = write(p, face_vec(f, Vec3f(+xo, +yo, f)));
				xo += hx + bx;
			}
			yo += hy + by;
		}
	}
	assert(p == dest.end());
	return 3;
}

inline Cage::IndexArray Cage::indices(Cage::DefaultTag /*unused*/) const
{
	IndexArray indices(index_count(), restartIndex());
	auto i = indices.begin();

	auto offs = 0;

	for (auto f = 0; f != 6; ++f) {
		for (auto v = 0; v != 10; ++v) {
			*i++ = offs + v % 8;
		}

		offs += 8;
		++i;

		auto dx = face_divs(f, 0);
		for (auto s = 0; s != 2; ++s) {
			for (auto x = 1u; x != dx; ++x) {
				for (auto v = 0; v != 4; ++v) {
					*i++ = offs + v;
				}
				offs += 4;
				++i;
			}
		}

		auto dy = face_divs(f, 1);
		for (auto s = 0; s != 2; ++s) {
			for (auto y = 1u; y != dy; ++y) {
				for (auto x = 0u; x != dx; ++x) {
					for (auto v = 0; v != 4; ++v) {
						*i++ = offs + v;
					}
					offs += 4;
					++i;
				}
			}
		}

		for (auto y = 0u; y != dy; ++y) {
			for (auto x = 0u; x != dx; ++x) {
				for (auto s = 0; s != 4; ++s) {
					for (auto w = 0; w != 4; ++w) {
						*i++ = offs + w;
					}
					offs += 4;
					++i;
				}
			}
		}
	}
	assert(i == indices.end());
	return indices;
}

inline std::vector<spu::SpuCommand> Cage::instructions(Cage::DefaultTag /*unused*/) const
{
	spu::SpuCommand com;
	com.target = GL_ELEMENT_ARRAY_BUFFER;
	com.mode = GL_TRIANGLE_STRIP;
	com.first = 0;
	com.count = index_count();
	com.flags = 0;

	return {com};
}

}  // namespace spu::oglplus::shapes
