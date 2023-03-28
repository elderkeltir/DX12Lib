#pragma once

#include <directx/d3d12.h>
#include <optional>
#include <string>
#include <cstdint>
#include <wrl.h>
#include "DynamicGpuHeap.h"
#include "CommandList.h"

using Microsoft::WRL::ComPtr;
class GpuResource;

class CommandQueue {
    friend class DXAppImplementation;
public:
    enum class QueueType {
        qt_gfx,
        qt_compute,
        qt_copy
    };

    void OnInit(ComPtr<ID3D12Device2> device, QueueType type, uint32_t command_list_num, std::optional<std::wstring> dbg_name = std::nullopt);
    void OnDestroy() { Flush(); CloseHandle(m_fenceEvent); }

    uint64_t Signal();
    void Signal(ComPtr<ID3D12Fence>& fence, uint64_t fence_value);
    void WaitOnCPU(uint64_t fence_value);
    void WaitOnGPU(ComPtr<ID3D12Fence> &fence, uint64_t fence_value);
    void Flush();

    CommandList& ResetActiveCL(ID3D12PipelineState* pipeline_state = nullptr);
    CommandList& GetActiveCL();
    void ExecuteActiveCL();

    DynamicGpuHeap& GetGpuHeap() { return m_dynamic_gpu_heaps[m_active_cl]; }

    ~CommandQueue() = default;
protected:
    uint64_t m_fence_value{0};
    HANDLE m_fenceEvent;
    uint32_t m_active_cl{0};
    ComPtr<ID3D12Fence> m_fence;
    ComPtr<ID3D12CommandQueue> m_commandQueue;

    QueueType m_type;

    uint32_t m_command_list_num{ 0 };
    std::unique_ptr<DynamicGpuHeap[]> m_dynamic_gpu_heaps;
    std::unique_ptr<ComPtr<ID3D12CommandAllocator>[]> m_commandAllocator;
    CommandList m_command_list;
};