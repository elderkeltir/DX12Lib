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
    void OnInit(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type, uint32_t command_list_num, std::optional<std::wstring> dbg_name = std::nullopt);
    void OnDestroy() { Flush(); CloseHandle(m_fenceEvent); }
    void SetPSO(uint32_t id);
    void SetRootSign(uint32_t id, bool gfx = true);
    uint64_t Signal();
    void Signal(ComPtr<ID3D12Fence>& fence, uint64_t fence_value);
    void WaitOnCPU(uint64_t fence_value);
    void WaitOnGPU(ComPtr<ID3D12Fence> &fence, uint64_t fence_value);
    void Flush();
    uint32_t GetPSO() const { return m_pso; }
    uint32_t GetRootSign() const { return m_root_sign; }

    CommandList& GetActiveCL();
    void ExecuteActiveCL();
    void ResourceBarrier(std::shared_ptr<GpuResource>& res, uint32_t to);
    void ResourceBarrier(GpuResource& res, uint32_t to);
    void ResourceBarrier(std::vector<std::shared_ptr<GpuResource>>& res, uint32_t to);
    DynamicGpuHeap& GetGpuHeap() { return m_dynamic_gpu_heaps[m_active_cl]; }
    CommandList& ResetActiveCL(ID3D12PipelineState* pipeline_state = nullptr);

    ~CommandQueue() = default;
protected:
    uint64_t m_fence_value{0};
    HANDLE m_fenceEvent;
    uint32_t m_active_cl{0};
    ComPtr<ID3D12Fence> m_fence;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    uint32_t m_pso{uint32_t(-1)};
    uint32_t m_root_sign{uint32_t(-1)};
    D3D12_COMMAND_LIST_TYPE m_type;

    uint32_t m_command_list_num{ 0 };
    std::unique_ptr<DynamicGpuHeap[]> m_dynamic_gpu_heaps;
    std::unique_ptr<ComPtr<ID3D12CommandAllocator>[]> m_commandAllocator;
    CommandList m_command_list;
};