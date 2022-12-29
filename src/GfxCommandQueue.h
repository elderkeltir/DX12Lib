#pragma once

#include <memory>
#include <vector>
#include "CommandQueue.h"
#include "DynamicGpuHeap.h"
#include "CommandList.h"

class GpuResource;

class GfxCommandQueue : public CommandQueue {
public:
    virtual void OnInit(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type, uint32_t command_list_num, std::optional<std::wstring> dbg_name = std::nullopt) override;
    CommandList& ResetActiveCL(ID3D12PipelineState *pipeline_state = nullptr);
    virtual void SetPSO(uint32_t id) override;
    virtual void SetRootSign(uint32_t id) override;
    CommandList& GetActiveCL();
    void ExecuteActiveCL();
    void ResourceBarrier(std::shared_ptr<GpuResource> &res, D3D12_RESOURCE_STATES to);
    void ResourceBarrier(GpuResource &res, D3D12_RESOURCE_STATES to);
    void ResourceBarrier(std::vector<std::shared_ptr<GpuResource>>& res, D3D12_RESOURCE_STATES to);
    DynamicGpuHeap& GetGpuHeap() { return m_dynamic_gpu_heaps[m_active_cl]; }
protected:
    uint32_t m_command_list_num{ 0 };
    std::unique_ptr<DynamicGpuHeap[]> m_dynamic_gpu_heaps;
    std::unique_ptr<ComPtr<ID3D12CommandAllocator>[]> m_commandAllocator;
    CommandList m_command_list;
};