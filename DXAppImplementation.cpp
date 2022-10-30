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
    m_depthStencil(std::make_unique<GpuResource>())
{
}

DXAppImplementation::~DXAppImplementation() = default;

void DXAppImplementation::OnInit()
{
    TODO("Minor! SetName should be used to make debugging easier")
    
    m_start_time = std::chrono::system_clock::now();
    gD3DApp = this;
    ResourceManager::OnInit();
    
    LoadPipeline();
    LoadAssets();
    m_level = std::make_shared<Level>();
    m_level->Load(L"test_level.json");

    LoadTechnique();
}

// Load the rendering pipeline dependencies.
void DXAppImplementation::LoadPipeline()
{
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

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    if (m_useWarpDevice)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

        ThrowIfFailed(D3D12CreateDevice(
            warpAdapter.Get(),
            D3D_FEATURE_LEVEL_12_0,
            IID_PPV_ARGS(&m_device)
            ));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);

        ThrowIfFailed(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_12_0,
            IID_PPV_ARGS(&m_device)
            ));
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
    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

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
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        Application::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
        ));

    // This sample does not support fullscreen transitions.
    ThrowIfFailed(factory->MakeWindowAssociation(Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain.As(&m_swapChain));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    
    // Create descriptor heaps.
    m_descriptor_heap_collection->Initialize();

    // Create frame resources.
    {
        // Create a RTV for each frame.
        for (uint32_t n = 0; n < FrameCount; n++)
        {
            ComPtr<ID3D12Resource> renderTarget;
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&renderTarget)));
            m_renderTargets[n].SetBuffer(renderTarget);
            m_renderTargets[n].CreateRTV();

            ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator[n])));
        }
    }
}

// Load the sample assets.
void DXAppImplementation::LoadAssets()
{
    // Create the command list.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[m_frameIndex].Get(), nullptr, IID_PPV_ARGS(&m_commandList)));

    // Command lists are created in the recording state, but there is nothing
    // to record yet. The main loop expects it to be closed, so close it now.
    ThrowIfFailed(m_commandList->Close());

    // Create synchronization objects.
    {
        ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }
    }

    // Create the depth stencil view.
    {
        D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
        depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
        depthOptimizedClearValue.DepthStencil.Stencil = 0;
        CD3DX12_RESOURCE_DESC res_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
        
        m_depthStencil->CreateTexture(HeapBuffer::BufferType::bt_deafult, res_desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, depthOptimizedClearValue);

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

    m_level->Update(m_dt.count());
}

// Render the scene.
void DXAppImplementation::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));

    // Signal for this frame
    m_fenceValues[m_frameIndex] = Signal(m_fenceValue);

    // get next frame
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Wait for signal from new frame
    Wait(m_fenceValues[m_frameIndex]);
}

void DXAppImplementation::OnDestroy()
{
    gD3DApp = nullptr;
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForPreviousFrame();

    CloseHandle(m_fenceEvent);
}

void DXAppImplementation::PopulateCommandList()
{
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_commandAllocator[m_frameIndex]->Reset());

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator[m_frameIndex].Get(), m_pipelineState.Get()));

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    ComPtr<ID3D12Resource> render_target;
    if (std::shared_ptr<HeapBuffer> render_target_buff = m_renderTargets[m_frameIndex].GetBuffer().lock()){
        render_target = render_target_buff->GetResource();
    }
    else {
        assert(false);
    }

    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(render_target.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
    if (std::shared_ptr<ResourceDescriptor> render_target_view = m_renderTargets[m_frameIndex].GetRTV().lock()){
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

    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
    m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Record commands.
    const float clearColor[] = { 0.2f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    RenderCube();

    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(render_target.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());
}

void DXAppImplementation::WaitForPreviousFrame()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    // Signal and increment the fence value.
    const uint64_t fence = ++m_fenceValue;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));

    // Wait until the previous frame is finished.
    if (m_fence->GetCompletedValue() < fence)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

uint64_t DXAppImplementation::Signal(uint64_t &fenceValue){
    uint64_t fenceValueForSignal = ++fenceValue;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fenceValueForSignal));

    return fenceValueForSignal;
}

void DXAppImplementation::Wait(uint64_t fenceValue){
    if (m_fence->GetCompletedValue() < fenceValue)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

void DXAppImplementation::LoadTechnique(){
    ThrowIfFailed(m_commandAllocator[m_frameIndex]->Reset());
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator[m_frameIndex].Get(), m_pipelineState.Get()));
    /*
    prepare/load technique(inputLayout + rootSignature + vs + ps + pipeline)
    */
TODO("Critical! Create Technique to avoid this shity code")
    // Create the vertex input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        //{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // Create a root signature.
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

    // A single 32-bit constant root parameter that is used by the vertex shader.
    CD3DX12_ROOT_PARAMETER1 rootParameters[1];
    rootParameters[0].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

    // Serialize the root signature.
    ComPtr<ID3DBlob> rootSignatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
        featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
    // Create the root signature.
    ThrowIfFailed(m_device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
        rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    IDxcBlob* vs_blob = m_shaderMgr->GetShaderBLOB(L"vertex_shader.hlsl");
    auto vs = CD3DX12_SHADER_BYTECODE(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize());
    IDxcBlob* ps_blob = m_shaderMgr->GetShaderBLOB(L"pixel_shader.hlsl");
    auto ps = CD3DX12_SHADER_BYTECODE(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize());

    pipelineStateStream.pRootSignature = m_rootSignature.Get();
    pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS = vs;
    pipelineStateStream.PS = ps;
    pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStream.RTVFormats = rtvFormats;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
        sizeof(PipelineStateStream), &pipelineStateStream
    };
    ThrowIfFailed(m_device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_PipelineState)));

    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    WaitForPreviousFrame();
}

void DXAppImplementation::RenderCube(){
    TODO("Critical! Create Gatherer or RenderScene to avoid this shity code")
    /*
    renderer who gather meshes in correct order (based on technique(to avoid swapping PSO etc.) AND able to draw instances
    */
    if (m_level->GetEntityCount() == 1){
        constexpr uint32_t id = 0;
        LevelEntity ent = m_level->GetEntityById(id);

        DirectX::XMMATRIX m_ModelMatrix = DirectX::XMLoadFloat4x4(&ent.GetXform());
        DirectX::XMMATRIX m_ViewMatrix;
        DirectX::XMMATRIX m_ProjectionMatrix;
        if (std::shared_ptr<FreeCamera> camera = m_level->GetCamera().lock()){
            m_ViewMatrix = DirectX::XMLoadFloat4x4(&camera->GetViewMx());
            m_ProjectionMatrix = DirectX::XMLoadFloat4x4(&camera->GetProjMx());
        }
        DirectX::XMMATRIX mvpMatrix = DirectX::XMMatrixMultiply(m_ModelMatrix, m_ViewMatrix);
        mvpMatrix = XMMatrixMultiply(mvpMatrix, m_ProjectionMatrix);
        
        /*
        should be aply technique params here
        */
        m_commandList->SetGraphicsRoot32BitConstants(0, sizeof(DirectX::XMMATRIX) / 4, &mvpMatrix, 0);
        m_commandList->SetPipelineState(m_PipelineState.Get());
        m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

        ent.Render(m_commandList);
    }
    else {
        assert(false);
    }
}