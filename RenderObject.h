#pragma once

#include <directx/d3dx12.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <vector>
#include <string>
#include <memory>
#include "TextureData.h"
#include "RenderMesh.h"

using Microsoft::WRL::ComPtr;

class GpuResource;

class RenderObject {
public:
    enum TextureType { DiffuseTexture = 0, NormalTexture, SpecularTexture, TextureCount };
public:
    virtual ~RenderObject();
    virtual void SetId(uint32_t id) { m_id = id; }
    virtual uint32_t GetId() const { return m_id; }
    virtual void SetName(const std::wstring &name) { m_name = name; }
    virtual const std::wstring& GetName() const { return m_name; }
    virtual void Initialized() { m_is_initialized = true; m_dirty |= db_vertex; m_dirty |= db_index; }
    virtual bool IsInitialized() const { return m_is_initialized; }
    virtual void LoadDataToGpu(ComPtr<ID3D12GraphicsCommandList6> &commandList) = 0;
    virtual void SetMesh(RenderMesh * mesh) { m_mesh = mesh; }
    virtual void SetTexture(TextureData * texture_data, TextureType type) {};

protected:
    virtual void LoadVertexDataOnGpu(ComPtr<ID3D12GraphicsCommandList6> &commandList, const void* data, uint32_t size_of_vertex, uint32_t vertex_count);
    virtual void LoadIndexDataOnGpu(ComPtr<ID3D12GraphicsCommandList6> &commandList);
    void Loadtexture(ComPtr<ID3D12GraphicsCommandList6> & commandList, GpuResource* res, TextureData* tex_data, const CD3DX12_RESOURCE_DESC &tex_desc, const D3D12_SRV_DIMENSION &srv_dim, uint32_t idx) const;
    void AllocateVertexBuffer(uint32_t size);

    enum dirty_bits {
        db_vertex       = 1 << 0,
        db_index        = 1 << 1,
        db_diffuse_tx   = 1 << 2,
        db_normals_tx   = 1 << 3,
        db_specular_tx  = 1 << 4,
        db_rt_tx        = 1 << 5,
        db_rt_cbv       = 1<< 6
    };

    RenderMesh* m_mesh {nullptr};

    uint64_t m_vertex_buffer_start{0};
    uint32_t m_vertex_buffer_size{0};

    std::unique_ptr<GpuResource> m_VertexBuffer;
    std::unique_ptr<GpuResource> m_IndexBuffer;
    std::unique_ptr<GpuResource> m_diffuse_tex;

    std::wstring m_name;
    uint32_t m_id{uint32_t(-1)};
    bool m_is_initialized{false};
    uint8_t m_dirty{0};
private:
    void DeallocateVertexBuffer();
};