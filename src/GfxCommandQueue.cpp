#include "GfxCommandQueue.h"
#include "DXHelper.h"
#include "GpuResource.h"
#include <directx/d3dx12.h>
#include "RootSignature.h"


#include "DXAppImplementation.h"

extern DXAppImplementation *gD3DApp;

void GfxCommandQueue::OnInit(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type, uint32_t command_list_num, std::optional<std::wstring> dbg_name){
    m_command_list_num = command_list_num;
    m_type = type;
    m_command_list.m_queue = this;

    // allocate
    m_dynamic_gpu_heaps.swap(std::make_unique<DynamicGpuHeap[]>(m_command_list_num));
    m_commandAllocator.swap(std::make_unique<ComPtr<ID3D12CommandAllocator>[]>(m_command_list_num));

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = type;
    ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
    SetName(m_commandQueue, dbg_name.value_or(L"").append(L"_direct_queue_"+ std::to_wstring(type)).c_str());

    for (uint32_t i = 0; i < m_command_list_num; i++){
        ThrowIfFailed(device->CreateCommandAllocator(type, IID_PPV_ARGS(&m_commandAllocator[i])));
        SetName(m_commandAllocator[i], dbg_name.value_or(L"").append(L"_cmd_allocator_" + std::to_wstring(type)).append(std::to_wstring(i)).c_str());
    }
    ThrowIfFailed(device->CreateCommandList(0, type, m_commandAllocator[m_active_cl].Get(), nullptr, IID_PPV_ARGS(&m_command_list.m_command_list)));
    SetName(m_command_list.m_command_list, dbg_name.value_or(L"").append(L"_cmd_list_" + std::to_wstring(type)).c_str());
    ThrowIfFailed(m_command_list.m_command_list->Close());

    {
        ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        SetName(m_fence, dbg_name.value_or(L"").append(L"_fence").c_str());
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }
    }

    for (uint32_t id = 0; id < m_command_list_num; id++) {
        m_dynamic_gpu_heaps[id].Initialize(id);
    }
}

CommandList& GfxCommandQueue::ResetActiveCL(ID3D12PipelineState *pipeline_state) {
    m_active_cl = ++m_active_cl % m_command_list_num;
    
    // reset PSO and root_sig
    m_pso = uint32_t(-1);
    m_root_sign = uint32_t(-1);
    
    ThrowIfFailed(m_commandAllocator[m_active_cl]->Reset());
    ThrowIfFailed(m_command_list.m_command_list->Reset(m_commandAllocator[m_active_cl].Get(), pipeline_state));

	// set gpu heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_dynamic_gpu_heaps[m_active_cl].GetVisibleHeap().Get() };
    m_command_list.m_command_list->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    m_dynamic_gpu_heaps[m_active_cl].Reset();

    return m_command_list;
}

CommandList& GfxCommandQueue::GetActiveCL(){
    return m_command_list;
}
void GfxCommandQueue::ExecuteActiveCL(){
    ThrowIfFailed(m_command_list.m_command_list->Close());
    ID3D12CommandList* ppCommandLists[] = { m_command_list.m_command_list.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

void GfxCommandQueue::ResourceBarrier(std::shared_ptr<GpuResource> &res, uint32_t to){
    if (std::shared_ptr<HeapBuffer> buff = res->GetBuffer().lock()){
        D3D12_RESOURCE_STATES calculated_from = (D3D12_RESOURCE_STATES)res->GetState();
        D3D12_RESOURCE_STATES to_native = (D3D12_RESOURCE_STATES)to;
        if (calculated_from != to_native) {
            m_command_list.m_command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buff->GetResource().Get(), calculated_from, to_native));
            res->UpdateState((ResourceState)to);
        }
    }
}

void GfxCommandQueue::ResourceBarrier(GpuResource &res, uint32_t to) {
    if (std::shared_ptr<HeapBuffer> buff = res.GetBuffer().lock()){
        D3D12_RESOURCE_STATES calculated_from = (D3D12_RESOURCE_STATES)res.GetState();
        D3D12_RESOURCE_STATES to_native = (D3D12_RESOURCE_STATES)to;
        if (calculated_from != to_native) {
            m_command_list.m_command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buff->GetResource().Get(), calculated_from, to_native));
            res.UpdateState((ResourceState)to);
        }
    }
}

void GfxCommandQueue::ResourceBarrier(std::vector<std::shared_ptr<GpuResource>>& res, uint32_t to) {
    std::vector<CD3DX12_RESOURCE_BARRIER> resources;
    resources.reserve(res.size());
    for (auto gpu_res : res){
        if (std::shared_ptr<HeapBuffer> buff = gpu_res->GetBuffer().lock()){
            D3D12_RESOURCE_STATES calculated_from = (D3D12_RESOURCE_STATES)gpu_res->GetState();
            D3D12_RESOURCE_STATES to_native = (D3D12_RESOURCE_STATES)to;
            if (calculated_from != to_native) {
                resources.push_back(CD3DX12_RESOURCE_BARRIER::Transition(buff->GetResource().Get(), calculated_from, to_native));
                gpu_res->UpdateState((ResourceState)to);
            }
        }
    }

    m_command_list.m_command_list->ResourceBarrier((uint32_t)resources.size(), resources.data());
}

void GfxCommandQueue::SetPSO(uint32_t id) {
    m_pso = id;
    auto tech = gD3DApp->GetTechniqueById(id);

    if (m_type == D3D12_COMMAND_LIST_TYPE_DIRECT) {
        m_command_list.m_command_list->SetPipelineState(tech->pipeline_state.Get());
    }
    else if (m_type == D3D12_COMMAND_LIST_TYPE_COMPUTE) {
        m_command_list.m_command_list->SetPipelineState(tech->pipeline_state.Get());
    }
}

void GfxCommandQueue::SetRootSign(uint32_t id, bool gfx) {
    m_root_sign = id;

    auto root_sign = gD3DApp->GetRootSignById(id);
    auto &r = root_sign->GetRootSignature();
    if (m_type == D3D12_COMMAND_LIST_TYPE_DIRECT && gfx) {
        m_command_list.m_command_list->SetGraphicsRootSignature(r.Get());
    }
    else {
        m_command_list.m_command_list->SetComputeRootSignature(r.Get());
    }
}