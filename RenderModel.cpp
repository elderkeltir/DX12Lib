#include "RenderModel.h"
#include "Transformations.h"
#include "GpuResource.h"
#include "DXHelper.h"
#include "DXAppImplementation.h"
#include "ResourceDescriptor.h"
#include "VertexFormats.h"
#include "GfxCommandQueue.h"

extern DXAppImplementation *gD3DApp;

RenderModel::RenderModel() :
    m_transformations(std::make_unique<Transformations>())
{

}

RenderModel::~RenderModel() = default;

void RenderModel::Load(const std::wstring &name){

}

void RenderModel::Move(const DirectX::XMFLOAT3 &pos){
    m_transformations->Move(pos);
}

void RenderModel::Rotate(const DirectX::XMFLOAT3 &angles){
    m_transformations->Rotate(angles);
}

void RenderModel::Scale(const DirectX::XMFLOAT3 &scale){
    m_transformations->Scale(scale);
}

void RenderModel::FormVertexes(){
    const Techniques::Technique * tech = gD3DApp->GetTechniqueById(m_tech_id);

    if (tech->vertex_type == 0) {
        using Vertex = Vertex0;
        AllocateVertexBuffer(m_mesh->GetVerticesNum() * sizeof(Vertex));
        Vertex* vertex_data_buffer = (Vertex*) m_vertex_buffer_start;
        for (uint32_t i = 0; i < m_mesh->GetVerticesNum(); i++) {
            vertex_data_buffer[i] = Vertex{ m_mesh->GetVertex(i), m_mesh->GetNormal(i), m_color};
        }
    }
    else if (tech->vertex_type == 1) {
        using Vertex = Vertex1;
        AllocateVertexBuffer(m_mesh->GetVerticesNum() * sizeof(Vertex));
        Vertex* vertex_data_buffer = (Vertex*) m_vertex_buffer_start;
        for (uint32_t i = 0; i < m_mesh->GetVerticesNum(); i++) {
            vertex_data_buffer[i] = Vertex{m_mesh->GetVertex(i), m_mesh->GetNormal(i), m_mesh->GetTangent(i), m_mesh->GetBiTangent(i), m_mesh->GetTexCoord(i) };
        }
    }
    else if (tech->vertex_type == 3){
        using Vertex = Vertex3;
        AllocateVertexBuffer(m_mesh->GetVerticesNum() * sizeof(Vertex));
        Vertex* vertex_data_buffer = (Vertex*) m_vertex_buffer_start;
        for (uint32_t i = 0; i < m_mesh->GetVerticesNum(); i++) {
            vertex_data_buffer[i] = Vertex{m_mesh->GetVertex(i) };
        }
    }
}

void RenderModel::LoadTextures(ComPtr<ID3D12GraphicsCommandList6> & command_list){
    if (!(m_dirty & (db_diffuse_tx | db_normals_tx | db_specular_tx))){
        return;
    }

    for (uint32_t idx = 0; idx < TextureType::TextureCount; idx++){
        GpuResource *res = nullptr;
        uint8_t flag = 0;

        switch(TextureType(idx)){
            case TextureType::DiffuseTexture:
                flag = db_diffuse_tx;
                if (m_dirty & flag){
                    m_diffuse_tex = std::make_unique<GpuResource>();
                    res = m_diffuse_tex.get();
                }
                break;
            case TextureType::NormalTexture:
                flag = db_normals_tx;
                if (m_dirty & flag){
                    m_normals_tex = std::make_unique<GpuResource>();
                    res = m_normals_tex.get();
                }
                break;
            case TextureType::SpecularTexture:
                flag = db_specular_tx;
                if(m_dirty & flag){
                    m_specular_tex = std::make_unique<GpuResource>();
                    res = m_specular_tex.get();
                }
                break;
        }

        if (res && m_dirty & flag){
            D3D12_SRV_DIMENSION srv_dim = D3D12_SRV_DIMENSION_TEXTURE2D;

            CD3DX12_RESOURCE_DESC tex_desc = {};
            switch (m_textures_data[idx]->meta_data.dimension)
            {
                case DirectX::TEX_DIMENSION_TEXTURE1D:
                    tex_desc = CD3DX12_RESOURCE_DESC::Tex1D(m_textures_data[idx]->meta_data.format, static_cast<UINT64>( m_textures_data[idx]->meta_data.width ), static_cast<UINT16>(m_textures_data[idx]->meta_data.arraySize) );
                    srv_dim = D3D12_SRV_DIMENSION_TEXTURE1D;
                    break;
                case DirectX::TEX_DIMENSION_TEXTURE2D:
                    tex_desc = CD3DX12_RESOURCE_DESC::Tex2D(m_textures_data[idx]->meta_data.format, static_cast<UINT64>( m_textures_data[idx]->meta_data.width ), static_cast<UINT>(m_textures_data[idx]->meta_data.height), static_cast<UINT16>(m_textures_data[idx]->meta_data.arraySize) );
                    srv_dim = (m_textures_data[idx]->meta_data.arraySize > 1) ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
                    break;
                case DirectX::TEX_DIMENSION_TEXTURE3D:
                    srv_dim = D3D12_SRV_DIMENSION_TEXTURE3D;
                    tex_desc = CD3DX12_RESOURCE_DESC::Tex3D(m_textures_data[idx]->meta_data.format, static_cast<UINT64>( m_textures_data[idx]->meta_data.width ), static_cast<UINT>(m_textures_data[idx]->meta_data.height), static_cast<UINT16>(m_textures_data[idx]->meta_data.depth) );
                    break;
                default:
                    throw std::exception( "Invalid texture dimension." );
                    break;
            }
            
            Loadtexture(command_list, res, m_textures_data[idx], tex_desc, srv_dim, idx);
            m_dirty &= (~flag);
        }
    }
}

