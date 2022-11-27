#include "ImguiHelper.h"

#include <vector>
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx12.h"

#include "Application.h"
#include "GfxCommandQueue.h"
#include "RenderQuad.h"
#include "GpuResource.h"
#include "DescriptorHeapCollection.h"
#include "ResourceDescriptor.h"
#include "DXAppImplementation.h"

extern DXAppImplementation* gD3DApp;


ImguiHelper::ImguiHelper() :
	m_commandQueueGfx(std::make_unique<GfxCommandQueue>()),
	m_rt(std::make_unique<RenderQuad>())
{

}

void ImguiHelper::Initialize(ComPtr<ID3D12Device2>& device, uint32_t frames_num)
{
	m_device = device;
	m_frames_num = frames_num;
	auto hwnd = Application::GetHwnd();
	m_rt->Initialize();
	std::vector<DXGI_FORMAT> formats{ DXGI_FORMAT_R8G8B8A8_UNORM };
	m_rt->CreateQuadTexture(gD3DApp->GetWidth(), gD3DApp->GetHeight(), formats, m_frames_num, L"imgui_quad");
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvuacbvHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE srvuacbvHandle_gpu;

	if (std::shared_ptr<DescriptorHeapCollection> heap_collection = gD3DApp->GetDescriptorHeapCollection().lock()) {
		heap_collection->ReserveSRVUAVCBVhandleOnGpu(srvuacbvHandle, srvuacbvHandle_gpu);
		m_gpu_visible_heap = heap_collection->GetGpuVisibleHeap();
	}

	m_commandQueueGfx->OnInit(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT, L"GUI");

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(m_device.Get(), m_frames_num,
		formats.front(), m_gpu_visible_heap.Get(),
		srvuacbvHandle,
		srvuacbvHandle_gpu);


	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);
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

	//// draw UI
	//	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	//if (show_demo_window)
	//	ImGui::ShowDemoWindow(&show_demo_window);
	//float clear_color[3];
	//// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
	//{
	//	static float f = 0.0f;
	//	static int counter = 0;

	//	ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

	//	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
	//	ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
	//	ImGui::Checkbox("Another Window", &show_another_window);

	//	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
	//	ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

	//	if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
	//		counter++;
	//	ImGui::SameLine();
	//	ImGui::Text("counter = %d", counter);

	//	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	//	ImGui::End();
	//}

	//// 3. Show another simple window.
	//if (show_another_window)
	//{
	//	ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	//	ImGui::Text("Hello from another window!");
	//	if (ImGui::Button("Close Me"))
	//		show_another_window = false;
	//	ImGui::End();
	//}

	 // Create a window called "My First Tool", with a menu bar.
	ImGui::Begin("My First Tool", &my_tool_active, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */ }
			if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Do stuff */ }
			if (ImGui::MenuItem("Close", "Ctrl+W")) { my_tool_active = false; }
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	// Edit a color stored as 4 floats
	ImGui::ColorEdit4("Color", my_color);

	// Generate samples and plot them
	float samples[100];
	for (int n = 0; n < 100; n++)
		samples[n] = sinf(n * 0.2f + ImGui::GetTime() * 1.5f);
	ImGui::PlotLines("Samples", samples, 100);

	// Display contents in a scrolling region
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Important Stuff");
	ImGui::BeginChild("Scrolling");
	for (int n = 0; n < 50; n++)
		ImGui::Text("%04d: Some text", n);
	ImGui::EndChild();
	ImGui::End();

	ComPtr<ID3D12GraphicsCommandList6>& command_list = m_commandQueueGfx->ResetActiveCL();

	// Set default settings
	//command_list->RSSetViewports(1, &m_viewport);
	//command_list->RSSetScissorRects(1, &m_scissorRect);

	// set gpu heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_gpu_visible_heap.Get() };
	command_list->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	UINT backBufferIdx = frame_id;
	if (std::shared_ptr<GpuResource> rt = m_rt->GetRt(frame_id).lock()) {
		m_commandQueueGfx->ResourceBarrier(rt, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		if (std::shared_ptr<ResourceDescriptor> render_target_view = rt->GetRTV().lock()) {
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = render_target_view->GetCPUhandle();
			command_list->OMSetRenderTargets(1, &rtvHandle, FALSE, NULL);
			const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
			command_list->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		}
	}

	// Render Dear ImGui graphics
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), command_list.Get());

	if (std::shared_ptr<GpuResource> rt = m_rt->GetRt(frame_id).lock()) {
		m_commandQueueGfx->ResourceBarrier(rt, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	m_commandQueueGfx->ExecuteActiveCL();
	m_commandQueueGfx->Flush();

	ImGui::EndFrame();
}

