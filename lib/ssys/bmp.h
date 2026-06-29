//
//
//
#pragma once
#include "ssys.h"

namespace spu {
uint8_t *bmp_read(const char *name, int32_t *width, int32_t *height, int32_t *bpp);
void bmp_write(const char *name, uint8_t *buf, int32_t width, int32_t height, int32_t isrev);  // RGB only
}  // namespace spu
