#include "ImguiHelper.h"

#include <vector>
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx12.h"

#include "Application.h"
#include "CommandQueue.h"
#include "RenderQuad.h"
#include "GpuResource.h"
#include "DescriptorHeapCollection.h"
#include "ResourceDescriptor.h"
#include "DXAppImplementation.h"
#include "Console.h"

extern DXAppImplementation* gD3DApp;

static void AddToConsoleLogCallback(const std::string &line) {
	gD3DApp->GetUiHelper()->AddToConsoleLog(line);
}


ImguiHelper::ImguiHelper() :
	m_commandQueueGfx(std::make_unique<CommandQueue>()),
	m_rt(std::make_unique<RenderQuad>()),
	m_console(std::make_unique< AppConsole>())
{

}

void ImguiHelper::Initialize(ComPtr<ID3D12Device2>& device, uint32_t frames_num)
{
	m_device = device;
	m_frames_num = frames_num;
	auto hwnd = Application::GetHwnd();
	m_rt->Initialize();
	std::vector<ResourceFormat> formats{ ResourceFormat::rf_r8g8b8a8_unorm };
	m_rt->CreateQuadTexture(gD3DApp->GetWidth(), gD3DApp->GetHeight(), formats, m_frames_num, 0, L"imgui_quad");
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvuacbvHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE srvuacbvHandle_gpu;

	if (std::shared_ptr<DescriptorHeapCollection> heap_collection = gD3DApp->GetDescriptorHeapCollection().lock()) {
		heap_collection->ReserveSRVUAVCBVhandleOnGpu(srvuacbvHandle, srvuacbvHandle_gpu);
		m_gpu_visible_heap = heap_collection->GetGpuVisibleHeap();
	}

	m_commandQueueGfx->OnInit(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_frames_num, L"GUI");

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
		(DXGI_FORMAT)formats.front(), m_gpu_visible_heap.Get(),
		srvuacbvHandle,
		srvuacbvHandle_gpu);

	// connect log to console
	FuncPtrVoidStr cb = AddToConsoleLogCallback;
	gD3DApp->GetLogger()->SetConsoleCb(cb);
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

	CommandList& command_list = m_commandQueueGfx->ResetActiveCL();

	// set gpu heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_gpu_visible_heap.Get() };
	command_list.SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	UINT backBufferIdx = frame_id;
	if (std::shared_ptr<GpuResource> rt = m_rt->GetRt(frame_id).lock()) {
		m_commandQueueGfx->ResourceBarrier(rt, ResourceState::rs_resource_state_render_target);
		if (std::shared_ptr<ResourceDescriptor> render_target_view = rt->GetRTV().lock()) {
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = render_target_view->GetCPUhandle();
			command_list.OMSetRenderTargets(1, &rtvHandle, FALSE, NULL);
			const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
			command_list.ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		}
	}

	// Render Dear ImGui graphics
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), command_list.GetRawCommandList().Get());

	if (std::shared_ptr<GpuResource> rt = m_rt->GetRt(frame_id).lock()) {
		m_commandQueueGfx->ResourceBarrier(rt, ResourceState::rs_resource_state_pixel_shader_resource);
	}

	m_commandQueueGfx->ExecuteActiveCL();
	m_commandQueueGfx->Flush();

	ImGui::EndFrame();
}

bool ImguiHelper::WantCapture(CaptureInput_type type) const
{
	if (m_rt->IsInitialized()) {
		ImGuiIO& io = ImGui::GetIO();

		switch (type) {
		case CaptureInput_type::cit_keyboard:
			if (io.WantCaptureKeyboard)
				return true;
			break;
		}
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

ImguiHelper::~ImguiHelper() = default;
