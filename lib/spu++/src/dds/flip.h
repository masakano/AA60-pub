//
//
//
#pragma once

#include "block.h"
#include "const.h"
#include <spu++/dds/dds.h>

namespace spu::dds {

namespace {
// flip a DXT1 color block
inline void flip_blocks_dxtc1(void *ptr, uint32_t numBlocks)
{
	auto *curblock = static_cast<BlockDXT1 *>(ptr);
	for (auto i = 0u; i < numBlocks; i++) {
		curblock->flip();
		curblock++;
	}
}

inline void flip_blocks_dxtc3(void *ptr, uint32_t numBlocks)
{
	auto *curblock = static_cast<BlockDXT1 *>(ptr);
	AlphaBlockDXT3 *alphablock;

	for (auto i = 0u; i < numBlocks; i++) {
		alphablock = reinterpret_cast<AlphaBlockDXT3 *>(curblock);
		alphablock->flip();
		curblock++;

		curblock->flip();
		curblock++;
	}
}

inline void flip_blocks_dxtc5(void *ptr, uint32_t numBlocks)
{
	auto *curblock = static_cast<BlockDXT1 *>(ptr);
	AlphaBlockDXT5 *alphablock;
	// uint8_t temp;

	for (auto i = 0u; i < numBlocks; i++) {
		alphablock = reinterpret_cast<AlphaBlockDXT5 *>(curblock);
		alphablock->flip();
		curblock++;

		curblock->flip();
		curblock++;
	}
}
#if 1  // when sizeof(AlphaBlockDXT5) is not 8

inline void flip_blocks_bc4(void *ptr, uint32_t numBlocks)
{
	auto *iptr = (uint8_t *)ptr;

	for (auto i = 0u; i < numBlocks; i++) {
		auto *alphablock = reinterpret_cast<AlphaBlockDXT5 *>(iptr);
		alphablock->flip();
		iptr += 8;
	}
}

inline void flip_blocks_bc5(void *ptr, uint32_t numBlocks)
{
	auto *iptr = (uint8_t *)ptr;

	for (auto i = 0u; i < numBlocks; i++) {
		auto *alphablock0 = reinterpret_cast<AlphaBlockDXT5 *>(iptr);
		alphablock0->flip();
		iptr += 8;

		auto *alphablock1 = reinterpret_cast<AlphaBlockDXT5 *>(iptr);
		alphablock1->flip();
		iptr += 8;
	}
}
#else
static_assert(sizeof(AlphaBlockDXT5) == 8);
inline void flip_blocks_bc4(void *ptr, uint32_t numBlocks)
{
	auto *alphablock = static_cast<AlphaBlockDXT5 *>(ptr);

	for (auto i = 0u; i < numBlocks; i++) {
		alphablock->flip();
		alphablock++;
	}
}

inline void flip_blocks_bc5(void *ptr, uint32_t numBlocks)
{
	auto *alphablock = static_cast<AlphaBlockDXT5 *>(ptr);

	for (auto i = 0u; i < numBlocks; i++) {
		alphablock->flip();
		alphablock++;

		alphablock->flip();
		alphablock++;
	}
}
#endif
}  // namespace

inline void Image::flip(void *ptr, int32_t width, int32_t height, int32_t depth) const
{
	assert(depth == 1);
	auto *surf = static_cast<uint8_t *>(ptr);

	{
		void (*flipblocks)(void *, uint32_t) = nullptr;

		uint32_t blockSize = 0;

		switch (m_pformat) {
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
			blockSize = 8;
			flipblocks = &flip_blocks_dxtc1;
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
			blockSize = 16;
			flipblocks = &flip_blocks_dxtc3;
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
			blockSize = 16;
			flipblocks = &flip_blocks_dxtc5;
			break;
		case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
		case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
		case GL_COMPRESSED_RED_RGTC1_EXT:
		case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:
			blockSize = 8;
			flipblocks = &flip_blocks_bc4;
			break;
		case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
		case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
		case GL_COMPRESSED_RG_RGTC2:
		case GL_COMPRESSED_SIGNED_RG_RGTC2:
			blockSize = 16;
			flipblocks = &flip_blocks_bc5;
			break;

		case GL_RGBA:
		case GL_RGB:
		case GL_BGRA:
		case GL_BGR:
		case GL_LUMINANCE: break;
		default: aux_error(true, "dds: cannot flip. format=[%s]\n", opengl_const(m_pformat)); return;
		}

		if (blockSize == 0) {  // uncompressed
			uint32_t lineSize = m_elementSize * width;
			uint32_t sliceSize = lineSize * height;
			std::vector<uint8_t> tempBuf(lineSize);

			for (auto ii = 0; ii < depth; ii++) {
				uint8_t *top = surf + ii * sliceSize;
				uint8_t *bottom = top + (sliceSize - lineSize);

				for (auto jj = 0; jj < (height >> 1); jj++) {
					memcpy(tempBuf.data(), top, lineSize);
					memcpy(top, bottom, lineSize);
					memcpy(bottom, tempBuf.data(), lineSize);

					top += lineSize;
					bottom -= lineSize;
				}
			}
		}
		else {
			width = (width + 3) / 4;
			height = (height + 3) / 4;

			auto lineSize = width * blockSize;
			auto *top = surf;
			auto *bottom = surf + (height - 1) * lineSize;
			std::vector<uint8_t> tempBuf(lineSize);

			for (auto j = 0; j < std::max(height >> 1, 1); j++) {
				if (top == bottom) {
					assert(flipblocks);
					flipblocks(top, width);
					break;
				}

				flipblocks(top, width);
				flipblocks(bottom, width);

				memcpy(tempBuf.data(), top, lineSize);
				memcpy(top, bottom, lineSize);
				memcpy(bottom, tempBuf.data(), lineSize);

				top += lineSize;
				bottom -= lineSize;
			}
		}
	}
}
}  // namespace spu::dds
