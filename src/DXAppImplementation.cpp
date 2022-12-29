#include "DXAppImplementation.h"
#include "Application.h"
#include "DXHelper.h"
#include "Level.h"
#include "DescriptorHeapCollection.h"
#include "GpuResource.h"
#include "HeapBuffer.h"
#include "ResourceDescriptor.h"
#include "ShaderManager.h"
#include "FreeCamera.h"
#include "LevelEntity.h"
#include "GfxCommandQueue.h"
#include "RenderQuad.h"
#include "MaterialManager.h"
#include "DynamicGpuHeap.h" // TODO: remove when encapsulate dynamic heap under queue
#include "SSAO.h"
#include "ImguiHelper.h"
#include "Logger.h"

#include "WinPixEventRuntime/pix3.h"

#if defined(USE_PIX) && defined(USE_PIX_DEBUG)
#include "PIXapi.h"
#endif // defined(USE_PIX) && defined(USE_PIX_DEBUG)

DXAppImplementation* gD3DApp = nullptr;

DXAppImplementation::DXAppImplementation(uint32_t width, uint32_t height, std::wstring name) :
	DXApp(width, height, name),
	m_frameIndex(0),
	m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
	m_descriptor_heap_collection(std::make_shared<DescriptorHeapCollection>()),
	m_renderTargets(std::make_unique<GpuResource[]>(FrameCount)),
	m_depthStencil(std::make_unique<GpuResource>()),
	m_commandQueueGfx(std::make_shared<GfxCommandQueue>()),
	m_commandQueueCompute(std::make_shared<GfxCommandQueue>()),
	m_post_process_quad(std::make_unique<RenderQuad>()),
	m_forward_quad(std::make_unique<RenderQuad>()),
	m_deferred_shading_quad(std::make_unique<RenderQuad>()),
	m_ssao(std::make_unique<SSAO>()),
	m_gui(std::make_unique<ImguiHelper>()),
	m_logger(std::make_unique<logger>("app.log", logger::log_level::ll_INFO))
{
}

DXAppImplementation::~DXAppImplementation() = default;

void DXAppImplementation::OnInit()
{
	m_start_time = std::chrono::system_clock::now();
	gD3DApp = this;
	ResourceManager::OnInit();
	CreateDevice(L"DXAppImplementation");
	m_commandQueueGfx->OnInit(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT, GfxQueueCmdList_num, L"Gfx");
	m_commandQueueCompute->OnInit(m_device, D3D12_COMMAND_LIST_TYPE_COMPUTE, ComputeQueueCmdList_num, L"Compute");
	CreateSwapChain(L"DXAppImplementation");
	Techniques::OnInit(m_device, L"PredefinedTechinue");
	ConstantBufferManager::OnInit();

	m_level = std::make_shared<Level>();
	m_level->Load(L"test_level.json");
	m_material_mgr->LoadMaterials();

	m_post_process_quad->Initialize();
	m_deferred_shading_quad->Initialize();
	m_forward_quad->Initialize();
	m_ssao->Initialize(m_width, m_height, L"SSAO_");

	m_gui->Initialize(m_device, FrameCount);
	ThrowIfFailed(m_device->CreateFence(m_fence_inter_queue_val, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&m_fence_inter_queue)));
	LOG_INFO("Initialized finieshed");
}

void DXAppImplementation::CreateDevice(std::optional<std::wstring> dbg_name) {
#if defined(USE_PIX) && defined(USE_PIX_DEBUG)
	{
		// Check to see if a copy of WinPixGpuCapturer.dll has already been injected into the application.
		// This may happen if the application is launched through the PIX UI. 
		if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0)
		{
			LoadLibrary(GetLatestWinPixGpuCapturerPath_Cpp17().c_str());
		}
	}
#endif // defined(USE_PIX) && defined(USE_PIX_DEBUG)
	uint32_t dxgiFactoryFlags = 0;

