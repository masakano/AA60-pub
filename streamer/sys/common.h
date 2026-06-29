//
// common.h
//
#pragma once
#include <cstdint>
#include <filesystem>
#include <vector>

namespace spu {

constexpr const char *c_shm_path = "reciever.dat";
constexpr const uint32_t c_width = 1280;
constexpr const uint32_t c_height = 720;

inline const std::vector<std::filesystem::path> paths = {
        "data/2026_04_09_13_29_49.h264", "data/2026_04_09_13_32_43.h264", "data/2026_04_09_13_52_36.h264",
        "data/2026_04_09_13_56_11.h264", "data/2026_04_09_14_00_02.h264",
};

struct RGBA8 {
	uint8_t r, g, b, a;
};
struct StreamControl {
	uint32_t serial = 0;
	uint32_t enable = 1;
	uint32_t fps = 60;
	uint32_t flags = 0;
};

struct SharedBuffer {
	uint32_t count;
	uint32_t i1, i2, i3;
	StreamControl control;
	RGBA8 pixels[0];
};

struct PayloadWithSize {
	uint32_t size;
	StreamControl payload;
};

}  // namespace spu
