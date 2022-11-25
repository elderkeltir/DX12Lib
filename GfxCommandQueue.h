#pragma once


#include "CommandQueue.h"

#include <memory>
#include <vector>

class GpuResource;

class GfxCommandQueue : public CommandQueue {
public:
    virtual void OnInit(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type, std::optional<std::wstring> dbg_name = std::nullopt) override;
    ComPtr<ID3D12GraphicsCommandList6>& ResetActiveCL(ID3D12PipelineState *pipeline_state = nullptr);
    virtual void SetPSO(uint32_t id) override;
    virtual void SetRootSign(uint32_t id) override;
    ComPtr<ID3D12GraphicsCommandList6>& GetActiveCL();
    void ExecuteActiveCL();
    void ResourceBarrier(std::shared_ptr<GpuResource> &res, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);
    void ResourceBarrier(GpuResource &res, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);
    void ResourceBarrier(std::vector<std::shared_ptr<GpuResource>>& res, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);
private:
    static constexpr uint32_t CommandListsCount = 2;
    ComPtr<ID3D12CommandAllocator> m_commandAllocator[CommandListsCount];
    ComPtr<ID3D12GraphicsCommandList6> m_command_list;
};