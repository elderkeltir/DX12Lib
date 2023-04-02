#pragma once

#include <DirectXMath.h>
#include <vector>
#include <string>
#include <memory>
#include "ITextureLoader.h"
#include "RenderMesh.h"
#include "defines.h"

class IGpuResource;
class ICommandList;

class RenderObject {
public:
    enum TextureType { DiffuseTexture = 0, NormalTexture, MetallicTexture, RoughTexture, TextureCount };
public:
    virtual ~RenderObject();
    virtual void SetId(uint32_t id) { m_id = id; }
    virtual uint32_t GetId() const { return m_id; }
    virtual void SetName(const std::wstring &name) { m_name = name; }
    virtual const std::wstring& GetName() const { return m_name; }
    virtual void Initialized() { m_is_initialized = true; m_dirty |= db_vertex; m_dirty |= db_index; }
    virtual bool IsInitialized() const { return m_is_initialized; }
    virtual void LoadDataToGpu(ICommandList* command_list) { };
    virtual void SetMesh(RenderMesh * mesh) { m_mesh = mesh; }
    virtual void SetTexture(ITextureLoader::TextureData * texture_data, TextureType type) {};

protected:
    virtual void LoadIndexDataOnGpu(ICommandList* command_list);
    void AllocateVertexBuffer(uint32_t size);

    enum dirty_bits {
        db_vertex       = 1 << 0,
        db_index        = 1 << 1,
        db_diffuse_tx   = 1 << 2,
        db_normals_tx   = 1 << 3,
        db_metallic_tx  = 1 << 4,
        db_rough_tx     = 1 << 5,
        db_rt_tx        = 1 << 6,
        db_rt_cbv       = 1 << 7
    };

    RenderMesh* m_mesh {nullptr};

    uint64_t m_vertex_buffer_start{0};
    uint32_t m_vertex_buffer_size{0};

    std::unique_ptr<IGpuResource> m_VertexBuffer;
    std::unique_ptr<IGpuResource> m_IndexBuffer;
    std::unique_ptr<IGpuResource> m_diffuse_tex;

    std::wstring m_name;
    uint32_t m_id{uint32_t(-1)};
    bool m_is_initialized{false};
    uint8_t m_dirty{0};
private:
    void DeallocateVertexBuffer();
};