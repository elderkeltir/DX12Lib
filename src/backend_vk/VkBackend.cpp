#include "VkBackend.h"
#include "CommandList.h"
#include "VkDevice.h"
#include "GpuResource.h"
#include "IResourceDescriptor.h"
#include "SwapChain.h"
#include "vk_helper.h"

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

void VkBackend::CreateFrameBuffer(std::vector<IGpuResource*> &rts, IGpuResource * depth, uint32_t tech_id) {
	if (rts.empty() && !depth)
		return;

	uint32_t size = rts.size() + (depth ? 1 : 0);
	std::vector<VkImageView> attachments(size);
	for (uint32_t i = 0; i < rts.size(); i++){
		GpuResource *  rt = (GpuResource*)rts[i];
		if (std::shared_ptr<IResourceDescriptor> rtv = rt->GetRTV().lock()){
			attachments[i] = (VkImageView)rtv->GetCPUhandle().ptr;
		}
	}

	if (depth) {
		if (std::shared_ptr<IResourceDescriptor> rtv = depth->GetRTV().lock()){
			attachments[size - 1] = (VkImageView)rtv->GetCPUhandle().ptr;
	}

	VkRenderPass rp = GetTechniqueById(tech_id)->render_pass; // TODO: implememnt later
	VkFramebufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	createInfo.renderPass = rp;
	createInfo.attachmentCount = attachments.size();
	createInfo.pAttachments = attachments.data();
	createInfo.width = m_swap_chain->GetWidth();
	createInfo.height = m_swap_chain->GetHeight();
	createInfo.layers = 1;

	VkFramebuffer framebuffer = 0;
	VkDevice device = m_device->GetNativeObject();
	VK_CHECK(vkCreateFramebuffer(device, &createInfo, 0, &framebuffer));

	GpuResource *  rt = (GpuResource*) ( !rts.empty() ? rts.front() : depth);
	rt->SetFrameBuffer(framebuffer);
	rt->SetRenderPass(rp);
}