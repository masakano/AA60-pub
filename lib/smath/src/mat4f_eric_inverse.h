//
// Matrix4 :
//
#include <smath/mat4f.h>
#include <ssys/random_generator.h>

namespace spu::eric {
struct Matrix4 {
public:
	union {
		float m[4][4];
		__m128 mVec[4];
	};

	Matrix4() = default;
	explicit Matrix4(const Mat4f &m4)
	{
		mVec[0] = m4.c[0].fv;
		mVec[1] = m4.c[1].fv;
		mVec[2] = m4.c[2].fv;
		mVec[3] = m4.c[3].fv;
	}
	explicit operator Mat4f() const
	{
		Mat4f m4;
		m4.c[0].fv = mVec[0];
		m4.c[1].fv = mVec[1];
		m4.c[2].fv = mVec[2];
		m4.c[3].fv = mVec[3];
		return m4;
	}
};

#define MakeShuffleMask(x, y, z, w) (x | (y << 2) | (z << 4) | (w << 6))

// vec(0, 1, 2, 3) -> (vec[x], vec[y], vec[z], vec[w])
#define VecSwizzleMask(vec, mask) _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(vec), mask))
#define VecSwizzle(vec, x, y, z, w) VecSwizzleMask(vec, MakeShuffleMask(x, y, z, w))
#define VecSwizzle1(vec, x) VecSwizzleMask(vec, MakeShuffleMask(x, x, x, x))
// special swizzle
#define VecSwizzle_0022(vec) _mm_moveldup_ps(vec)
#define VecSwizzle_1133(vec) _mm_movehdup_ps(vec)

// return (vec1[x], vec1[y], vec2[z], vec2[w])
#define VecShuffle(vec1, vec2, x, y, z, w) _mm_shuffle_ps(vec1, vec2, MakeShuffleMask(x, y, z, w))
// special shuffle
#define VecShuffle_0101(vec1, vec2) _mm_movelh_ps(vec1, vec2)
#define VecShuffle_2323(vec1, vec2) _mm_movehl_ps(vec2, vec1)

// for column major matrix
// we use __m128 to represent 2x2 matrix as A = | A0  A2 |
//                                              | A1  A3 |
// 2x2 column major Matrix multiply A*B
inline __m128 Mat2Mul(__m128 vec1, __m128 vec2)
{
	return _mm_add_ps(
	        _mm_mul_ps(vec1, VecSwizzle(vec2, 0, 0, 3, 3)),
	        _mm_mul_ps(VecSwizzle(vec1, 2, 3, 0, 1), VecSwizzle(vec2, 1, 1, 2, 2)));
}
// 2x2 column major Matrix adjugate multiply (A#)*B
inline __m128 Mat2AdjMul(__m128 vec1, __m128 vec2)
{
	return _mm_sub_ps(
	        _mm_mul_ps(VecSwizzle(vec1, 3, 0, 3, 0), vec2),
	        _mm_mul_ps(VecSwizzle(vec1, 2, 1, 2, 1), VecSwizzle(vec2, 1, 0, 3, 2)));
}
// 2x2 column major Matrix multiply adjugate A*(B#)
inline __m128 Mat2MulAdj(__m128 vec1, __m128 vec2)
{
	return _mm_sub_ps(
	        _mm_mul_ps(vec1, VecSwizzle(vec2, 3, 3, 0, 0)),
	        _mm_mul_ps(VecSwizzle(vec1, 2, 3, 0, 1), VecSwizzle(vec2, 1, 1, 2, 2)));
}

// Inverse function is the same no matter column major or row major
// this version treats it as column major
inline Matrix4 GetInverse(const Matrix4 &inM)
{
	// use block matrix method
	// A is a matrix, then i(A) or iA means inverse of A, A# (or A_ in code) means adjugate of A, |A| (or
	// detA in code) is determinant, tr(A) is trace

	// sub matrices
	__m128 A = VecShuffle_0101(inM.mVec[0], inM.mVec[1]);
	__m128 C = VecShuffle_2323(inM.mVec[0], inM.mVec[1]);
	__m128 B = VecShuffle_0101(inM.mVec[2], inM.mVec[3]);
	__m128 D = VecShuffle_2323(inM.mVec[2], inM.mVec[3]);

#if 0
	__m128 detA = _mm_set1_ps(inM.m[0][0] * inM.m[1][1] - inM.m[0][1] * inM.m[1][0]);
	__m128 detC = _mm_set1_ps(inM.m[0][2] * inM.m[1][3] - inM.m[0][3] * inM.m[1][2]);
	__m128 detB = _mm_set1_ps(inM.m[2][0] * inM.m[3][1] - inM.m[2][1] * inM.m[3][0]);
	__m128 detD = _mm_set1_ps(inM.m[2][2] * inM.m[3][3] - inM.m[2][3] * inM.m[3][2]);
#else
	// determinant as (|A| |C| |B| |D|)
	__m128 detSub = _mm_sub_ps(
	        _mm_mul_ps(
	                VecShuffle(inM.mVec[0], inM.mVec[2], 0, 2, 0, 2),
	                VecShuffle(inM.mVec[1], inM.mVec[3], 1, 3, 1, 3)),
	        _mm_mul_ps(
	                VecShuffle(inM.mVec[0], inM.mVec[2], 1, 3, 1, 3),
	                VecShuffle(inM.mVec[1], inM.mVec[3], 0, 2, 0, 2)));
	__m128 detA = VecSwizzle1(detSub, 0);
	__m128 detC = VecSwizzle1(detSub, 1);
	__m128 detB = VecSwizzle1(detSub, 2);
	__m128 detD = VecSwizzle1(detSub, 3);
#endif

	// let iM = 1/|M| * | X  Y |
	//                  | Z  W |

	// D#C
	__m128 D_C = Mat2AdjMul(D, C);
	// A#B
	__m128 A_B = Mat2AdjMul(A, B);
	// X# = |D|A - B(D#C)
	__m128 X_ = _mm_sub_ps(_mm_mul_ps(detD, A), Mat2Mul(B, D_C));
	// W# = |A|D - C(A#B)
	__m128 W_ = _mm_sub_ps(_mm_mul_ps(detA, D), Mat2Mul(C, A_B));

	// |M| = |A|*|D| + ... (continue later)
	__m128 detM = _mm_mul_ps(detA, detD);

	// Y# = |B|C - D(A#B)#
	__m128 Y_ = _mm_sub_ps(_mm_mul_ps(detB, C), Mat2MulAdj(D, A_B));
	// Z# = |C|B - A(D#C)#
	__m128 Z_ = _mm_sub_ps(_mm_mul_ps(detC, B), Mat2MulAdj(A, D_C));

	// |M| = |A|*|D| + |B|*|C| ... (continue later)
	detM = _mm_add_ps(detM, _mm_mul_ps(detB, detC));

	// tr((A#B)(D#C))
	__m128 tr = _mm_mul_ps(A_B, VecSwizzle(D_C, 0, 2, 1, 3));
	tr = _mm_hadd_ps(tr, tr);
	tr = _mm_hadd_ps(tr, tr);
	// |M| = |A|*|D| + |B|*|C| - tr((A#B)(D#C))
	detM = _mm_sub_ps(detM, tr);

	const __m128 adjSignMask = _mm_setr_ps(1.f, -1.f, -1.f, 1.f);
	// (1/|M|, -1/|M|, -1/|M|, 1/|M|)
	__m128 rDetM = _mm_div_ps(adjSignMask, detM);

	X_ = _mm_mul_ps(X_, rDetM);
	Y_ = _mm_mul_ps(Y_, rDetM);
	Z_ = _mm_mul_ps(Z_, rDetM);
	W_ = _mm_mul_ps(W_, rDetM);

	Matrix4 r;

	// apply adjugate and store, here we combine adjugate shuffle and store shuffle
	r.mVec[0] = VecShuffle(X_, Z_, 3, 1, 3, 1);
	r.mVec[1] = VecShuffle(X_, Z_, 2, 0, 2, 0);
	r.mVec[2] = VecShuffle(Y_, W_, 3, 1, 3, 1);
	r.mVec[3] = VecShuffle(Y_, W_, 2, 0, 2, 0);

	return r;
}

Mat4f Inverse(const Mat4f &m) { return Mat4f(GetInverse(Matrix4(m))); }

#if 0
void TestMain()
{
	const auto total_count = 5000000;
	auto frand = RandomGenerator<float>(-10, +10);
	auto max_err = 0.0f;
	
	Mat4f worst_src;
	Mat4f worst_inv;
	
	for (auto i = 0; i < total_count; i++) {
		Mat4f src;
		for (auto i = 0; i < 16; i++) {
			src.data()[i] = frand();
		}
		Mat4f inv = Inverse(src);
		Mat4f u0;
		Mat4f u1 = inv * src;
		float err = 0;
		for (auto i = 0; i < 16; i++) {
			float d0 = u0.data()[i];
			float d1 = u1.data()[i];
			err += (d0 - d1) * (d0 - d1);
		}
		if (err > max_err) {
			max_err = err;
			worst_src = src;
			worst_inv = inv;
		}
		if (i % (total_count / 10) == 0) {
			printf("%d:\n", i);
		}
	}
	auto precise_worst_inv = worst_src.precise_inverse();
	worst_src.report("worst_src");
	worst_inv.report("worst_inv");
	precise_worst_inv.report("precis_worst_inv");
	(worst_inv * worst_src).report("worst_src * worst_inv");
	(precise_worst_inv * worst_src).report("precise_worst_src * worst_inv");
}
#endif

}  // namespace spu::eric
