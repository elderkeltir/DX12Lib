#pragma once

#include <cstdint>
#include <array>
#include "ICommandList.h"
#include "ICommandQueue.h"

#include <wrl.h>
using Microsoft::WRL::ComPtr;
struct ID3D12CommandAllocator;
struct ID3D12CommandQueue;

class IDynamicGpuHeap;
class IFence;

class CommandQueue : public ICommandQueue {
    friend class DXAppImplementation;
public:
    void OnInit(QueueType type, uint32_t command_list_num, std::optional<std::wstring> dbg_name = std::nullopt) override;
    void OnDestroy() override { Flush(); CloseHandle(m_fenceEvent); }
    uint32_t Signal() override;
    void Signal(std::unique_ptr<IFence>& fence, uint32_t fence_value) override;
    void WaitOnCPU(uint32_t fence_value) override;
    void WaitOnGPU(std::unique_ptr<IFence>& fence, uint32_t fence_value) override;

    // not used
    void Signal(IFence* fence, bool on_cpu) override {}
    void WaitOnCPU(IFence* fence) override {}
    void WaitOnGPU(IFence* fence) override {}
    //
    void Flush() override;

    ICommandList* ResetActiveCL() override;
    ICommandList* GetActiveCL() override;
    void ExecuteActiveCL() override;

    IDynamicGpuHeap& GetGpuHeap() override;

    ~CommandQueue() override;

    ComPtr<ID3D12CommandQueue>& GetNativeObject() {
        return m_commandQueue;
    }
protected:
    uint32_t m_fence_value{0};
    HANDLE m_fenceEvent;
    uint32_t m_active_cl{0};
    std::unique_ptr<IFence> m_fence;
    ComPtr<ID3D12CommandQueue> m_commandQueue;

    QueueType m_type;

    uint32_t m_command_list_num{ 0 };
    std::array<std::unique_ptr<IDynamicGpuHeap>, 6> m_dynamic_gpu_heaps;
    std::unique_ptr<ComPtr<ID3D12CommandAllocator>[]> m_commandAllocator;
    std::unique_ptr<ICommandList> m_command_list;
};