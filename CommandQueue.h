#pragma once

#include <directx/d3d12.h>
#include <optional>
#include <string>
#include <cstdint>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class CommandQueue {
    friend class DXAppImplementation;
public:
    virtual void OnInit(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type, std::optional<std::wstring> dbg_name = std::nullopt) = 0;
    virtual void OnDestroy() { Flush(); CloseHandle(m_fenceEvent); }
    uint64_t Signal();
    void Wait(uint64_t fence_value);
    void Flush();
    virtual ~CommandQueue() = default;
protected:
    uint64_t m_fence_value{0};
    HANDLE m_fenceEvent;
    uint32_t m_active_cl{0};
    ComPtr<ID3D12Fence> m_fence;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
};