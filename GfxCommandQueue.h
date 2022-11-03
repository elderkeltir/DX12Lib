#pragma once


#include "CommandQueue.h"

class GfxCommandQueue : public CommandQueue {
public:
    virtual void OnInit(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type, std::optional<std::wstring> dbg_name = std::nullopt) override;
    ComPtr<ID3D12GraphicsCommandList6>& ResetActiveCL(ID3D12PipelineState *pipeline_state = nullptr);
    ComPtr<ID3D12GraphicsCommandList6>& GetActiveCL();
    void ExecuteActiveCL();
    void ResourceBarrier(ComPtr<ID3D12Resource> &res, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);
private:
    static constexpr uint32_t CommandListsCount = 2;
    ComPtr<ID3D12CommandAllocator> m_commandAllocator[CommandListsCount];
    ComPtr<ID3D12GraphicsCommandList6> m_commandList;
};