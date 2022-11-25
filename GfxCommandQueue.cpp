#include "GfxCommandQueue.h"
#include "DXHelper.h"
#include "GpuResource.h"
#include <directx/d3dx12.h>
#include "RootSignature.h"

#include "DXAppImplementation.h"

extern DXAppImplementation *gD3DApp;


void GfxCommandQueue::OnInit(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type, std::optional<std::wstring> dbg_name){
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = type;
    ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
    SetName(m_commandQueue, dbg_name.value_or(L"").append(L"_cmd_queue").c_str());

    for (uint32_t i = 0; i < CommandListsCount; i++){
        ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator[i])));
        SetName(m_commandAllocator[i], dbg_name.value_or(L"").append(L"_cmd_allocator_").append(std::to_wstring(i)).c_str());
    }
    ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[m_active_cl].Get(), nullptr, IID_PPV_ARGS(&m_command_list)));
    SetName(m_command_list, dbg_name.value_or(L"").append(L"_cmd_list").c_str());
    ThrowIfFailed(m_command_list->Close());

    {
        ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        SetName(m_fence, dbg_name.value_or(L"").append(L"_fence").c_str());
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }
    }
}

ComPtr<ID3D12GraphicsCommandList6> & GfxCommandQueue::ResetActiveCL(ID3D12PipelineState *pipeline_state) {
    m_active_cl = ++m_active_cl % CommandListsCount;
    ThrowIfFailed(m_commandAllocator[m_active_cl]->Reset());
    ThrowIfFailed(m_command_list->Reset(m_commandAllocator[m_active_cl].Get(), pipeline_state));

    return m_command_list;
}

ComPtr<ID3D12GraphicsCommandList6>& GfxCommandQueue::GetActiveCL(){
    return m_command_list;
}
void GfxCommandQueue::ExecuteActiveCL(){
    ThrowIfFailed(m_command_list->Close());
    ID3D12CommandList* ppCommandLists[] = { m_command_list.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

void GfxCommandQueue::ResourceBarrier(std::shared_ptr<GpuResource> &res, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to){
    TODO("Normal! Implement feasible way to store/get curent state of resource. later")
    if (std::shared_ptr<HeapBuffer> buff = res->GetBuffer().lock()){
        m_command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buff->GetResource().Get(), from, to));
    }
}

void GfxCommandQueue::ResourceBarrier(GpuResource &res, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to) {
    if (std::shared_ptr<HeapBuffer> buff = res.GetBuffer().lock()){
        m_command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buff->GetResource().Get(), from, to));
    }
}

void GfxCommandQueue::ResourceBarrier(std::vector<std::shared_ptr<GpuResource>>& res, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to) {
    std::vector<CD3DX12_RESOURCE_BARRIER> resources;
    resources.reserve(res.size());
    for (auto gpu_res : res){
        if (std::shared_ptr<HeapBuffer> buff = gpu_res->GetBuffer().lock()){
            resources.push_back(CD3DX12_RESOURCE_BARRIER::Transition(buff->GetResource().Get(), from, to));
        }
    }

    m_command_list->ResourceBarrier((uint32_t)resources.size(), resources.data());
}

void GfxCommandQueue::SetPSO(uint32_t id) {
    m_pso = id;
    auto tech = gD3DApp->GetTechniqueById(id);
    m_command_list->SetPipelineState(tech->pipeline_state.Get());
}

void GfxCommandQueue::SetRootSign(uint32_t id) {
    m_root_sign = id;

    auto root_sign = gD3DApp->GetRootSignById(id);
    auto &r = root_sign->GetRootSignature();
    m_command_list->SetGraphicsRootSignature(r.Get());
}