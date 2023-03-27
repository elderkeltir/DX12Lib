#pragma once

#include <memory>
#include <optional>
#include <wrl.h>
#include "defines.h"
using Microsoft::WRL::ComPtr;

#include <directx/d3dx12.h> // TODO: remove later

class ResourceDescriptor; // TODO: swap to interfaces later
class CommandList;
class HeapBuffer;



struct DSVdesc {
    ResourceFormat format;
    enum class DSVdimensionType {
        dsv_dt_unknown = 0,
        dsv_dt_texture1d = 1,
        dsv_dt_texture1darray = 2,
        dsv_dt_texture2d = 3,
        dsv_dt_texture2darray = 4,
        dsv_dt_texture2dms = 5,
        dsv_dt_texture2dmsarray = 6
    } dimension;
    enum class Flags {
        f_none = 0,
        f_read_only_depth = 0x1,
        f_read_only_stencil = 0x2
    };
    Flags flags{ Flags::f_none };
};

class IGpuResource {
public:
    struct IndexVufferView {
        uint64_t buffer_location;
        uint32_t size_in_bytes;
        ResourceFormat format;
    };


    
    
    virtual ~IGpuResource() {}
    virtual void CreateBuffer(HeapType type, uint32_t bufferSize, ResourceState initial_state, std::optional<std::wstring> dbg_name) = 0;
    virtual void CreateTexture(HeapType type, const ResourceDesc& res_desc, ResourceState initial_state, const ClearColor* clear_val, std::optional<std::wstring> dbg_name) = 0;
    virtual void SetBuffer(ComPtr<ID3D12Resource> res) = 0; // TODO: remove later
    virtual void LoadBuffer(CommandList& command_list, uint32_t numElements, uint32_t elementSize, const void* bufferData) = 0;
    virtual void LoadBuffer(CommandList& command_list, uint32_t firstSubresource, uint32_t numSubresources, SubresourceData* subresourceData) = 0;

    virtual void CreateRTV() = 0;
    virtual void Create_DSV(const DSVdesc& desc) = 0;
    virtual void Create_SRV(const SRVdesc& desc) = 0;
    virtual void Create_UAV(const UAVdesc& desc) = 0;
    virtual void Create_CBV(const CBVdesc& desc) = 0;
    virtual void Create_Index_View(ResourceFormat format, uint32_t SizeInBytes) = 0;

    virtual std::weak_ptr<HeapBuffer> GetBuffer() = 0;
    virtual std::weak_ptr<ResourceDescriptor> GetRTV() = 0;
    virtual std::weak_ptr<ResourceDescriptor> GetDSV() = 0;
    virtual std::weak_ptr<ResourceDescriptor> GetSRV() = 0;
    virtual std::weak_ptr<ResourceDescriptor> GetUAV() = 0;
    virtual std::weak_ptr<ResourceDescriptor> GetCBV() = 0;
    virtual std::weak_ptr<IndexVufferView> Get_Index_View() = 0;
protected:
    friend class GfxCommandQueue;
    virtual ResourceState GetState() const = 0;
    virtual void UpdateState(ResourceState new_state) = 0;

};