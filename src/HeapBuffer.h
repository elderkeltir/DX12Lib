#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <directx/d3dx12.h>
#include <wrl.h>
#include "defines.h"
using Microsoft::WRL::ComPtr;

class CommandList;
struct ClearColor;
struct ResourceDesc;

class HeapBuffer{
public:
    enum class BufferType { bt_default, bt_upload, bt_readback };
    enum class UseFlag { uf_none, uf_rt, uf_ds, uf_srv, uf_uav };
public:
    void Create(HeapType type, uint32_t bufferSize, ResourceState initial_state, std::optional<std::wstring> dbg_name = std::nullopt);
    void CreateTexture(HeapType type, const ResourceDesc& res_desc, ResourceState initial_state, const ClearColor* clear_val, std::optional<std::wstring> dbg_name = std::nullopt);
    
    void Load(CommandList& command_list, uint32_t numElements, uint32_t elementSize, const void* bufferData);
    void Load(CommandList& command_list, uint32_t firstSubresource, uint32_t numSubresources, SubresourceData* subresourceData);
    
    BYTE* Map();
    void Unmap();
    void* GetCpuData() { return m_cpu_data; }
    void Copy(ComPtr<ID3D12GraphicsCommandList> &command_list, HeapBuffer &dest, uint64_t dstOffset, uint64_t srcOffset, uint64_t size);

    void Set(ComPtr<ID3D12Resource> resourse) { m_resourse = resourse; }

    ComPtr<ID3D12Resource> &GetResource() { return m_resourse; }
private:
    ComPtr<ID3D12Resource> m_resourse;
    ComPtr<ID3D12Resource> pIntermediateResource;
    void* m_cpu_data{nullptr};
    bool m_recreate_intermediate_res{false};
};