//
//$<<Header>>$
//

#pragma once

#include <blender/pointer.hpp>
#include <string>

namespace spu::oglplus::imports {

struct BlendFileVisitor {
	virtual void visitStr(const std::string &) = 0;

	virtual void visitPtr(BlendFilePointer) = 0;
	virtual void visitPPtr(BlendFilePointerToPointer) = 0;

	virtual void visitDbl(double) = 0;
	virtual void visitFlt(float v) { visitDbl(v); }

	virtual void visitU64(uint64_t) = 0;
	virtual void visitU32(uint32_t v) { visitU64(v); }
	virtual void visitU16(uint16_t v) { visitU64(v); }
	virtual void visitU8(uint8_t v) { visitU64(v); }

	virtual void visitI64(int64_t) = 0;
	virtual void visitI32(int32_t v) { visitI64(v); }
	virtual void visitI16(int16_t v) { visitI64(v); }
	virtual void visitI8(int8_t v) { visitI64(v); }

	virtual void visitChr(char) = 0;

	virtual void visitRaw(const char *, std::size_t) = 0;
};

}  // namespace spu::oglplus::imports
