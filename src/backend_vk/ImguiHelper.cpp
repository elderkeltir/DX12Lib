#include "ImguiHelper.h"
#include <directx/d3dx12.h>
#include <vector>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "SwapChain.h"
#include "CommandQueue.h"
#include "IGpuResource.h"
#include "ResourceDescriptor.h"
#include "DxBackend.h"
#include "Console.h"
#include "DynamicGpuHeap.h"
#include "Logger.h"
#include "CommandList.h"
#include "DxDevice.h"

extern DxBackend* gBackend;

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void AddToConsoleLogCallback(const std::string& line) {
	gBackend->GetUiHelper()->AddToConsoleLog(line);
}

void ImguiHelper::CreateQuadTexture(uint32_t width, uint32_t height, ResourceFormat formats, uint32_t texture_nums) {
	for (uint32_t n = 0; n < texture_nums; n++)
	{
		m_rts[n].reset(CreateGpuResource());
		IGpuResource* res = m_rts[n].get();

		uint32_t res_flags = ResourceDesc::ResourceFlags::rf_allow_render_target;

		ResourceDesc res_desc = ResourceDesc::tex_2d(formats, width, height, 1, 0, 1, 0, (ResourceDesc::ResourceFlags)res_flags);
		res->CreateTexture(HeapType::ht_default, res_desc, ResourceState::rs_resource_state_pixel_shader_resource, nullptr, L"UIquad");
		res->CreateRTV();

		SRVdesc srv_desc = {};
		srv_desc.format = formats;
		srv_desc.dimension = SRVdesc::SRVdimensionType::srv_dt_texture2d;
		srv_desc.texture2d.most_detailed_mip = 0;
		srv_desc.texture2d.mip_levels = 1;
		srv_desc.texture2d.res_min_lod_clamp = 0.0f;
		res->Create_SRV(srv_desc);
	}
}

ImguiHelper::ImguiHelper() :
	m_console(std::make_unique< AppConsole>())
{
	m_commandQueueGfx.reset(new CommandQueue);
}

void ImguiHelper::Initialize(uint32_t frames_num)
{
	ComPtr<ID3D12Device2>& device = gBackend->GetDevice()->GetNativeObject();
	HWND hwnd = *(HWND*)(gBackend->GetSwapChain()->GetWindowHndl().ptr);
	m_device = device;
	m_frames_num = frames_num;

	ResourceFormat format = ResourceFormat::rf_r8g8b8a8_unorm;
	CreateQuadTexture(gBackend->GetSwapChain()->GetWidth(), gBackend->GetSwapChain()->GetHeight(), format, m_frames_num);
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvuacbvHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE srvuacbvHandle_gpu;

	m_gpu_visible_heap.reset(new DynamicGpuHeap);
	m_gpu_visible_heap->Initialize(99);

	CPUdescriptor cpu_decs;
	GPUdescriptor gpu_desc;
	m_gpu_visible_heap->ReserveDescriptor(cpu_decs, gpu_desc);
	srvuacbvHandle.ptr = cpu_decs.ptr;
	srvuacbvHandle_gpu.ptr = gpu_desc.ptr;

	m_commandQueueGfx->OnInit(ICommandQueue::QueueType::qt_gfx, m_frames_num, L"GUI");

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(m_device.Get(), m_frames_num,
		(DXGI_FORMAT)format, ((DynamicGpuHeap*)m_gpu_visible_heap.get())->GetVisibleHeap().Get(),
		srvuacbvHandle,
		srvuacbvHandle_gpu);

	// connect log to console
	FuncPtrVoidStr cb = AddToConsoleLogCallback;
	gBackend->GetLogger()->SetConsoleCb(cb);

	m_is_initialized = true;
}


void ImguiHelper::Destroy()
{
	m_commandQueueGfx->Flush();

	// Cleanup
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImguiHelper::Render(uint32_t frame_id)
{
	// Start the Dear ImGui frame
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//

	if (m_console && m_console->Active()) {
		m_console->Draw("Console");
	}

	ICommandList* command_list = m_commandQueueGfx->ResetActiveCL();

	// set gpu heap
	command_list->SetDescriptorHeap(m_gpu_visible_heap.get());

	UINT backBufferIdx = frame_id;
	if (IGpuResource* rt = m_rts[frame_id].get()) {
		std::vector<IGpuResource*> rts{ rt };
		command_list->ResourceBarrier(*rt, ResourceState::rs_resource_state_render_target);
		command_list->SetRenderTargets(rts,nullptr);
		const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		command_list->ClearRenderTargetView(rt, clearColor, 0, nullptr);
	}

	// Render Dear ImGui graphics
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), ((CommandList*)command_list)->GetRawCommandList().Get());

	if (IGpuResource* rt = m_rts[frame_id].get()) {
		command_list->ResourceBarrier(*rt, ResourceState::rs_resource_state_pixel_shader_resource);
	}

	m_commandQueueGfx->ExecuteActiveCL();
	m_commandQueueGfx->Flush();

	ImGui::EndFrame();
}

bool ImguiHelper::WantCapture(CaptureInput_type type) const
{
	if (!m_is_initialized)
		return false;

	ImGuiIO& io = ImGui::GetIO();

	switch (type) {
	case CaptureInput_type::cit_keyboard:
		if (io.WantCaptureKeyboard)
			return true;
		break;
	}

	return false;
}

void ImguiHelper::ShowConsole()
{
	bool& active = m_console->Active();
	active = !active;
}

void ImguiHelper::AddToConsoleLog(const std::string& line)
{
	m_console->AddToLog(line);
}

bool ImguiHelper::PassImguiWndProc(const ImguiWindowData& data)
{
	HWND hWnd = *(HWND*)(data.hWnd.ptr);
	uint32_t message = data.msg;
	WPARAM wParam = (WPARAM)(data.wParam);
	LPARAM lParam = (LPARAM)(data.lParam);

	return ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);
}

ImguiHelper::~ImguiHelper() = default;
