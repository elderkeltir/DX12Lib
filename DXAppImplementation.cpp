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

#include "WinPixEventRuntime/pix3.h"

#if defined(USE_PIX) && defined(USE_PIX_DEBUG)
#include "PIXapi.h"
#endif // defined(USE_PIX) && defined(USE_PIX_DEBUG)

DXAppImplementation *gD3DApp = nullptr;

DXAppImplementation::DXAppImplementation(uint32_t width, uint32_t height, std::wstring name) :
    DXApp(width, height, name),
    m_frameIndex(0),
    m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
    m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_descriptor_heap_collection(std::make_shared<DescriptorHeapCollection>()),
    m_renderTargets(std::make_unique<GpuResource[]>(FrameCount)),
    m_depthStencil(std::make_unique<GpuResource>()),
    m_commandQueueGfx(std::make_unique<GfxCommandQueue>()),
    m_post_process_quad(std::make_unique<RenderQuad>()),
    m_deferred_shading_quad(std::make_unique<RenderQuad>())
{
}

DXAppImplementation::~DXAppImplementation() = default;

void DXAppImplementation::OnInit()
{
    m_start_time = std::chrono::system_clock::now();
    gD3DApp = this;
    ResourceManager::OnInit();
    CreateDevice(L"DXAppImplementation");
    m_commandQueueGfx->OnInit(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT, L"Gfx");
    CreateSwapChain(L"DXAppImplementation");
    Techniques::OnInit(m_device, L"PredefinedTechinue");

    m_level = std::make_shared<Level>();
    m_level->Load(L"test_level.json");

    m_post_process_quad->Initialize();
    m_deferred_shading_quad->Initialize();
}

void DXAppImplementation::CreateDevice(std::optional<std::wstring> dbg_name){
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

void DXAppImplementation::CreateSwapChain(std::optional<std::wstring> dbg_name){
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
    }
}

// Update frame-based values.
void DXAppImplementation::OnUpdate()
{
    // Time logic
    m_frame_id++;
    std::chrono::time_point t = std::chrono::system_clock::now();
    m_dt = t-m_time;
    m_time = t;
    m_total_time = t - m_start_time;

    if (std::shared_ptr<FreeCamera> camera = m_level->GetCamera().lock()){
        UpdateCamera(camera, m_dt.count());
    }

    m_level->Update(m_dt.count());
}

