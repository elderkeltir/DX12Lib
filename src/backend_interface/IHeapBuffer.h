#pragma once

#include <optional>
#include <string>
#include "defines.h"

#include <wrl.h>
using Microsoft::WRL::ComPtr;
struct ID3D12Resource;

class ICommandList;

class IHeapBuffer {
public:
    virtual void Create(HeapType type, uint32_t bufferSize, ResourceState initial_state, std::optional<std::wstring> dbg_name = std::nullopt) = 0;
    virtual void CreateTexture(HeapType type, const ResourceDesc& res_desc, ResourceState initial_state, const ClearColor* clear_val, std::optional<std::wstring> dbg_name = std::nullopt) = 0;

    virtual void Load(ICommandList* command_list, uint32_t numElements, uint32_t elementSize, const void* bufferData) = 0;
    virtual void Load(ICommandList* command_list, uint32_t firstSubresource, uint32_t numSubresources, SubresourceData* subresourceData) = 0;

    virtual uint8_t* Map() = 0;
    virtual void Unmap() = 0;
    virtual void* GetCpuData() = 0;

    // TODO: get rid of dx12 depend
    virtual void Set(ComPtr<ID3D12Resource> resourse) = 0;
    virtual ComPtr<ID3D12Resource>& GetResource() = 0;
};