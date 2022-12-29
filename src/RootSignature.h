#pragma once

#include <vector>
#include <directx/d3dx12.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class RootSignature {
public:
    ComPtr<ID3D12RootSignature>& GetRootSignature() { return m_root_signature; }
    const ComPtr<ID3D12RootSignature>& GetRootSignature() const { return m_root_signature; }
    std::vector<CD3DX12_ROOT_PARAMETER1>& GetRootParams() { return m_root_parameters; }
    const std::vector<CD3DX12_ROOT_PARAMETER1>& GetRootParams() const { return m_root_parameters; }
    uint32_t id{uint32_t(-1)};

private:
    ComPtr<ID3D12RootSignature> m_root_signature;
    std::vector<CD3DX12_ROOT_PARAMETER1> m_root_parameters;
};