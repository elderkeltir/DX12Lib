#include "CommandQueue.h"
#include "dx12_helper.h"
#include "DynamicGpuHeap.h"
#include <directx/d3dx12.h>
#include "RootSignature.h"
#include "Fence.h"
#include "CommandList.h"
#include "DxBackend.h"
#include "DxDevice.h"

extern DxBackend* gBackend;

#define GetFence(fence) ((Fence*)fence.get())->GetFence()
#define GetNativeCmdList() ((CommandList*)m_command_list.get())->m_command_list

void CommandQueue::Flush()
{
    const uint32_t fence_value = Signal();
    WaitOnCPU(fence_value);
}

uint32_t CommandQueue::Signal(){
    uint32_t fenceValueForSignal = ++m_fence_value;
    ThrowIfFailed(m_commandQueue->Signal(GetFence(m_fence).Get(), fenceValueForSignal));

    return fenceValueForSignal;
}

void CommandQueue::Signal(std::unique_ptr<IFence> &fence, uint32_t fence_value)
{
    m_commandQueue->Signal(GetFence(fence).Get(), fence_value);
}

void CommandQueue::WaitOnCPU(uint32_t fence_value){
    if (GetFence(m_fence).Get()->GetCompletedValue() < fence_value) {
        ThrowIfFailed(GetFence(m_fence).Get()->SetEventOnCompletion(fence_value, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

void CommandQueue::WaitOnGPU(std::unique_ptr<IFence> &fence, uint32_t fence_value)
{
	if (GetFence(m_fence).Get()->GetCompletedValue() < fence_value) {
        ThrowIfFailed(m_commandQueue->Wait(GetFence(fence).Get(), fence_value));
	}
}

void CommandQueue::OnInit(QueueType type, uint32_t command_list_num, std::optional<std::wstring> dbg_name) {
    CommandList* cmd_list = new CommandList;

    m_command_list_num = command_list_num;
    m_type = type;
    cmd_list->m_queue = this;
    cmd_list->m_type = (type == QueueType::qt_gfx ? CommandListType::clt_direct : CommandListType::clt_compute);

    // allocate
    for (uint32_t i = 0; i < 6; i++) {
        m_dynamic_gpu_heaps[i].reset(new DynamicGpuHeap);
    }

    m_commandAllocator.swap(std::make_unique<ComPtr<ID3D12CommandAllocator>[]>(m_command_list_num));

    // commandList type
    D3D12_COMMAND_LIST_TYPE cmd_list_type = (type == QueueType::qt_gfx ? D3D12_COMMAND_LIST_TYPE_DIRECT : D3D12_COMMAND_LIST_TYPE_COMPUTE);

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = cmd_list_type;
    ComPtr<ID3D12Device2>& device = gBackend->GetDevice()->GetNativeObject();
    ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
    SetName(m_commandQueue, dbg_name.value_or(L"").append(L"_direct_queue_" + std::to_wstring((uint32_t)type)).c_str());

    for (uint32_t i = 0; i < m_command_list_num; i++) {
        ThrowIfFailed(device->CreateCommandAllocator(cmd_list_type, IID_PPV_ARGS(&m_commandAllocator[i])));
        SetName(m_commandAllocator[i], dbg_name.value_or(L"").append(L"_cmd_allocator_" + std::to_wstring((uint32_t)type)).append(std::to_wstring(i)).c_str());
    }
    ThrowIfFailed(device->CreateCommandList(0, cmd_list_type, m_commandAllocator[m_active_cl].Get(), nullptr, IID_PPV_ARGS(&cmd_list->m_command_list)));
    SetName(cmd_list->m_command_list, dbg_name.value_or(L"").append(L"_cmd_list_" + std::to_wstring((uint32_t)type)).c_str());
    ThrowIfFailed(cmd_list->m_command_list->Close());

    m_fence.reset(new Fence);
    m_fence->Initialize(m_fence_value);
    SetName(GetFence(m_fence), dbg_name.value_or(L"").append(L"_fence").c_str());
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }

    for (uint32_t id = 0; id < m_command_list_num; id++) {
        m_dynamic_gpu_heaps[id]->Initialize(id);
    }

    m_command_list.reset(cmd_list);
}

ICommandList* CommandQueue::ResetActiveCL() {
    m_active_cl = ++m_active_cl % m_command_list_num;

    ThrowIfFailed(m_commandAllocator[m_active_cl]->Reset());
    ThrowIfFailed(GetNativeCmdList()->Reset(m_commandAllocator[m_active_cl].Get(), nullptr));

    // set gpu heap
    m_command_list->SetDescriptorHeap(m_dynamic_gpu_heaps[m_active_cl].get());
    m_dynamic_gpu_heaps[m_active_cl]->Reset();
    m_command_list->Reset();

    return m_command_list.get();
}

ICommandList* CommandQueue::GetActiveCL() {
    return m_command_list.get();
}
void CommandQueue::ExecuteActiveCL() {
    ThrowIfFailed(GetNativeCmdList()->Close());
    ID3D12CommandList* ppCommandLists[] = { GetNativeCmdList().Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

IDynamicGpuHeap& CommandQueue::GetGpuHeap()
{
    return *m_dynamic_gpu_heaps[m_active_cl];
}

CommandQueue::~CommandQueue()
{

}

