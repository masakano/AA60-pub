//
// SpuSite :
//
#include <spu++/spu_site.h>

using namespace spu;

int32_t main(int32_t, const char **argv)
{
	Attrs attrs(argv + 1);
	attrs.trace("main", false);

	Attrs startup_attrs = {
	        {"startup.conf_path", "AA60/environ.conf"},
	        {"startup.conf_tag",  "spu"              },
	};
	SpuSite::main(startup_attrs + attrs);
	return 0;
}
