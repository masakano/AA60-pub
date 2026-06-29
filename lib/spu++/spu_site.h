//
// SpuSite :
//
#pragma once
#include "spu_backoffice.h"
#include "spu_page.h"

namespace spu {

class SpuSite : public SpuBackofficeManager<SpuFrame, SpuPage> {
public:
	SpuSite(const Attrs &attrs) { init(attrs); }
	static void main(const Attrs &attrs);
};
}  // namespace spu
