#pragma once

#include <memory>
#include <optional>
#include <string>
#include "defines.h"

class IResourceDescriptor;
class ICommandList;
class IHeapBuffer;

struct IndexVufferView {
    std::shared_ptr<IHeapBuffer> buffer_location;
    uint32_t size_in_bytes;
    ResourceFormat format;
};

class IGpuResource {
public:
    virtual void CreateBuffer(HeapType type, uint32_t bufferSize, ResourceState initial_state, std::optional<std::wstring> dbg_name = std::nullopt) = 0;
    virtual void CreateTexture(HeapType type, const ResourceDesc& res_desc, ResourceState initial_state, const ClearColor* clear_val, std::optional<std::wstring> dbg_name = std::nullopt) = 0;
    virtual void LoadBuffer(ICommandList* command_list, uint32_t numElements, uint32_t elementSize, const void* bufferData) = 0;
    virtual void LoadBuffer(ICommandList* command_list, uint32_t firstSubresource, uint32_t numSubresources, SubresourceData* subresourceData) = 0;

    virtual void CreateRTV() = 0;
    virtual void Create_DSV(const DSVdesc& desc) = 0;
    virtual void Create_SRV(const SRVdesc& desc) = 0;
    virtual void Create_UAV(const UAVdesc& desc) = 0;
    virtual void Create_CBV(const CBVdesc& desc) = 0;
    virtual void Create_Index_View(ResourceFormat format, uint32_t SizeInBytes) = 0;

    virtual std::weak_ptr<IHeapBuffer> GetBuffer() = 0;
    virtual std::weak_ptr<IResourceDescriptor> GetRTV() = 0;
    virtual std::weak_ptr<IResourceDescriptor> GetDSV() = 0;
    virtual std::weak_ptr<IResourceDescriptor> GetSRV() = 0;
    virtual std::weak_ptr<IResourceDescriptor> GetUAV() = 0;
    virtual std::weak_ptr<IResourceDescriptor> GetCBV() = 0;
    virtual std::weak_ptr<IndexVufferView> Get_Index_View() = 0;

    virtual ResourceState GetState() const = 0;
    virtual void UpdateState(ResourceState new_state) = 0;
    virtual ~IGpuResource() = default;
};

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
    EXPORT IGpuResource* CreateGpuResourceDx();
    EXPORT IGpuResource* CreateGpuResourceVk();
    EXPORT IGpuResource* CreateGpuResource(BackendType bk_type);
}
