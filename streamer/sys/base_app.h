//
// BaseApp :
//
#pragma once
#include "common.h"
#include <gsys/canvas/gs_page.h>

namespace spu {

class BaseApp : public gs_canvas::GsPage {
public:
	explicit BaseApp(const char *name);

	static void senderCallback(void *arg);
	static void receiverCallback(void *arg);

protected:
	StreamControl getSenderControl() const;
	void setReceiverControl(const StreamControl &control);
};

}  // namespace spu