#if defined(_DEBUG) && !defined(USE_PIX_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)));
	//SetName(m_factory, dbg_name.value_or(L"").append(L"_factory").c_str());

	if (m_useWarpDevice)
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(m_factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_12_0,
			IID_PPV_ARGS(&m_device)
		));
	}
	else
	{
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(m_factory.Get(), &hardwareAdapter);

		ThrowIfFailed(D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_12_0,
			IID_PPV_ARGS(&m_device)
		));
		SetName(m_device, dbg_name.value_or(L"").append(L"_device").c_str());
	}

	// Enable debug messages in debug mode.
#if defined(_DEBUG) && !defined(USE_PIX_DEBUG)
	ComPtr<ID3D12InfoQueue> pInfoQueue;
	if (SUCCEEDED(m_device.As(&pInfoQueue)))
	{
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		// Suppress whole categories of messages
		//D3D12_MESSAGE_CATEGORY Categories[] = {};

		// Suppress messages based on their severity level
		D3D12_MESSAGE_SEVERITY Severities[] =
		{
			D3D12_MESSAGE_SEVERITY_INFO
		};

		// Suppress individual messages by their ID
		D3D12_MESSAGE_ID DenyIds[] = {
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
		};

		D3D12_INFO_QUEUE_FILTER NewFilter = {};
		//NewFilter.DenyList.NumCategories = _countof(Categories);
		//NewFilter.DenyList.pCategoryList = Categories;
		NewFilter.DenyList.NumSeverities = _countof(Severities);
		NewFilter.DenyList.pSeverityList = Severities;
		NewFilter.DenyList.NumIDs = _countof(DenyIds);
		NewFilter.DenyList.pIDList = DenyIds;

		ThrowIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
	}
#endif
}

