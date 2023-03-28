#include "CommandQueue.h"
#include "DXHelper.h"
#include "GpuResource.h"
#include <directx/d3dx12.h>
#include "RootSignature.h"
#include "DXAppImplementation.h"

extern DXAppImplementation* gD3DApp;

void CommandQueue::Flush()
{
    const uint64_t fence_value = Signal();
    WaitOnCPU(fence_value);
}

uint64_t CommandQueue::Signal(){
    uint64_t fenceValueForSignal = ++m_fence_value;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fenceValueForSignal));

    return fenceValueForSignal;
}

void CommandQueue::Signal(ComPtr<ID3D12Fence>& fence, uint64_t fence_value)
{
    m_commandQueue->Signal(fence.Get(), fence_value);
}

void CommandQueue::WaitOnCPU(uint64_t fence_value){
    if (m_fence->GetCompletedValue() < fence_value) {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fence_value, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

void CommandQueue::WaitOnGPU(ComPtr<ID3D12Fence> &fence, uint64_t fence_value)
{
	if (m_fence->GetCompletedValue() < fence_value) {
        ThrowIfFailed(m_commandQueue->Wait(fence.Get(), fence_value));
	}
}

void CommandQueue::OnInit(ComPtr<ID3D12Device2> device, QueueType type, uint32_t command_list_num, std::optional<std::wstring> dbg_name) {
    m_command_list_num = command_list_num;
    m_type = type;
    m_command_list.m_queue = this;
    m_command_list.m_type = (type == QueueType::qt_gfx ? CommandListType::clt_direct : CommandListType::clt_compute);

    // allocate
    m_dynamic_gpu_heaps.swap(std::make_unique<DynamicGpuHeap[]>(m_command_list_num));
    m_commandAllocator.swap(std::make_unique<ComPtr<ID3D12CommandAllocator>[]>(m_command_list_num));

    // commandList type
    D3D12_COMMAND_LIST_TYPE cmd_list_type = (type == QueueType::qt_gfx ? D3D12_COMMAND_LIST_TYPE_DIRECT : D3D12_COMMAND_LIST_TYPE_COMPUTE);

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = cmd_list_type;
    ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
    SetName(m_commandQueue, dbg_name.value_or(L"").append(L"_direct_queue_" + std::to_wstring((uint32_t)type)).c_str());

    for (uint32_t i = 0; i < m_command_list_num; i++) {
        ThrowIfFailed(device->CreateCommandAllocator(cmd_list_type, IID_PPV_ARGS(&m_commandAllocator[i])));
        SetName(m_commandAllocator[i], dbg_name.value_or(L"").append(L"_cmd_allocator_" + std::to_wstring((uint32_t)type)).append(std::to_wstring(i)).c_str());
    }
    ThrowIfFailed(device->CreateCommandList(0, cmd_list_type, m_commandAllocator[m_active_cl].Get(), nullptr, IID_PPV_ARGS(&m_command_list.m_command_list)));
    SetName(m_command_list.m_command_list, dbg_name.value_or(L"").append(L"_cmd_list_" + std::to_wstring((uint32_t)type)).c_str());
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

CommandList& CommandQueue::ResetActiveCL(ID3D12PipelineState* pipeline_state) {
    m_active_cl = ++m_active_cl % m_command_list_num;

    ThrowIfFailed(m_commandAllocator[m_active_cl]->Reset());
    ThrowIfFailed(m_command_list.m_command_list->Reset(m_commandAllocator[m_active_cl].Get(), pipeline_state));

    // set gpu heap
    ID3D12DescriptorHeap* descriptorHeaps[] = { m_dynamic_gpu_heaps[m_active_cl].GetVisibleHeap().Get() };
    m_command_list.m_command_list->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    m_dynamic_gpu_heaps[m_active_cl].Reset();
    m_command_list.Reset();

    return m_command_list;
}

CommandList& CommandQueue::GetActiveCL() {
    return m_command_list;
}
void CommandQueue::ExecuteActiveCL() {
    ThrowIfFailed(m_command_list.m_command_list->Close());
    ID3D12CommandList* ppCommandLists[] = { m_command_list.m_command_list.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