void RenderModel::Render(ComPtr<ID3D12GraphicsCommandList6> &command_list, const DirectX::XMFLOAT4X4 &parent_xform){
    DirectX::XMMATRIX parent_xform_mx = DirectX::XMLoadFloat4x4(&parent_xform);
    parent_xform_mx = DirectX::XMMatrixMultiply(m_transformations->GetModel(), parent_xform_mx);

    if (m_mesh && m_mesh->GetIndicesNum() > 0){
        if (std::shared_ptr<D3D12_VERTEX_BUFFER_VIEW> vert_view = m_VertexBuffer->Get_Vertex_View().lock()){
            command_list->IASetVertexBuffers(0, 1, vert_view.get());
        }
        else{
            assert(false);
        }

        if (std::shared_ptr<D3D12_INDEX_BUFFER_VIEW> ind_view = m_IndexBuffer->Get_Index_View().lock()){
            command_list->IASetIndexBuffer(ind_view.get());
        }
        else {
            assert(false);
        }
        command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        gD3DApp->SetMatrix4Constant(Constants::cM, parent_xform_mx, command_list);
        if (m_diffuse_tex){
            if (std::shared_ptr<ResourceDescriptor> srv = m_diffuse_tex->GetSRV().lock()){
                command_list->SetGraphicsRootDescriptorTable(5, srv->GetGPUhandle());
            }
        }

        if (m_constant_buffer){
            if (std::shared_ptr<HeapBuffer> buff = m_constant_buffer->GetBuffer().lock()){
                if (std::shared_ptr<GfxCommandQueue> queue = gD3DApp->GetGfxQueue().lock()){
                    queue->ResourceBarrier(*m_constant_buffer.get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
                }
                uint32_t cb_size = (sizeof(uint32_t) + 255) & ~255;
                buff->Load(command_list, 1, cb_size, &m_material_id);
                if (std::shared_ptr<GfxCommandQueue> queue = gD3DApp->GetGfxQueue().lock()){
                    queue->ResourceBarrier(*m_constant_buffer.get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
                }
            }
            if (std::shared_ptr<ResourceDescriptor> srv = m_constant_buffer->GetCBV().lock()){
                command_list->SetGraphicsRootDescriptorTable(4, srv->GetGPUhandle());
            }
        }


        TODO("Major! DrawIndexed should be at upper level where you know there are few such meshes to render")
        command_list->DrawIndexedInstanced(m_mesh->GetIndicesNum(), 1, 0, 0, 0);
    }
    DirectX::XMFLOAT4X4 new_parent_xform;
    DirectX::XMStoreFloat4x4(&new_parent_xform, parent_xform_mx);

    for (auto &child : m_children){
        child->Render(command_list, new_parent_xform);
    }
}

void RenderModel::LoadDataToGpu(ComPtr<ID3D12GraphicsCommandList6> &command_list){
    if (m_mesh && m_mesh->GetIndicesNum() > 0){
        if (m_dirty & db_vertex){
            FormVertexes();
        }
        const Techniques::Technique * tech = gD3DApp->GetTechniqueById(m_tech_id);
        LoadVertexDataOnGpu(command_list, (const void*)m_vertex_buffer_start, GetSizeByVertexType(tech->vertex_type), m_mesh->GetVerticesNum());
        LoadIndexDataOnGpu(command_list);
        LoadTextures(command_list);
        LoadConstantData(command_list);
    }

    for (auto &child : m_children){
            child->LoadDataToGpu(command_list);
    }
}

void RenderModel::SetTexture(TextureData * texture_data, TextureType type){
    m_textures_data[type] = texture_data;
    switch (type){
        case TextureType::DiffuseTexture:
            m_dirty |= db_diffuse_tx;
            break;
        case TextureType::NormalTexture:
            m_dirty |= db_normals_tx;
            break;
        case TextureType::SpecularTexture:
            //m_dirty |= db_specular_tx;
            break;
    }
}

void RenderModel::LoadConstantData(ComPtr<ID3D12GraphicsCommandList6> &command_list){
    if (m_dirty & db_rt_cbv){
        m_constant_buffer = std::make_unique<GpuResource>();
        uint32_t cb_size = (sizeof(uint32_t) + 255) & ~255;
        m_constant_buffer->CreateBuffer(HeapBuffer::BufferType::bt_default, cb_size, HeapBuffer::UseFlag::uf_none, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, std::wstring(L"models_cbv_").append(m_name));
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
        desc.SizeInBytes = cb_size;
        m_constant_buffer->Create_CBV(desc);
        m_dirty &= (~db_rt_cbv);
    }
}