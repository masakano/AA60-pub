//
// GsSite :
//
#include "base_app.h"
#include <gsys/canvas/gs_site.h>

using namespace spu;

int32_t main(int32_t /*argc*/, const char **argv)
{
	Attrs attrs(argv + 1);
	attrs.trace("main", false);

	auto window = string_printf("0,0,%d,%d", int32_t(c_width), int32_t(c_height));
	Attrs startup_attrs = {
	        {"startup.conf_path", "AA60/environ.conf"      },
	        {"startup.conf_tag",  "spu:gsys"               },
	        {"c",                 "sender"                 },
	        {"window",            window                   },
	        {"sender.callback",   BaseApp::senderCallback  },
	        {"receiver.callback", BaseApp::receiverCallback},
	};
	gs_canvas::GsSite::main(startup_attrs + attrs);
	return 0;
}
