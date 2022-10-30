#include "ConstantBufferManager.h"

// #include "DXAppImplementation.h"
// #include "DXHelper.h"

// extern DXAppImplementation *gD3DApp;

static const uint32_t constansIds[] = {
    0,                  // Constants::cM
    1,                  // Constants::cV
    2,                  // Constants::cP
};

void ConstantBufferManager::SetMatrix4Constant(Constants id, const DirectX::XMMATRIX & matrix, ComPtr<ID3D12GraphicsCommandList6> &commandList){
    commandList->SetGraphicsRoot32BitConstants(constansIds[id], sizeof(DirectX::XMMATRIX) / 4, &matrix, 0);
}

void ConstantBufferManager::SetMatrix4Constant(Constants id, const DirectX::XMFLOAT4X4 & matrix, ComPtr<ID3D12GraphicsCommandList6> &commandList){
    commandList->SetGraphicsRoot32BitConstants(constansIds[id], sizeof(DirectX::XMFLOAT4X4) / 4, &matrix, 0);
}

void ConstantBufferManager::SetVector4Constant(Constants id, const DirectX::XMVECTOR & vec, ComPtr<ID3D12GraphicsCommandList6> &commandList){
    commandList->SetGraphicsRoot32BitConstants(constansIds[id], sizeof(DirectX::XMVECTOR) / 4, &vec, 0);
}

void ConstantBufferManager::SetVector4Constant(Constants id, const DirectX::XMFLOAT4 & vec, ComPtr<ID3D12GraphicsCommandList6> &commandList){
    commandList->SetGraphicsRoot32BitConstants(constansIds[id], sizeof(DirectX::XMFLOAT4) / 4, &vec, 0);
}