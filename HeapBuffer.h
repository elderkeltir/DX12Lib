#pragma once

#include <cstdint>
#include <d3dx12.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

class HeapBuffer{
public:
    enum class BufferType { bt_deafult, bt_upload, bt_readback };
    enum class UseFlag { uf_rt, uf_ds, uf_srv, uf_uav };
public:
    void Create(BufferType type, uint32_t bufferSize, UseFlag flag, D3D12_RESOURCE_STATES initial_state);
    void CreateTexture(BufferType type, const CD3DX12_RESOURCE_DESC &res_desc, D3D12_RESOURCE_STATES initial_state, const D3D12_CLEAR_VALUE &clear_val);
    BYTE* Map();
    void Unmap();
    void Copy(ComPtr<ID3D12GraphicsCommandList> &commandList, HeapBuffer &dest, uint64_t dstOffset, uint64_t srcOffset, uint64_t size);

    void Set(ComPtr<ID3D12Resource> resourse) { m_resourse = resourse; }

    ComPtr<ID3D12Resource> &GetResource() { return m_resourse; }
private:
    ComPtr<ID3D12Resource> m_resourse;
};