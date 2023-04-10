#include "VkBackend.h"
#include "CommandList.h"
#include "VkDevice.h"

#define GetNativeCmdList(cmd_list) ((CommandList*)cmd_list)->GetNativeObject()

void VkBackend::OnInit(const WindowHandler& window_hndl, uint32_t width, uint32_t height, const std::filesystem::path& path) {

	VkResult volkInitialize();

	void volkLoadInstance(VkInstance instance);
	void volkLoadDevice(VkDevice device);
}

uint32_t VkBackend::GetCurrentBackBufferIndex() const  {

}

IGpuResource& VkBackend::GetCurrentBackBuffer() {

}

IGpuResource* VkBackend::GetDepthBuffer() {

}

void VkBackend::Present() {
	m_swap_chain->Present();

	// Signal for this frame
	m_commandQueueGfx->Signal(m_cpu_fences[m_frameIndex], true);

	// get next frame
	m_frameIndex = m_swap_chain->GetCurrentBackBufferIndex();
}

void VkBackend::OnResizeWindow() {

}

std::shared_ptr<ICommandQueue>& VkBackend::GetQueue(ICommandQueue::QueueType type) {

}

logger* VkBackend::GetLogger() {

}

void VkBackend::SyncWithCPU() {
	m_commandQueueGfx->WaitOnCPU(m_cpu_fences[m_frameIndex]);
}

void VkBackend::SyncWithGpu(ICommandQueue::QueueType from, ICommandQueue::QueueType to) {

}

ICommandList* VkBackend::InitCmdList() {

}

void VkBackend::ChechUpdatedShader() {

}

void VkBackend::RenderUI() {

}

void VkBackend::DebugSectionBegin(ICommandList* cmd_list, const std::string & name) {
	VkDebugUtilsLabelEXT markerInfo = {};
	markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	markerInfo.pLabelName = name.c_str();
	vkCmdBeginDebugUtilsLabelEXT(GetNativeCmdList(cmd_list), &markerInfo);
}

void VkBackend::DebugSectionEnd(ICommandList* cmd_list) {
	vkCmdEndDebugUtilsLabelEXT(GetNativeCmdList(cmd_list));
}

const ITechniques::Technique* VkBackend::GetTechniqueById(uint32_t id) const  {

}

const IRootSignature* VkBackend::GetRootSignById(uint32_t id) {

}

uint32_t VkBackend::GetRenderMode() const  {

}

uint32_t VkBackend::GetFrameCount() const  {

}

IImguiHelper* VkBackend::GetUI() {

}

bool VkBackend::PassImguiWndProc(const ImguiWindowData& data) {

}

bool VkBackend::ShouldClose() {

}

