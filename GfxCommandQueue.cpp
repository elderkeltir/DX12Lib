#include "GfxCommandQueue.h"
#include "DXHelper.h"
#include <directx/d3dx12.h>


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
    ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[m_active_cl].Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
    SetName(m_commandList, dbg_name.value_or(L"").append(L"_cmd_list").c_str());
    ThrowIfFailed(m_commandList->Close());

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
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator[m_active_cl].Get(), pipeline_state));

    return m_commandList;
}

ComPtr<ID3D12GraphicsCommandList6>& GfxCommandQueue::GetActiveCL(){
    return m_commandList;
}
void GfxCommandQueue::ExecuteActiveCL(){
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

void GfxCommandQueue::ResourceBarrier(ComPtr<ID3D12Resource> &res, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to){
    TODO("Normal! Implement feasible way to store/get curent state of resource. later")
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(res.Get(), from, to));

}