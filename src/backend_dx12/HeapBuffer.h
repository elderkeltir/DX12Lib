#pragma once

#include <cstdint>
#include "IHeapBuffer.h"
#include <wrl.h>
using Microsoft::WRL::ComPtr;

class ICommandList;
struct ID3D12Resource;
struct ClearColor;
struct ResourceDesc;

class HeapBuffer : public IHeapBuffer {
public:
    void Create(HeapType type, uint32_t bufferSize, ResourceState initial_state, std::optional<std::wstring> dbg_name = std::nullopt) override;
    void CreateTexture(HeapType type, const ResourceDesc& res_desc, ResourceState initial_state, const ClearColor* clear_val, std::optional<std::wstring> dbg_name = std::nullopt) override;
    
    void Load(ICommandList* command_list, uint32_t numElements, uint32_t elementSize, const void* bufferData) override;
    void Load(ICommandList* command_list, uint32_t firstSubresource, uint32_t numSubresources, SubresourceData* subresourceData) override;
    
    uint8_t* Map() override;
    void Unmap() override;
    void* GetCpuData() override { return m_cpu_data; }
    //void Copy(const ICommandList &command_list, HeapBuffer &dest, uint64_t dstOffset, uint64_t srcOffset, uint64_t size);

    void Set(ComPtr<ID3D12Resource> resourse) { m_resourse = resourse; }
    ComPtr<ID3D12Resource> &GetResource() { return m_resourse; }
private:
    ComPtr<ID3D12Resource> m_resourse;
    ComPtr<ID3D12Resource> pIntermediateResource;
    void* m_cpu_data{nullptr};
    bool m_recreate_intermediate_res{false};
};