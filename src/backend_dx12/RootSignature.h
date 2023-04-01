#pragma once

#include "IRootSignature.h"
#include <vector>
#include <directx/d3dx12.h> // TODO: pls
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class RootSignature : public IRootSignature {
public:
    uint32_t GetRSId() const override;
    void SetRSId(uint32_t id) override;

    ComPtr<ID3D12RootSignature>& GetRootSignature() { return m_root_signature; }
    const ComPtr<ID3D12RootSignature>& GetRootSignature() const { return m_root_signature; }
    std::vector<CD3DX12_ROOT_PARAMETER1>& GetRootParams() { return m_root_parameters; }
    const std::vector<CD3DX12_ROOT_PARAMETER1>& GetRootParams() const { return m_root_parameters; }

private:
    uint32_t m_id{ uint32_t(-1) };
    ComPtr<ID3D12RootSignature> m_root_signature;
    std::vector<CD3DX12_ROOT_PARAMETER1> m_root_parameters; // TODO: re-create structure in user-defined types!
};