void DXAppImplementation::CreateSwapChain(std::optional<std::wstring> dbg_name) {
	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(m_factory->CreateSwapChainForHwnd(
		m_commandQueueGfx->m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
		Application::GetHwnd(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	// This sample does not support fullscreen transitions.
	ThrowIfFailed(m_factory->MakeWindowAssociation(Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain.As(&m_swapChain));
	//SetName(m_swapChain, dbg_name.value_or(L"").append(L"_swap_chain").c_str());

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	m_descriptor_heap_collection->Initialize(L"DxAppImplemenation");

	// Create frame resources.
	{
		// Create a RTV for each frame.
		for (uint32_t n = 0; n < FrameCount; n++)
		{
			ComPtr<ID3D12Resource> renderTarget;
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&renderTarget)));
			SetName(renderTarget, std::wstring(L"render_target_").append(std::to_wstring(n)).c_str());
			m_renderTargets[n].SetBuffer(renderTarget);
			m_renderTargets[n].CreateRTV();
		}
	}

	// Create the depth stencil view.
	{
		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;
		CD3DX12_RESOURCE_DESC res_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		m_depthStencil->CreateTexture(HeapBuffer::BufferType::bt_default, res_desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthOptimizedClearValue, L"depth_stencil");

		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		m_depthStencil->Create_DSV(depthStencilDesc);

		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MostDetailedMip = 0;
		srv_desc.Texture2D.MipLevels = 1;
		srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
		m_depthStencil->Create_SRV(srv_desc);
	}
}

// Update frame-based values.
void DXAppImplementation::OnUpdate()
{
	// Time logic
	m_frame_id++;
	std::chrono::time_point t = std::chrono::system_clock::now();
	m_dt = t - m_time;
	m_time = t;
	m_total_time = t - m_start_time;

	if (std::shared_ptr<FreeCamera> camera = m_level->GetCamera().lock()) {
		UpdateCamera(camera, m_dt.count());
	}

	m_level->Update(m_dt.count());
}

// Render the scene.
void DXAppImplementation::OnRender()
{
	// Wait for signal from new frame
	m_commandQueueGfx->WaitOnCPU(m_fenceValues[m_frameIndex]);

	auto InitCmdList = [this]() {
		ComPtr<ID3D12GraphicsCommandList6>& command_list_gfx = m_commandQueueGfx->ResetActiveCL();
		command_list_gfx->RSSetViewports(1, &m_viewport);
		command_list_gfx->RSSetScissorRects(1, &m_scissorRect);

		return command_list_gfx;
	};

	if (m_rebuild_shaders) {
		m_commandQueueGfx->Flush();
		Techniques::RebuildShaders(L"Rebuilt techniques");
		m_rebuild_shaders = false;
	}

	// Gui
	m_gui->Render(m_frameIndex);

	// Record all the commands we need to render the scene into the command list.
	ComPtr<ID3D12GraphicsCommandList6>& command_list_gfx = InitCmdList();

	// Render scene
	BEGIN_EVENT(command_list_gfx, "G-Buffer");
	RenderLevel(command_list_gfx);
	m_commandQueueGfx->ResourceBarrier(*m_ssao->GetSSAOres(2), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	END_EVENT(command_list_gfx);
	
	// send G-buffer to execute
	m_commandQueueGfx->ExecuteActiveCL();
	m_commandQueueGfx->Signal(m_fence_inter_queue, ++m_fence_inter_queue_val);
	m_commandQueueCompute->WaitOnGPU(m_fence_inter_queue, m_fence_inter_queue_val);

	ComPtr<ID3D12GraphicsCommandList6>& command_list_compute = m_commandQueueCompute->ResetActiveCL();
	
	BEGIN_EVENT(command_list_compute, "SSAO");
	RenderSSAOquad(command_list_compute);
	BlurSSAO(command_list_compute);
	END_EVENT(command_list_compute);

	// send SSAO to execute
	m_commandQueueCompute->ExecuteActiveCL();
	m_commandQueueCompute->Signal(m_fence_inter_queue, ++m_fence_inter_queue_val);
	m_commandQueueGfx->WaitOnGPU(m_fence_inter_queue, m_fence_inter_queue_val);

	command_list_gfx = InitCmdList();

	// deferred shading
	BEGIN_EVENT(command_list_gfx, "Deferred Shading");
	RenderDeferredShadingQuad(command_list_gfx);
	END_EVENT(command_list_gfx);

	// forward pass
	BEGIN_EVENT(command_list_gfx, "Forward Pass");
	RenderForwardQuad(command_list_gfx);
	END_EVENT(command_list_gfx);

	// post process
	BEGIN_EVENT(command_list_gfx, "Post Processing");
	RenderPostProcessQuad(command_list_gfx);
	END_EVENT(command_list_gfx);

	// Indicate that the back buffer will now be used to present.
	m_commandQueueGfx->ResourceBarrier(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_PRESENT);
	m_commandQueueGfx->ExecuteActiveCL();

	// Present the frame.
	ThrowIfFailed(m_swapChain->Present(1, 0));

	// Signal for this frame
	m_fenceValues[m_frameIndex] = m_commandQueueGfx->Signal();

	// get next frame
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void DXAppImplementation::OnDestroy()
{
	m_gui->Destroy();
	m_commandQueueGfx->OnDestroy();
	gD3DApp = nullptr;
}

void DXAppImplementation::PrepareRenderTarget(ComPtr<ID3D12GraphicsCommandList6>& command_list, const std::vector<std::shared_ptr<GpuResource>>& rts, bool set_dsv, bool clear_dsv) {
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	if (std::shared_ptr<ResourceDescriptor> render_target_view = rts.at(0)->GetRTV().lock()) {
		rtvHandle = render_target_view->GetCPUhandle();
	}
	else {
		assert(false);
	}

	if (set_dsv) {
		if (std::shared_ptr<ResourceDescriptor> depth_view = m_depthStencil->GetDSV().lock()) {
			dsvHandle = depth_view->GetCPUhandle();
			if (clear_dsv) {
				command_list->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
			}
		}
		else {
			assert(false);
		}

		command_list->OMSetRenderTargets((uint32_t)rts.size(), &rtvHandle, rts.size() > 1 ? TRUE : FALSE, &dsvHandle);
	}
	else {
		command_list->OMSetRenderTargets((uint32_t)rts.size(), &rtvHandle, rts.size() > 1 ? TRUE : FALSE, NULL);
	}

	// Record commands.
	const float clearColor[] = { 0.f, 0.f, 0.f, 0.0f };
	for (auto& rt : rts) {
		if (std::shared_ptr<ResourceDescriptor> render_target_view = rt->GetRTV().lock()) {
			rtvHandle = render_target_view->GetCPUhandle();
			command_list->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		}
	}
}

void DXAppImplementation::PrepareRenderTarget(ComPtr<ID3D12GraphicsCommandList6>& command_list, GpuResource& rt, bool set_dsv, bool clear_dsv) {
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	if (std::shared_ptr<ResourceDescriptor> render_target_view = rt.GetRTV().lock()) {
		rtvHandle = render_target_view->GetCPUhandle();
	}
	else {
		assert(false);
	}
	if (std::shared_ptr<ResourceDescriptor> depth_view = m_depthStencil->GetDSV().lock()) {
		dsvHandle = depth_view->GetCPUhandle();
	}
	else {
		assert(false);
	}

	if (set_dsv) {
		command_list->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
		if (clear_dsv) {
			command_list->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		}
	}
	else {
		command_list->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	}

	// Record commands.
	const float clearColor[] = { 0.f, 0.f, 0.f, 0.0f };
	command_list->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
}

void DXAppImplementation::RenderLevel(ComPtr<ID3D12GraphicsCommandList6>& command_list) {
	std::vector<DXGI_FORMAT> formats = { DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT };
	m_deferred_shading_quad->CreateQuadTexture(m_width, m_height, formats, FrameCount, 0, L"m_deferred_shading_quad_");
	{
		std::vector<std::shared_ptr<GpuResource>>& rts = m_deferred_shading_quad->GetRts(m_frameIndex);
		m_commandQueueGfx->ResourceBarrier(rts, D3D12_RESOURCE_STATE_RENDER_TARGET);
		PrepareRenderTarget(command_list, rts);
	}

	m_level->Render(command_list);

	m_commandQueueGfx->ResourceBarrier(*m_depthStencil.get(), D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	{
		std::vector< std::shared_ptr<GpuResource>>& rts = m_deferred_shading_quad->GetRts(m_frameIndex);
		for (uint32_t i = 0; i < rts.size(); i++) {
			if (i == 1 || i == 2) {
				m_commandQueueGfx->ResourceBarrier(rts[i], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			}
			else {
				m_commandQueueGfx->ResourceBarrier(rts[i], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			}
		}
	}
}

void DXAppImplementation::RenderPostProcessQuad(ComPtr<ID3D12GraphicsCommandList6>& command_list) {
	if (std::shared_ptr<GpuResource> rt = m_post_process_quad->GetRt(m_frameIndex).lock()) {
		m_commandQueueGfx->ResourceBarrier(rt, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
	else {
		assert(false);
	}

	m_commandQueueGfx->ResourceBarrier(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET);
	PrepareRenderTarget(command_list, m_renderTargets[m_frameIndex], false);

	const Techniques::Technique* tech = GetTechniqueById(tt_post_processing);
	if (m_commandQueueGfx->GetPSO() != tt_post_processing) {
		m_commandQueueGfx->SetPSO(tt_post_processing);
	}
	if (m_commandQueueGfx->GetRootSign() != tech->root_signature) {
		m_commandQueueGfx->SetRootSign(tech->root_signature);
		m_commandQueueGfx->GetGpuHeap().CacheRootSignature(gD3DApp->GetRootSignById(tech->root_signature));
	}

	if (std::shared_ptr<GpuResource> rt = m_post_process_quad->GetRt(m_frameIndex).lock()) {
		if (std::shared_ptr<ResourceDescriptor> srv = rt->GetSRV().lock()) {
			m_commandQueueGfx->GetGpuHeap().StageDesctriptor(bi_post_proc_input_tex_table, tto_postp_input, srv->GetCPUhandle());
		}
	}

	m_post_process_quad->LoadDataToGpu(command_list);
	if (std::shared_ptr<GpuResource> rt = m_gui->GetGuiQuad()->GetRt(m_frameIndex).lock()) {
		if (std::shared_ptr<ResourceDescriptor> srv = rt->GetSRV().lock()) {
			m_commandQueueGfx->GetGpuHeap().StageDesctriptor(bi_post_proc_input_tex_table, tto_postp_gui, srv->GetCPUhandle());
		}
	}

	if (std::shared_ptr<GpuResource> rt = m_forward_quad->GetRt(m_frameIndex).lock()) {
		if (std::shared_ptr<ResourceDescriptor> srv = rt->GetSRV().lock()) {
			m_commandQueueGfx->GetGpuHeap().StageDesctriptor(bi_post_proc_input_tex_table, tto_postp_fwd, srv->GetCPUhandle());
		}
	}

	if (std::shared_ptr<ResourceDescriptor> srv = m_ssao->GetSSAOres(1)->GetSRV().lock()) {
		m_commandQueueGfx->GetGpuHeap().StageDesctriptor(bi_post_proc_input_tex_table, tto_postp_ssao, srv->GetCPUhandle());
	}

	CommitCB(command_list, cb_scene);

	m_commandQueueGfx->GetGpuHeap().CommitRootSignature(command_list);

	m_post_process_quad->Render(command_list);
}

void DXAppImplementation::RenderForwardQuad(ComPtr<ID3D12GraphicsCommandList6>& command_list) {
	{
		std::vector<DXGI_FORMAT> formats = { DXGI_FORMAT_R16G16B16A16_FLOAT };
		m_forward_quad->CreateQuadTexture(m_width, m_height, formats, FrameCount, 0, L"m_forward_quad_");
	}

	if (std::shared_ptr<GpuResource> rt = m_forward_quad->GetRt(m_frameIndex).lock()) {
		m_commandQueueGfx->ResourceBarrier(rt, D3D12_RESOURCE_STATE_RENDER_TARGET);
		PrepareRenderTarget(command_list, *rt.get(), true, false);
	}
	else {
		assert(false);
	}

	// water
	m_level->RenderWater(command_list);

	if (std::shared_ptr<GpuResource> rt = m_forward_quad->GetRt(m_frameIndex).lock()) {
		m_commandQueueGfx->ResourceBarrier(rt, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	//m_post_process_quad->Render(command_list);
}

void DXAppImplementation::RenderDeferredShadingQuad(ComPtr<ID3D12GraphicsCommandList6>& command_list) {
	m_commandQueueGfx->ResourceBarrier(*m_ssao->GetSSAOres(2), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	m_commandQueueGfx->ResourceBarrier(*m_depthStencil.get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);

	{
		std::vector< std::shared_ptr<GpuResource>>& rts = m_deferred_shading_quad->GetRts(m_frameIndex);
		for (uint32_t i = 0; i < rts.size(); i++) {
			if (i == 1 || i == 2) {
				m_commandQueueGfx->ResourceBarrier(rts[i], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			}
		}
	}

	{
		std::vector<DXGI_FORMAT> formats = { DXGI_FORMAT_R16G16B16A16_FLOAT };
		m_post_process_quad->CreateQuadTexture(m_width, m_height, formats, FrameCount, 0, L"m_post_process_quad_");
		if (std::shared_ptr<GpuResource> rt = m_post_process_quad->GetRt(m_frameIndex).lock()) {
			m_commandQueueGfx->ResourceBarrier(rt, D3D12_RESOURCE_STATE_RENDER_TARGET);
			PrepareRenderTarget(command_list, m_post_process_quad->GetRts(m_frameIndex), false);
		}
		else {
			assert(false);
		}
	}

	// set technique
	const Techniques::Technique* tech = GetTechniqueById(tt_deferred_shading);
	if (m_commandQueueGfx->GetPSO() != tt_deferred_shading) {
		m_commandQueueGfx->SetPSO(tt_deferred_shading);
	}
	if (m_commandQueueGfx->GetRootSign() != tech->root_signature) {
		m_commandQueueGfx->SetRootSign(tech->root_signature);
		m_commandQueueGfx->GetGpuHeap().CacheRootSignature(gD3DApp->GetRootSignById(tech->root_signature));
	}

	m_deferred_shading_quad->LoadDataToGpu(command_list);
	gD3DApp->CommitCB(command_list, cb_scene);
	m_level->BindLights(command_list);

	auto& rts_vec = m_deferred_shading_quad->GetRts(m_frameIndex);
	for (uint32_t i = 0; i < rts_vec.size(); i++) {
		std::shared_ptr<GpuResource>& rt = rts_vec[i];
		if (std::shared_ptr<ResourceDescriptor> srv = rt->GetSRV().lock()) {
			m_commandQueueGfx->GetGpuHeap().StageDesctriptor(bi_deferred_shading_tex_table, TextureTableOffset(i), srv->GetCPUhandle());
		}
	}

	if (std::shared_ptr<ResourceDescriptor> srv = m_ssao->GetSSAOres(1)->GetSRV().lock()) {
		m_commandQueueGfx->GetGpuHeap().StageDesctriptor(bi_deferred_shading_tex_table, tto_gbuff_ssao, srv->GetCPUhandle());
	}

	m_commandQueueGfx->GetGpuHeap().CommitRootSignature(command_list);

	m_deferred_shading_quad->Render(command_list);
}

void DXAppImplementation::RenderSSAOquad(ComPtr<ID3D12GraphicsCommandList6>& command_list) {
	m_commandQueueCompute->ResourceBarrier(*m_ssao->GetSSAOres(0), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	const Techniques::Technique* tech = GetTechniqueById(tt_ssao);
	if (m_commandQueueCompute->GetPSO() != tt_ssao) {
		m_commandQueueCompute->SetPSO(tt_ssao);
	}
	if (m_commandQueueCompute->GetRootSign() != tech->root_signature) {
		m_commandQueueCompute->SetRootSign(tech->root_signature);
		m_commandQueueCompute->GetGpuHeap().CacheRootSignature(gD3DApp->GetRootSignById(tech->root_signature));
	}

	if (std::shared_ptr<GpuResource> rt = m_deferred_shading_quad->GetRt(m_frameIndex, 1).lock()) {
		if (std::shared_ptr<ResourceDescriptor> srv = rt->GetSRV().lock()) {
			m_commandQueueCompute->GetGpuHeap().StageDesctriptor(bi_ssao_input_tex, tto_ssao_normals, srv->GetCPUhandle());
		}
	}
	if (std::shared_ptr<GpuResource> rt = m_deferred_shading_quad->GetRt(m_frameIndex, 2).lock()) {
		if (std::shared_ptr<ResourceDescriptor> srv = rt->GetSRV().lock()) {
			m_commandQueueCompute->GetGpuHeap().StageDesctriptor(bi_ssao_input_tex, tto_ssao_positions, srv->GetCPUhandle());
		}
	}
	if (std::shared_ptr<ResourceDescriptor> srv = m_ssao->GetRandomVals()->GetSRV().lock()) {
		m_commandQueueCompute->GetGpuHeap().StageDesctriptor(bi_ssao_input_tex, tto_ssao_random_vals, srv->GetCPUhandle());
	}
	if (std::shared_ptr<ResourceDescriptor> depth_view = m_depthStencil->GetSRV().lock()) {
		m_commandQueueCompute->GetGpuHeap().StageDesctriptor(bi_ssao_input_tex, tto_ssao_depth, depth_view->GetCPUhandle());
	}
	if (std::shared_ptr<ResourceDescriptor> uav = m_ssao->GetSSAOres(0)->GetUAV().lock()) {
		m_commandQueueCompute->GetGpuHeap().StageDesctriptor(bi_ssao_uav_tex, tto_ssao_blur_uav, uav->GetCPUhandle());
	}

	gD3DApp->CommitCB(command_list, cb_scene, false);
	// bind ssao cb
	m_ssao->GenerateSSAO(command_list, false);

	m_commandQueueCompute->GetGpuHeap().CommitRootSignature(command_list, false);

	const float threads_num = 32.f;
	command_list->Dispatch((uint32_t)ceilf(float(m_width) / threads_num), (uint32_t)ceilf(float(m_height) / threads_num), 1);

	m_commandQueueCompute->ResourceBarrier(*m_ssao->GetSSAOres(0), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	m_commandQueueCompute->ResourceBarrier(*m_ssao->GetSSAOres(1), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

void DXAppImplementation::BlurSSAO(ComPtr<ID3D12GraphicsCommandList6>& command_list)
{
	const Techniques::Technique* tech = GetTechniqueById(tt_blur);
	if (m_commandQueueCompute->GetPSO() != tt_blur) {
		m_commandQueueCompute->SetPSO(tt_blur);
	}
	if (m_commandQueueCompute->GetRootSign() != tech->root_signature) {
		m_commandQueueCompute->SetRootSign(tech->root_signature);
	}
	m_commandQueueCompute->GetGpuHeap().CacheRootSignature(gD3DApp->GetRootSignById(tech->root_signature));

	if (std::shared_ptr<ResourceDescriptor> srv = m_ssao->GetSSAOres(0)->GetSRV().lock()) {
		m_commandQueueCompute->GetGpuHeap().StageDesctriptor(bi_ssao_input_tex, tto_ssao_blur_srv, srv->GetCPUhandle());
	}

	if (std::shared_ptr<ResourceDescriptor> uav = m_ssao->GetSSAOres(1)->GetUAV().lock()) {
		m_commandQueueCompute->GetGpuHeap().StageDesctriptor(bi_ssao_uav_tex, tto_ssao_blur_uav, uav->GetCPUhandle());
	}
	// bind ssao cb
	//m_ssao->GenerateSSAO(command_list, false);

	m_commandQueueCompute->GetGpuHeap().CommitRootSignature(command_list, false);
	const float threads_num = 32.f;
	command_list->Dispatch((uint32_t)ceilf(float(m_width) / threads_num), (uint32_t)ceilf(float(m_height) / threads_num), 1);

	// vertical pass
	m_commandQueueCompute->GetGpuHeap().CacheRootSignature(gD3DApp->GetRootSignById(tech->root_signature));

	m_commandQueueCompute->ResourceBarrier(*m_ssao->GetSSAOres(1), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);


	if (std::shared_ptr<ResourceDescriptor> srv = m_ssao->GetSSAOres(1)->GetSRV().lock()) {
		m_commandQueueCompute->GetGpuHeap().StageDesctriptor(bi_ssao_input_tex, tto_ssao_blur_srv, srv->GetCPUhandle());
	}

	if (std::shared_ptr<ResourceDescriptor> uav = m_ssao->GetSSAOres(2)->GetUAV().lock()) {
		m_commandQueueCompute->GetGpuHeap().StageDesctriptor(bi_ssao_uav_tex, tto_ssao_blur_uav, uav->GetCPUhandle());
	}

	m_commandQueueCompute->GetGpuHeap().CommitRootSignature(command_list, false);

	command_list->Dispatch((uint32_t)ceilf(float(m_width) / threads_num), (uint32_t)ceilf(float(m_height) / threads_num), 1);
}

void DXAppImplementation::OnMouseMoved(WPARAM btnState, int x, int y) {
	if ((btnState & MK_RBUTTON) != 0) {
		m_camera_movement.camera_x_delta = x - m_camera_movement.camera_x;
		m_camera_movement.camera_y_delta = y - m_camera_movement.camera_y;
	}

	m_camera_movement.camera_x = x;
	m_camera_movement.camera_y = y;
}

void DXAppImplementation::OnKeyDown(UINT8 key) {
	if (key != VK_OEM_3 && m_gui && m_gui->WantCapture(ImguiHelper::CaptureInput_type::cit_keyboard)) {
		return;
	}
	switch (key) {
	case 'W':
		m_camera_movement.camera_movement_state |= m_camera_movement.cm_fwd;
		break;
	case 'A':
		m_camera_movement.camera_movement_state |= m_camera_movement.cm_left;
		break;
	case 'S':
		m_camera_movement.camera_movement_state |= m_camera_movement.cm_bcwd;
		break;
	case 'D':
		m_camera_movement.camera_movement_state |= m_camera_movement.cm_right;
		break;
	case VK_OEM_3:
		m_gui->ShowConsole();
		break;
	}
}

void DXAppImplementation::OnKeyUp(UINT8 key) {
	if (m_gui && m_gui->WantCapture(ImguiHelper::CaptureInput_type::cit_keyboard)) {
		return;
	}
	switch (key) {
	case 'W':
		m_camera_movement.camera_movement_state &= (~m_camera_movement.cm_fwd);
		break;
	case 'A':
		m_camera_movement.camera_movement_state &= (~m_camera_movement.cm_left);
		break;
	case 'S':
		m_camera_movement.camera_movement_state &= (~m_camera_movement.cm_bcwd);
		break;
	case 'D':
		m_camera_movement.camera_movement_state &= (~m_camera_movement.cm_right);
		break;
	}
}

void DXAppImplementation::RebuildShaders(std::optional<std::wstring> dbg_name /*= std::nullopt*/)
{
	m_rebuild_shaders = true;
}

void DXAppImplementation::UpdateCamera(std::shared_ptr<FreeCamera>& camera, float dt) {
	auto move_func = [&camera, dt](const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& dir, bool neg) {
		const float camera_speed = 30;
		const DirectX::XMVECTOR dir_vec = neg ? DirectX::XMVectorNegate(DirectX::XMLoadFloat3(&dir)) : DirectX::XMLoadFloat3(&dir);
		const DirectX::XMVECTOR pos_vec = DirectX::XMLoadFloat3(&pos);
		DirectX::XMVECTOR new_pos_vec = DirectX::XMVectorMultiplyAdd(dir_vec, DirectX::XMLoadFloat3(&DirectX::XMFLOAT3(camera_speed * dt, camera_speed * dt, camera_speed * dt)), pos_vec);
		camera->Move(new_pos_vec);
	};

	if ((m_camera_movement.camera_movement_state & m_camera_movement.cm_fwd) && !(m_camera_movement.camera_movement_state & m_camera_movement.cm_bcwd)) {
		move_func(camera->GetPosition(), camera->GetDirection(), false);
	}
	else if ((m_camera_movement.camera_movement_state & m_camera_movement.cm_bcwd) && !(m_camera_movement.camera_movement_state & m_camera_movement.cm_fwd)) {
		move_func(camera->GetPosition(), camera->GetDirection(), true);
	}
	if ((m_camera_movement.camera_movement_state & m_camera_movement.cm_right) && !(m_camera_movement.camera_movement_state & m_camera_movement.cm_left)) {
		move_func(camera->GetPosition(), camera->GetRightDirection(), false);
	}
	else if ((m_camera_movement.camera_movement_state & m_camera_movement.cm_left) && !(m_camera_movement.camera_movement_state & m_camera_movement.cm_right)) {
		move_func(camera->GetPosition(), camera->GetRightDirection(), true);
	}

	auto rot_func = [&camera, dt](const DirectX::XMFLOAT3& dir, const DirectX::XMFLOAT3& rot_axis, bool x, uint32_t w, uint32_t h, int32_t dx, int32_t dy) {

		const float ang = DirectX::XMConvertToRadians(0.25f * static_cast<float>(x ? dx : dy));

		const DirectX::XMVECTOR dir_vec = DirectX::XMLoadFloat3(&dir);
		const DirectX::XMVECTOR axis_vec = DirectX::XMLoadFloat3(&rot_axis);

		DirectX::XMMATRIX rot_mx = DirectX::XMMatrixRotationAxis(axis_vec, ang);
		const DirectX::XMVECTOR new_dir = DirectX::XMVector3TransformNormal(dir_vec, rot_mx);

		camera->Rotate(new_dir);
	};

	if (m_camera_movement.camera_x_delta != 0) {
		rot_func(camera->GetDirection(), camera->GetUpDirection(), true, m_width, m_height, m_camera_movement.camera_x_delta, 0);
		m_camera_movement.camera_x_delta = 0;
	}

	if (m_camera_movement.camera_y_delta != 0) {
		rot_func(camera->GetDirection(), camera->GetRightDirection(), false, m_width, m_height, 0, m_camera_movement.camera_y_delta);
		m_camera_movement.camera_y_delta = 0;
	}
}
