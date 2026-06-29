//
// HalfFloat :
//
#include <ssys/half_float.h>

namespace spu {
namespace {
union Bits {
	float f;
	int32_t si;
	uint32_t ui;
};

constexpr int32_t shift = 13;
constexpr int32_t shiftSign = 16;

constexpr int32_t infN = 0x7F800000;   // flt32 infinity
constexpr int32_t maxN = 0x477FE000;   // max flt16 normal as a flt32
constexpr int32_t minN = 0x38800000;   // min flt16 normal as a flt32
constexpr int32_t signN = 0x80000000;  // flt32 sign bit

constexpr int32_t infC = infN >> shift;
constexpr int32_t nanN = (infC + 1) << shift;  // minimum flt16 nan as a flt32
constexpr int32_t maxC = maxN >> shift;
constexpr int32_t minC = minN >> shift;
constexpr int32_t signC = signN >> shiftSign;  // flt16 sign bit

constexpr int32_t mulN = 0x52000000;  // (1 << 23) / minN
constexpr int32_t mulC = 0x33800000;  // minN / (1 << (23 - shift))

constexpr int32_t subC = 0x003FF;  // max flt32 subnormal down shifted
constexpr int32_t norC = 0x00400;  // min flt32 normal down shifted

constexpr int32_t maxD = infC - maxC - 1;
constexpr int32_t minD = minC - subC - 1;

}  // namespace

uint16_t HalfFloat::to_half(float value) const
{
	// NoFloatingException fpe_except;
	Bits v;
	Bits s;
	v.f = value;
	uint32_t sign = v.si & signN;
	v.si ^= sign;
	sign >>= shiftSign;  // logical shift
	s.si = mulN;
	s.si = s.f * v.f;  // correct subnormals
	v.si ^= (s.si ^ v.si) & -int32_t(minN > v.si);
	v.si ^= (infN ^ v.si) & -(int32_t(infN > v.si) & int32_t(v.si > maxN));
	v.si ^= (nanN ^ v.si) & -(int32_t(nanN > v.si) & int32_t(v.si > infN));
	v.ui >>= shift;  // logical shift
	v.si ^= ((v.si - maxD) ^ v.si) & -int32_t(v.si > maxC);
	v.si ^= ((v.si - minD) ^ v.si) & -int32_t(v.si > subC);
	return (v.ui | sign);
}

float HalfFloat::from_half(uint16_t value) const
{
	// NoFloatingException fpe_except;
	Bits v;
	v.ui = value;
	int32_t sign = v.si & signC;
	v.si ^= sign;
	sign <<= shiftSign;
	v.si ^= ((v.si + minD) ^ v.si) & -int32_t(v.si > subC);
	v.si ^= ((v.si + maxD) ^ v.si) & -int32_t(v.si > maxC);
	Bits s;
	s.si = mulC;
	s.f *= v.si;
	int32_t mask = -int32_t(norC > v.si);
	v.si <<= shift;
	v.si ^= (s.si ^ v.si) & mask;
	v.si |= sign;
	return v.f;
}
}  // namespace spu
