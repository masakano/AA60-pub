//
//
//
#pragma once
#include <cstdint>
#include <cstddef>

namespace spu::murmur3 {

inline constexpr uint32_t seed = 0;

inline constexpr uint32_t to_uint32(char const* key, size_t i = sizeof(uint32_t), uint32_t u32 = 0)
{
	return i ? to_uint32(key, i - 1, (u32 << 8) | key[i - 1]) : u32;
}

inline constexpr uint32_t murmur3a_5(uint32_t h) { return (h * 5) + 0xe6546b64; }
inline constexpr uint32_t murmur3a_4(uint32_t h) { return murmur3a_5((h << 13) | (h >> 19)); }
inline constexpr uint32_t murmur3a_3(uint32_t k, uint32_t h) { return murmur3a_4(h ^ k); }
inline constexpr uint32_t murmur3a_2(uint32_t k, uint32_t h) { return murmur3a_3(k * 0x1b873593, h); }
inline constexpr uint32_t murmur3a_1(uint32_t k, uint32_t h) { return murmur3a_2((k << 15) | (k >> 17), h); }
inline constexpr uint32_t murmur3a_0(uint32_t k, uint32_t h) { return murmur3a_1(k * 0xcc9e2d51, h); }
inline constexpr uint32_t murmur3a(char const* key, size_t i, uint32_t h = seed)
{
	return i ? murmur3a(key + sizeof(uint32_t), i - 1, murmur3a_0(to_uint32(key), h)) : h;
}
inline constexpr uint32_t murmur3b_3(uint32_t k, uint32_t h) { return h ^ k; }
inline constexpr uint32_t murmur3b_2(uint32_t k, uint32_t h) { return murmur3b_3(k * 0x1b873593, h); }
inline constexpr uint32_t murmur3b_1(uint32_t k, uint32_t h) { return murmur3b_2((k << 15) | (k >> 17), h); }
inline constexpr uint32_t murmur3b_0(uint32_t k, uint32_t h) { return murmur3b_1(k * 0xcc9e2d51, h); }
inline constexpr uint32_t murmur3b(char const* key, size_t i, uint32_t h)
{
	return i ? murmur3b_0(to_uint32(key, i), h) : h;
}
inline constexpr uint32_t murmur3c_4(uint32_t h) { return h ^ (h >> 16); }
inline constexpr uint32_t murmur3c_3(uint32_t h) { return murmur3c_4(h * 0xc2b2ae35); }
inline constexpr uint32_t murmur3c_2(uint32_t h) { return murmur3c_3(h ^ (h >> 13)); }
inline constexpr uint32_t murmur3c_1(uint32_t h) { return murmur3c_2(h * 0x85ebca6b); }
inline constexpr uint32_t murmur3c_0(uint32_t h) { return murmur3c_1(h ^ (h >> 16)); }
inline constexpr uint32_t murmur3c(uint32_t h, size_t len) { return murmur3c_0(h ^ len); }
inline constexpr uint32_t murmur3d(char const* str, size_t len)
{
	return murmur3c(murmur3b(str + ((len >> 2) * sizeof(uint32_t)), len & 3, murmur3a(str, len >> 2)), len);
}
inline constexpr uint32_t operator"" _murmur3(char const* str, size_t len) { return murmur3d(str, len); }
inline constexpr uint32_t strlen(char const* str) { return *str == '\0' ? 0 : 1 + strlen(str + 1); }

}  // namespace spu::murmur3
