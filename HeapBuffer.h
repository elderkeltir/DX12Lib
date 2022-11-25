#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <directx/d3dx12.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

class HeapBuffer{
public:
    enum class BufferType { bt_default, bt_upload, bt_readback };
    enum class UseFlag { uf_none, uf_rt, uf_ds, uf_srv, uf_uav };
public:
    void Create(BufferType type, uint32_t bufferSize, UseFlag flag, D3D12_RESOURCE_STATES initial_state, std::optional<std::wstring> dbg_name = std::nullopt);
    void CreateTexture(BufferType type, const CD3DX12_RESOURCE_DESC &res_desc, D3D12_RESOURCE_STATES initial_state, const D3D12_CLEAR_VALUE *clear_val, std::optional<std::wstring> dbg_name = std::nullopt);
    
    void Load(ComPtr<ID3D12GraphicsCommandList6> &commandList, uint32_t numElements, uint32_t elementSize, const void* bufferData);
    void Load(ComPtr<ID3D12GraphicsCommandList6> &commandList, uint32_t firstSubresource, uint32_t numSubresources, D3D12_SUBRESOURCE_DATA* subresourceData);
    
    BYTE* Map();
    void Unmap();
    void* GetCpuData() { return m_cpu_data; }
    void Copy(ComPtr<ID3D12GraphicsCommandList> &commandList, HeapBuffer &dest, uint64_t dstOffset, uint64_t srcOffset, uint64_t size);

    void Set(ComPtr<ID3D12Resource> resourse) { m_resourse = resourse; }

    ComPtr<ID3D12Resource> &GetResource() { return m_resourse; }
private:
    ComPtr<ID3D12Resource> m_resourse;
    ComPtr<ID3D12Resource> pIntermediateResource;
    void* m_cpu_data{nullptr};
    bool m_recreate_intermediate_res{false};
};