#include "RenderObject.h"
#include "GpuResource.h"
#include "DXAppImplementation.h"
#include "GpuDataManager.h"
#include "GfxCommandQueue.h"

extern DXAppImplementation *gD3DApp;

RenderObject::~RenderObject() {
    DeallocateVertexBuffer();
}

void RenderObject::LoadIndexDataOnGpu(CommandList& command_list){
    if (m_dirty & db_index && m_mesh->GetIndicesNum()){
        m_IndexBuffer = std::make_unique<GpuResource>();
        m_IndexBuffer->CreateBuffer(HeapType::ht_default, (m_mesh->GetIndicesNum() * sizeof(uint16_t)), ResourceState::rs_resource_state_copy_dest, std::wstring(L"index_buffer").append(m_name));
        m_IndexBuffer->LoadBuffer(command_list, m_mesh->GetIndicesNum(), sizeof(uint16_t), m_mesh->GetIndicesData());
        command_list.GetQueue()->ResourceBarrier(*m_IndexBuffer, ResourceState::rs_resource_state_index_buffer);
        m_IndexBuffer->Create_Index_View(ResourceFormat::rf_r16_uint, (m_mesh->GetIndicesNum() * sizeof(uint16_t)));
        m_dirty &= (~db_index);
    }
}

void RenderObject::Loadtexture(CommandList& command_list, GpuResource* res, TextureData* tex_data, const ResourceDesc &tex_desc, const SRVdesc::SRVdimensionType &srv_dim, uint32_t idx) const {
    res->CreateTexture(HeapType::ht_default, tex_desc, ResourceState::rs_resource_state_copy_dest, nullptr, std::wstring(m_name).append(L"model_srv_").append(std::to_wstring(idx)).c_str());

    const uint32_t image_count = (uint32_t)tex_data->scratch_image.GetImageCount();
    std::vector<SubresourceData> subresources(image_count);
    const DirectX::Image* pImages = tex_data->scratch_image.GetImages();
    for (uint32_t i = 0; i < (uint32_t)image_count; ++i) {
        auto& subresource = subresources[i];
        subresource.row_pitch = pImages[i].rowPitch;
        subresource.slice_pitch = pImages[i].slicePitch;
        subresource.data = pImages[i].pixels;
    }
    res->LoadBuffer(command_list, 0, (uint32_t)subresources.size(), subresources.data());
    command_list.GetQueue()->ResourceBarrier(*res, ResourceState::rs_resource_state_pixel_shader_resource);

    SRVdesc srv_desc = {};

    srv_desc.format = (ResourceFormat)tex_data->meta_data.format; // TODO: remove when load text moved etc.
    srv_desc.dimension = srv_dim;
    if (srv_dim == SRVdesc::SRVdimensionType::srv_dt_texture2d){
        srv_desc.texture2d.most_detailed_mip = 0;
        srv_desc.texture2d.mip_levels = (UINT)tex_data->meta_data.mipLevels;
        srv_desc.texture2d.res_min_lod_clamp = 0.0f;
    }
    else if (srv_dim == SRVdesc::SRVdimensionType::srv_dt_texturecube) {
        srv_desc.texture_cube.most_detailed_mip = 0;
        srv_desc.texture_cube.mip_levels = (UINT)tex_data->meta_data.mipLevels;
        srv_desc.texture_cube.res_min_lod_clamp = 0.0f;
    }

    res->Create_SRV(srv_desc);
}

void RenderObject::AllocateVertexBuffer(uint32_t size) {
    if (std::shared_ptr<GpuDataManager> gpu_res_mgr = gD3DApp->GetGpuDataManager().lock()){
        m_vertex_buffer_start = gpu_res_mgr->AllocateVertexBuffer(size);
        m_vertex_buffer_size = size;
    }
}

void RenderObject::DeallocateVertexBuffer() {
    if (!gD3DApp) {
        return;
    }
    
    if (std::shared_ptr<GpuDataManager> gpu_res_mgr = gD3DApp->GetGpuDataManager().lock()){
        gpu_res_mgr->DeallocateVertexBuffer(m_vertex_buffer_start, m_vertex_buffer_size);
        m_vertex_buffer_start = 0ull;
        m_vertex_buffer_size = 0ul;
    }
}