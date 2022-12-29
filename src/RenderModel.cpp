#include "RenderModel.h"
#include "Transformations.h"
#include "GpuResource.h"
#include "DXHelper.h"
#include "DXAppImplementation.h"
#include "ResourceDescriptor.h"
#include "VertexFormats.h"
#include "GfxCommandQueue.h"
#include "DynamicGpuHeap.h"

extern DXAppImplementation *gD3DApp;

RenderModel::RenderModel() :
    m_transformations(std::make_unique<Transformations>())
{
    m_dirty |= db_rt_cbv;
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
    if (!(m_dirty & (db_diffuse_tx | db_normals_tx | db_metallic_tx | db_rough_tx))){
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
            case TextureType::MetallicTexture:
                flag = db_metallic_tx;
                if(m_dirty & flag){
                    m_metallic_tex = std::make_unique<GpuResource>();
                    res = m_metallic_tex.get();
                }
                break;
			case TextureType::RoughTexture:
				flag = db_rough_tx;
				if (m_dirty & flag) {
                    m_roughness_tex = std::make_unique<GpuResource>();
					res = m_roughness_tex.get();
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
        if (m_VertexBuffer) {
            if (std::shared_ptr<D3D12_VERTEX_BUFFER_VIEW> vert_view = m_VertexBuffer->Get_Vertex_View().lock()) {
                command_list->IASetVertexBuffers(0, 1, vert_view.get());
            }
            else {
                assert(false);
            }
        }

        if (std::shared_ptr<D3D12_INDEX_BUFFER_VIEW> ind_view = m_IndexBuffer->Get_Index_View().lock()){
            command_list->IASetIndexBuffer(ind_view.get());
        }
        else {
            assert(false);
        }
        command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        if (std::shared_ptr<GfxCommandQueue> gfx_queue = gD3DApp->GetGfxQueue().lock()) {
            if (m_diffuse_tex) {
                if (std::shared_ptr<ResourceDescriptor> srv = m_diffuse_tex->GetSRV().lock()) {
                    gfx_queue->GetGpuHeap().StageDesctriptor(bi_g_buffer_tex_table, tto_albedo, srv->GetCPUhandle());
                }
            }

            if (m_normals_tex) {
                if (std::shared_ptr<ResourceDescriptor> srv = m_normals_tex->GetSRV().lock()) {
                    gfx_queue->GetGpuHeap().StageDesctriptor(bi_g_buffer_tex_table, tto_normals, srv->GetCPUhandle());
                }
            }

            if (m_metallic_tex) {
                if (std::shared_ptr<ResourceDescriptor> srv = m_metallic_tex->GetSRV().lock()) {
                    gfx_queue->GetGpuHeap().StageDesctriptor(bi_g_buffer_tex_table, tto_metallic, srv->GetCPUhandle());
                }
            }

            if (m_roughness_tex) {
                if (std::shared_ptr<ResourceDescriptor> srv = m_roughness_tex->GetSRV().lock()) {
                    gfx_queue->GetGpuHeap().StageDesctriptor(bi_g_buffer_tex_table, tto_roughness, srv->GetCPUhandle());
                }
            }

            gfx_queue->GetGpuHeap().CommitRootSignature(command_list);
        }

        
        if (m_constant_buffer) {
            gD3DApp->SetModelCB(m_constant_buffer.get());
            gD3DApp->SetMatrix4Constant(Constants::cM, parent_xform_mx);
            gD3DApp->SetUint32(Constants::cMat, m_material_id);
            gD3DApp->CommitCB(command_list, cb_model);
        }

        TODO("Major! DrawIndexed should be at upper level where you know there are few such meshes to render")
        command_list->DrawIndexedInstanced(m_mesh->GetIndicesNum(), m_instance_num, 0, 0, 0);
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
        case TextureType::MetallicTexture:
            m_dirty |= db_metallic_tx;
            break;
        case TextureType::RoughTexture:
            m_dirty |= db_rough_tx;
            break;
    }
}

GpuResource* RenderModel::GetTexture(TextureType type)
{
    switch (type) {
	case RenderObject::DiffuseTexture:
		return m_diffuse_tex.get();
	default:
		assert(false);
        return nullptr;
    }
}

void RenderModel::LoadConstantData(ComPtr<ID3D12GraphicsCommandList6> &command_list){
    if (m_dirty & db_rt_cbv){
        m_constant_buffer = std::make_unique<GpuResource>();
        uint32_t cb_size = calc_cb_size(sizeof(ConstantBufferManager::ModelCB));
        m_constant_buffer->CreateBuffer(HeapBuffer::BufferType::bt_upload, cb_size, HeapBuffer::UseFlag::uf_none, D3D12_RESOURCE_STATE_GENERIC_READ, std::wstring(L"models_cbv_").append(m_name));
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
        desc.SizeInBytes = cb_size;
        m_constant_buffer->Create_CBV(desc);
		if (std::shared_ptr<HeapBuffer> buff = m_constant_buffer->GetBuffer().lock()) {
			buff->Map();
		}
        m_dirty &= (~db_rt_cbv);
    }
}