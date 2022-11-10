#pragma once

#include <directx/d3dx12.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <vector>
#include <string>
#include <memory>
#include "TextureData.h"

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
    virtual void Initialized() { m_is_initialized = true; }
    virtual bool IsInitialized() const { return m_is_initialized; }
    virtual void SetVertices(std::vector<DirectX::XMFLOAT3> vertices) {
        m_vertices.swap(vertices);
        m_dirty |= db_vertex;
    }

    virtual void SetIndices(std::vector<uint16_t> indices) {
        m_dirty |= db_index;
        m_indices.swap(indices);
    }

    virtual void SetTextureCoords(std::vector<DirectX::XMFLOAT2> textCoords) {
        m_textCoords.swap(textCoords);
    }

    virtual void LoadDataToGpu(ComPtr<ID3D12GraphicsCommandList6> &commandList) = 0;

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

    std::vector<DirectX::XMFLOAT3> m_vertices;
    std::vector<uint16_t> m_indices;
    std::vector<DirectX::XMFLOAT2> m_textCoords;

    std::unique_ptr<GpuResource> m_VertexBuffer;
    std::unique_ptr<GpuResource> m_IndexBuffer;
    std::unique_ptr<GpuResource> m_diffuse_tex;

    uint64_t m_vertex_buffer_start{0};
    uint32_t m_vertex_buffer_size{0};

    std::wstring m_name;
    uint32_t m_id{uint32_t(-1)};
    bool m_is_initialized{false};
    uint8_t m_dirty{0};
private:
        void DeallocateVertexBuffer();
};