//
// BaseApp :
//
#include "base_app.h"
#include <cstdio>
#include <mutex>

namespace spu {
namespace {

struct ControlState {
	std::mutex mutex;
	StreamControl prev_control;
	StreamControl curr_control;
	StreamControl sender_control;
};

ControlState g_control_state;

}  // namespace

BaseApp::BaseApp(const char *name) : GsPage(name) {}

StreamControl BaseApp::getSenderControl() const
{
	std::lock_guard<std::mutex> lock(g_control_state.mutex);
	return g_control_state.sender_control;
}

void BaseApp::setReceiverControl(const StreamControl &control)
{
	std::lock_guard<std::mutex> lock(g_control_state.mutex);
	g_control_state.curr_control = control;
}

void BaseApp::senderCallback(void *arg)
{
	auto *packet = static_cast<PayloadWithSize *>(arg);
	if (packet->size != sizeof(StreamControl)) {
		return;
	}

	std::lock_guard<std::mutex> lock(g_control_state.mutex);
	if (g_control_state.sender_control.serial == packet->payload.serial) {
		return;
	}
	g_control_state.sender_control = packet->payload;
	printf("sender control dry-run: serial=%u enable=%u fps=%u flags=%u\n",
	       g_control_state.sender_control.serial, g_control_state.sender_control.enable,
	       g_control_state.sender_control.fps, g_control_state.sender_control.flags);
}

void BaseApp::receiverCallback(void *arg)
{
	auto *packet = static_cast<PayloadWithSize *>(arg);
	std::lock_guard<std::mutex> lock(g_control_state.mutex);

	if (g_control_state.prev_control.serial == g_control_state.curr_control.serial) {
		packet->size = 0;
		return;
	}

	packet->size = sizeof(StreamControl);
	packet->payload = g_control_state.curr_control;
	g_control_state.prev_control = g_control_state.curr_control;
	printf("receiver control upload: serial=%u enable=%u fps=%u flags=%u\n",
	       g_control_state.curr_control.serial, g_control_state.curr_control.enable,
	       g_control_state.curr_control.fps, g_control_state.curr_control.flags);
}

}  // namespace spu
