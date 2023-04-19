#include "VkBackend.h"
#include "CommandList.h"
#include "CommandQueue.h"
#include "VkDevice.h"
#include "GpuResource.h"
#include "IResourceDescriptor.h"
#include "SwapChain.h"
#include "VkMemoryHelper.h"
#include "vk_helper.h"
#include "Techniques.h"
//#include "ImguiHelper.h"
#include "vk_helper.h"
#include "DescriptorHeapCollection.h"
#include "Logger.h"
#include "ShaderManager.h"
#include "Fence.h"

VkBackend* gBackend = nullptr;

IBackend* CreateBackend() {
    assert(!gBackend);

    gBackend = new VkBackend;
    return (IBackend *)gBackend;
}

void DestroyBackend() {
    assert(gBackend);
    delete gBackend;
    gBackend = nullptr;
}

#define GetNativeCmdList(cmd_list) ((CommandList*)cmd_list)->GetNativeObject()

void VkBackend::CreateInstance(const std::vector<const char*> &extensions) {
    VkApplicationInfo appinfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appinfo.apiVersion = VK_API_VERSION_1_3; // TODO: check supported versions
    VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    createInfo.pApplicationInfo = &appinfo;

#ifdef _DEBUG
    const char* debugLayers[] =
    {
        "VK_LAYER_KHRONOS_validation",
    };

    createInfo.ppEnabledLayerNames = debugLayers;
    createInfo.enabledLayerCount = sizeof(debugLayers) / sizeof(debugLayers[0]);
#endif

    std::vector<const char*> extensions_vk = extensions;
    extensions_vk.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    extensions_vk.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    createInfo.ppEnabledExtensionNames = extensions_vk.data();
    createInfo.enabledExtensionCount = extensions_vk.size();

    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_instance));
}

void VkBackend::OnInit(const WindowHandler& window_hndl, uint32_t width, uint32_t height, const std::filesystem::path& path) {

    VK_CHECK(volkInitialize());

    // Create Instance
    CreateInstance(window_hndl.extensions);
    volkLoadInstance(m_instance);

    // Create Device
    m_device.reset(new VkDeviceW);
    m_device->Init(m_instance);
    volkLoadDevice(m_device->GetNativeObject());

    // queues
    m_commandQueueGfx.reset(new CommandQueue);
    m_commandQueueCompute.reset(new CommandQueue);
    m_commandQueueGfx->OnInit(ICommandQueue::QueueType::qt_gfx, GfxQueueCmdList_num, L"Gfx");
    m_commandQueueCompute->OnInit(ICommandQueue::QueueType::qt_compute, ComputeQueueCmdList_num, L"Compute");

    m_descriptor_heap_collection.reset(new DescriptorHeapCollection);
    m_descriptor_heap_collection->Initialize();

    m_memory_helper.reset(new VkMemoryHelper);
    m_memory_helper->Init();

    m_root_dir = path;
    m_shader_mgr.reset(new ShaderManager);
    m_techniques.reset(new Techniques);
    m_techniques->OnInit();

    // SwapChain
    m_swap_chain.reset(new SwapChain);
    m_swap_chain->OnInit(window_hndl, width, height, FramesCount);

    // Misc
    m_viewport = ViewPort(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    m_scissorRect = RectScissors(0, 0, static_cast<long>(width), static_cast<long>(height));

    m_logger.reset(new logger("app.log", logger::log_level::ll_INFO));

    m_frameIndex = m_swap_chain->GetCurrentBackBufferIndex();


    //m_gui.reset(new ImguiHelper);
    //m_gui->Initialize(FramesCount);

    m_interqueue_fence.reset(new Fence);
    m_interqueue_fence->Initialize(0);

    for (uint32_t i = 0; i < 2; i++) {
        m_cpu_fences[i].reset(new Fence);
        m_cpu_fences[i]->Initialize(0);
    }
}

uint32_t VkBackend::GetCurrentBackBufferIndex() const  {
    return m_swap_chain->GetCurrentBackBufferIndex();
}

IGpuResource& VkBackend::GetCurrentBackBuffer() {
    return m_swap_chain->GetCurrentBackBuffer();
}

IGpuResource* VkBackend::GetDepthBuffer() {
    return m_swap_chain->GetDepthBuffer();
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
    return (type == ICommandQueue::QueueType::qt_gfx ? m_commandQueueGfx : m_commandQueueCompute);
}

logger* VkBackend::GetLogger() {
    return m_logger.get();
}

void VkBackend::SyncWithCPU() {
	m_commandQueueGfx->WaitOnCPU(m_cpu_fences[m_frameIndex].get());
}

void VkBackend::SyncWithGpu(ICommandQueue::QueueType from, ICommandQueue::QueueType to) {
    std::shared_ptr<ICommandQueue>& from_queue = (from == ICommandQueue::QueueType::qt_gfx ? m_commandQueueGfx : m_commandQueueCompute);
    std::shared_ptr<ICommandQueue>& to_queue = (to == ICommandQueue::QueueType::qt_gfx ? m_commandQueueGfx : m_commandQueueCompute);

    from_queue->Signal(m_interqueue_fence.get(), false);
    to_queue->WaitOnGPU(m_interqueue_fence.get());
}

ICommandList* VkBackend::InitCmdList() {
    ICommandList* command_list_gfx = m_commandQueueGfx->ResetActiveCL();
    command_list_gfx->RSSetViewports(1, &m_viewport);
    command_list_gfx->RSSetScissorRects(1, &m_scissorRect);

    return command_list_gfx;
}

void VkBackend::ChechUpdatedShader() {
    if (m_rebuild_shaders) {
        m_commandQueueGfx->Flush();
        m_techniques->RebuildShaders(L"Rebuild techniques");
        m_rebuild_shaders = false;
    }
}

void VkBackend::RenderUI() {
    //m_gui->Render(m_frameIndex);
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
    return m_techniques->GetTechniqueById(id);
}

const IRootSignature* VkBackend::GetRootSignById(uint32_t id) {
    return m_techniques->GetRootSignById(id);
}

uint32_t VkBackend::GetRenderMode() const  {
    return m_render_mode;
}

IImguiHelper* VkBackend::GetUI() {
    //return m_gui.get();
    return nullptr;
}

VkBackend::~VkBackend() {

}

bool VkBackend::PassImguiWndProc(const ImguiWindowData& data) {
//    if(m_gui)
//        return ((ImguiHelper*)(m_gui.get()))->PassImguiWndProc(data);

   return false;
}

bool VkBackend::ShouldClose() {
    return m_should_close;
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
		if (std::shared_ptr<IResourceDescriptor> rtv = depth->GetDSV().lock()){
			attachments[size - 1] = (VkImageView)rtv->GetCPUhandle().ptr;
        }
	}

    VkRenderPass rp = m_techniques->GetRenderPassById(((Techniques::TechniqueVk*)GetTechniqueById(tech_id))->rp_id).rpass; // TODO: implememnt later
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

VkSampler VkBackend::GetSamplerById(uint32_t id) const {
    return m_techniques->GetSamplerById(id);
}
