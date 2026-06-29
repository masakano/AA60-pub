//
// SpuPage :
//
#include "base_app.h"
#include <spu++/imgui/imgui_impl_spu.h>

namespace spu {

void BaseApp::end()
{
	if (m_isMenu) {
		ImGui_ImplSpu_NewFrame();
		ImGui::Begin("Main");
		ImGui::Checkbox("paused", &m_isPaused);
		menu();
		ImGui::End();
		ImGui::Render();
		ImGui_ImplSpu_RenderDrawData(ImGui::GetDrawData());
	}
	getSeconds().setFixedDelta(m_isPaused ? 0 : -1);  // -1: real time
	SpuPage::end();
}

}  // namespace spu
