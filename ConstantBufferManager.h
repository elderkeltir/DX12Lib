#pragma once

#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

enum Constants {
    cM,             // model matrix
    cV,             // view matrix
    cP              // projection matrix
};

class ConstantBufferManager {
public:
    void SetMatrix4Constant(Constants id, const DirectX::XMMATRIX & matrix, ComPtr<ID3D12GraphicsCommandList6> &commandList);
    void SetMatrix4Constant(Constants id, const DirectX::XMFLOAT4X4 & matrix, ComPtr<ID3D12GraphicsCommandList6> &commandList);
    void SetVector4Constant(Constants id, const DirectX::XMVECTOR & vec, ComPtr<ID3D12GraphicsCommandList6> &commandList);
    void SetVector4Constant(Constants id, const DirectX::XMFLOAT4 & vec, ComPtr<ID3D12GraphicsCommandList6> &commandList);
};