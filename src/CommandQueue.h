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
    virtual void OnInit(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type, uint32_t command_list_num, std::optional<std::wstring> dbg_name = std::nullopt) = 0;
    virtual void OnDestroy() { Flush(); CloseHandle(m_fenceEvent); }
    virtual void SetPSO(uint32_t id) = 0;
    virtual void SetRootSign(uint32_t id, bool gfx) = 0;
    uint64_t Signal();
    void Signal(ComPtr<ID3D12Fence>& fence, uint64_t fence_value);
    void WaitOnCPU(uint64_t fence_value);
    void WaitOnGPU(ComPtr<ID3D12Fence> &fence, uint64_t fence_value);
    void Flush();
    uint32_t GetPSO() const { return m_pso; }
    uint32_t GetRootSign() const { return m_root_sign; }

    virtual ~CommandQueue() = default;
protected:
    uint64_t m_fence_value{0};
    HANDLE m_fenceEvent;
    uint32_t m_active_cl{0};
    ComPtr<ID3D12Fence> m_fence;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    uint32_t m_pso{uint32_t(-1)};
    uint32_t m_root_sign{uint32_t(-1)};
    D3D12_COMMAND_LIST_TYPE m_type;
};