// Render the scene.
void DXAppImplementation::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    ComPtr<ID3D12GraphicsCommandList6>& command_list = m_commandQueueGfx->ResetActiveCL();

    // Set default settings
    command_list->RSSetViewports(1, &m_viewport);
    command_list->RSSetScissorRects(1, &m_scissorRect);

    // Clear rt and set proper state
    BEGIN_EVENT(command_list, "G-Buffer");
    {
        std::vector<DXGI_FORMAT> formats = { DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R8G8B8A8_SNORM, DXGI_FORMAT_R16G16B16A16_FLOAT };
        m_deferred_shading_quad->CreateQuadTexture(m_width, m_height, formats, FrameCount, L"m_deferred_shading_quad_");
        std::vector<std::shared_ptr<GpuResource>>& rts = m_deferred_shading_quad->GetRts(m_frameIndex);
        for (uint32_t i = 0; i < rts.size(); i++){
            m_commandQueueGfx->ResourceBarrier(rts.at(i), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
        }
        PrepareRenderTarget(command_list, rts);
    }

    // Render scene
    RenderLevel(command_list);
    END_EVENT(command_list);

    // deferred shading setup rts
    BEGIN_EVENT(command_list, "Deferred Shading");
    {
        std::vector< std::shared_ptr<GpuResource>>& rts = m_deferred_shading_quad->GetRts(m_frameIndex);
        for (uint32_t i = 0; i < rts.size(); i++){
            m_commandQueueGfx->ResourceBarrier(rts.at(i), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        }
        std::vector<DXGI_FORMAT> formats = { DXGI_FORMAT_R16G16B16A16_FLOAT };
        m_post_process_quad->CreateQuadTexture(m_width, m_height, formats, FrameCount, L"m_post_process_quad_");
        if (std::shared_ptr<GpuResource> rt = m_post_process_quad->GetRt(m_frameIndex).lock()){
            m_commandQueueGfx->ResourceBarrier(rt, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            PrepareRenderTarget(command_list, m_post_process_quad->GetRts(m_frameIndex));
        }
        else {
            assert(false);
        }
    }

    // deferred shading
    RenderDeferredShadingQuad(command_list);
    END_EVENT(command_list);

    // post process
    BEGIN_EVENT(command_list, "Post Processing");
    if (std::shared_ptr<GpuResource> rt = m_post_process_quad->GetRt(m_frameIndex).lock()){
        m_commandQueueGfx->ResourceBarrier(rt, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
    else {
        assert(false);
    }

    m_commandQueueGfx->ResourceBarrier(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    PrepareRenderTarget(command_list, m_renderTargets[m_frameIndex]);

    RenderPostProcessQuad(command_list);
    END_EVENT(command_list);

    // Indicate that the back buffer will now be used to present.
    m_commandQueueGfx->ResourceBarrier(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

    m_commandQueueGfx->ExecuteActiveCL();

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));

    // Signal for this frame
    m_fenceValues[m_frameIndex] = m_commandQueueGfx->Signal();

    // get next frame
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Wait for signal from new frame
    m_commandQueueGfx->Wait(m_fenceValues[m_frameIndex]);
}

void DXAppImplementation::OnDestroy()
{
    m_commandQueueGfx->OnDestroy();
    gD3DApp = nullptr;
}

void DXAppImplementation::PrepareRenderTarget(ComPtr<ID3D12GraphicsCommandList6> &command_list, const std::vector<std::shared_ptr<GpuResource>> &rts){
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
    if (std::shared_ptr<ResourceDescriptor> render_target_view = rts.at(0)->GetRTV().lock()){
        rtvHandle = render_target_view->GetCPUhandle();
    }
    else {
        assert(false);
    }
    if (std::shared_ptr<ResourceDescriptor> depth_view = m_depthStencil->GetDSV().lock()){
        dsvHandle = depth_view->GetCPUhandle();
    }
    else {
        assert(false);
    }

    command_list->OMSetRenderTargets((uint32_t)rts.size(), &rtvHandle, rts.size() > 1 ? TRUE : FALSE, &dsvHandle);
    command_list->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Record commands.
    const float clearColor[] = { 0.2f, 0.2f, 0.4f, 1.0f };
    for (auto & rt : rts){
        if (std::shared_ptr<ResourceDescriptor> render_target_view = rt->GetRTV().lock()){
            rtvHandle = render_target_view->GetCPUhandle();
            command_list->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
        }
    }
}

void DXAppImplementation::PrepareRenderTarget(ComPtr<ID3D12GraphicsCommandList6> &command_list, GpuResource &rt) {
   CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
    if (std::shared_ptr<ResourceDescriptor> render_target_view = rt.GetRTV().lock()){
        rtvHandle = render_target_view->GetCPUhandle();
    }
    else {
        assert(false);
    }
    if (std::shared_ptr<ResourceDescriptor> depth_view = m_depthStencil->GetDSV().lock()){
        dsvHandle = depth_view->GetCPUhandle();
    }
    else {
        assert(false);
    }

    command_list->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
    command_list->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Record commands.
    const float clearColor[] = { 0.2f, 0.2f, 0.4f, 1.0f };
    command_list->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
}

void DXAppImplementation::RenderLevel(ComPtr<ID3D12GraphicsCommandList6>& command_list){
    TODO("Normal! Create Gatherer or RenderScene to avoid this shity code")
    bool is_scene_constants_set = false;
    for (uint32_t id = 0; id < m_level->GetEntityCount(); id++){
        LevelEntity ent = m_level->GetEntityById(id);
        ent.LoadDataToGpu(command_list);

        const Technique *tech = GetTechniqueById(ent.GetTechniqueId());

        // set technique
        command_list->SetPipelineState(tech->pipeline_state.Get());
        command_list->SetGraphicsRootSignature(tech->root_signature.Get());

        // set root desc
        if (ent.GetTechniqueId() == 1){
            ID3D12DescriptorHeap* descriptorHeaps[] = { m_descriptor_heap_collection->GetShaderVisibleHeap().Get() };
            command_list->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
        }

        if (!is_scene_constants_set){
            if (std::shared_ptr<FreeCamera> camera = m_level->GetCamera().lock()){
                DirectX::XMMATRIX m_ViewMatrix = DirectX::XMLoadFloat4x4(&camera->GetViewMx());
                DirectX::XMMATRIX m_ProjectionMatrix = DirectX::XMLoadFloat4x4(&camera->GetProjMx());
                TODO("Critical! Implement processing of different root desc setup")
                gD3DApp->SetMatrix4Constant(Constants::cV, m_ViewMatrix, command_list);
                gD3DApp->SetMatrix4Constant(Constants::cP, m_ProjectionMatrix, command_list);
            }
            is_scene_constants_set = !is_scene_constants_set;
        }

        ent.Render(command_list);
    }
}

void DXAppImplementation::RenderPostProcessQuad(ComPtr<ID3D12GraphicsCommandList6>& command_list){
    // set technique
    const Techniques::Technique* tech = GetTechniqueById(2);
    command_list->SetPipelineState(tech->pipeline_state.Get());
    command_list->SetGraphicsRootSignature(tech->root_signature.Get());

    // set root desc
    ID3D12DescriptorHeap* descriptorHeaps[] = { m_descriptor_heap_collection->GetShaderVisibleHeap().Get() };
    command_list->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    m_post_process_quad->LoadDataToGpu(command_list);
    m_post_process_quad->SetSrv(command_list, m_frameIndex, 0);
    
    m_post_process_quad->Render(command_list);
}

void DXAppImplementation::RenderDeferredShadingQuad(ComPtr<ID3D12GraphicsCommandList6>& command_list) {
    // set technique
    const Techniques::Technique* tech = GetTechniqueById(3);
    command_list->SetPipelineState(tech->pipeline_state.Get());
    command_list->SetGraphicsRootSignature(tech->root_signature.Get());

    // set root desc
    ID3D12DescriptorHeap* descriptorHeaps[] = { m_descriptor_heap_collection->GetShaderVisibleHeap().Get() };
    command_list->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    m_deferred_shading_quad->LoadDataToGpu(command_list);
    if (std::shared_ptr<FreeCamera> camera = m_level->GetCamera().lock()){
        DirectX::XMFLOAT3 cam_pos;
        cam_pos = camera->GetPosition();
        gD3DApp->SetVector3Constant(Constants::cCP, cam_pos, command_list);
    }
    m_deferred_shading_quad->SetSrv(command_list, m_frameIndex, 1);
    
    m_deferred_shading_quad->Render(command_list);
}

void DXAppImplementation::OnMouseMoved(WPARAM btnState, int x, int y){
	if ((btnState & MK_RBUTTON) != 0) {
			m_camera_movement.camera_x_delta = x - m_camera_movement.camera_x;
			m_camera_movement.camera_y_delta = y - m_camera_movement.camera_y;
	}

	m_camera_movement.camera_x = x;
	m_camera_movement.camera_y = y;
}

void DXAppImplementation::OnKeyDown(UINT8 key) {
    switch(key){
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
    }
}

void DXAppImplementation::OnKeyUp(UINT8 key) {
    switch(key){
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

void DXAppImplementation::UpdateCamera(std::shared_ptr<FreeCamera> &camera, float dt){
    auto move_func = [&camera,dt](const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& dir, bool neg){
           const float camera_speed = 30; 
        const DirectX::XMVECTOR dir_vec = neg ? DirectX::XMVectorNegate(DirectX::XMLoadFloat3(&dir)) : DirectX::XMLoadFloat3(&dir);
        const DirectX::XMVECTOR pos_vec = DirectX::XMLoadFloat3(&pos);
        DirectX::XMVECTOR new_pos_vec = DirectX::XMVectorMultiplyAdd(dir_vec, DirectX::XMLoadFloat3(&DirectX::XMFLOAT3(camera_speed * dt, camera_speed * dt, camera_speed * dt)), pos_vec);
        camera->Move(new_pos_vec);
    };

    if ((m_camera_movement.camera_movement_state & m_camera_movement.cm_fwd) && !(m_camera_movement.camera_movement_state & m_camera_movement.cm_bcwd)){
        move_func(camera->GetPosition(), camera->GetDirection(), false);
    }
    else if ((m_camera_movement.camera_movement_state & m_camera_movement.cm_bcwd) && !(m_camera_movement.camera_movement_state & m_camera_movement.cm_fwd)){
        move_func(camera->GetPosition(), camera->GetDirection(), true);
    }
    if ((m_camera_movement.camera_movement_state & m_camera_movement.cm_right) && !(m_camera_movement.camera_movement_state & m_camera_movement.cm_left)){
        move_func(camera->GetPosition(), camera->GetRightDirection(), false);
    }
    else if ((m_camera_movement.camera_movement_state & m_camera_movement.cm_left) && !(m_camera_movement.camera_movement_state & m_camera_movement.cm_right)){
        move_func(camera->GetPosition(), camera->GetRightDirection(), true);
    }

    auto rot_func = [&camera,dt](const DirectX::XMFLOAT3& dir, const DirectX::XMFLOAT3& rot_axis, bool x, uint32_t w, uint32_t h, int32_t dx, int32_t dy){

        const float ang = DirectX::XMConvertToRadians(0.25f*static_cast<float>( x ? dx : dy));

        const DirectX::XMVECTOR dir_vec = DirectX::XMLoadFloat3(&dir);
        const DirectX::XMVECTOR axis_vec = DirectX::XMLoadFloat3(&rot_axis);

        DirectX::XMMATRIX rot_mx = DirectX::XMMatrixRotationAxis(axis_vec, ang);
        const DirectX::XMVECTOR new_dir = DirectX::XMVector3TransformNormal(dir_vec, rot_mx);

        camera->Rotate(new_dir);
    };

    if (m_camera_movement.camera_x_delta != 0){
        rot_func(camera->GetDirection(), camera->GetUpDirection(), true, m_width, m_height, m_camera_movement.camera_x_delta, 0);
        m_camera_movement.camera_x_delta = 0;
    }

    if (m_camera_movement.camera_y_delta != 0){
        rot_func(camera->GetDirection(), camera->GetRightDirection(), false, m_width, m_height, 0, m_camera_movement.camera_y_delta);
        m_camera_movement.camera_y_delta = 0;
    } 
}