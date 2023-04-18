#include "RenderModel.h"
#include "Transformations.h"
#include "IGpuResource.h"
#include "RenderHelper.h"
#include "Frontend.h"
#include "VertexFormats.h"
#include "ICommandQueue.h"
#include "IDynamicGpuHeap.h"
#include "GpuDataManager.h"
#include "defines.h"
#include "ICommandList.h"
#include "IResourceDescriptor.h"
#include "IHeapBuffer.h"
#include "ITechniques.h"
#include "defines.h"
#include "ConstantBufferManager.h"
#include "Frontend.h"
#include "Level.h"
#include "FileManager.h"

extern Frontend* gFrontend;

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
    const ITechniques::Technique * tech = gFrontend->GetTechniqueById(m_tech_id);

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

void RenderModel::LoadTextures(ICommandList* command_list){
    if (!(m_dirty & (db_diffuse_tx | db_normals_tx | db_metallic_tx | db_rough_tx))){
        return;
    }

    for (uint32_t idx = 0; idx < TextureType::TextureCount; idx++){
        IGpuResource *res = nullptr;
        uint8_t flag = 0;

        switch(TextureType(idx)){
            case TextureType::DiffuseTexture:
                flag = db_diffuse_tx;
                if (m_dirty & flag){
                    m_diffuse_tex.reset(CreateGpuResource());
                    res = m_diffuse_tex.get();
                }
                break;
            case TextureType::NormalTexture:
                flag = db_normals_tx;
                if (m_dirty & flag){
                    m_normals_tex.reset(CreateGpuResource());
                    res = m_normals_tex.get();
                }
                break;
            case TextureType::MetallicTexture:
                flag = db_metallic_tx;
                if(m_dirty & flag){
                    m_metallic_tex.reset(CreateGpuResource());
                    res = m_metallic_tex.get();
                }
                break;
			case TextureType::RoughTexture:
				flag = db_rough_tx;
				if (m_dirty & flag) {
                    m_roughness_tex.reset(CreateGpuResource());
					res = m_roughness_tex.get();
				}
				break;
        }

        if (res && m_dirty & flag){
            if (std::shared_ptr<FileManager> fm = gFrontend->GetFileManager().lock()) {
                fm->LoadTextureOnGPU(command_list, res, m_textures_data[idx]);
            }

            m_dirty &= (~flag);
        }
    }
}

void RenderModel::Render(ICommandList* command_list, const DirectX::XMFLOAT4X4 &parent_xform){
    DirectX::XMMATRIX parent_xform_mx = DirectX::XMLoadFloat4x4(&parent_xform);
    parent_xform_mx = DirectX::XMMatrixMultiply(m_transformations->GetModel(), parent_xform_mx);

    if (m_mesh && m_mesh->GetIndicesNum() > 0){
        if (std::shared_ptr<IndexVufferView> ind_view = m_IndexBuffer->Get_Index_View().lock()){
            command_list->SetIndexBuffer(ind_view.get());
        }
        else {
            assert(false);
        }
        command_list->SetPrimitiveTopology(PrimitiveTopology::pt_trianglelist);
        ICommandQueue * gfx_queue = command_list->GetQueue();
        if (m_diffuse_tex) {
            if (std::shared_ptr<IResourceDescriptor> srv = m_diffuse_tex->GetSRV().lock()) {
                gfx_queue->GetGpuHeap().StageDesctriptorInTable(bi_g_buffer_tex_table, tto_albedo, srv);
            }
        }

        if (m_normals_tex) {
            if (std::shared_ptr<IResourceDescriptor> srv = m_normals_tex->GetSRV().lock()) {
                gfx_queue->GetGpuHeap().StageDesctriptorInTable(bi_g_buffer_tex_table, tto_normals, srv);
            }
        }

        if (m_metallic_tex) {
            if (std::shared_ptr<IResourceDescriptor> srv = m_metallic_tex->GetSRV().lock()) {
                gfx_queue->GetGpuHeap().StageDesctriptorInTable(bi_g_buffer_tex_table, tto_metallic, srv);
            }
        }

        if (m_roughness_tex) {
            if (std::shared_ptr<IResourceDescriptor> srv = m_roughness_tex->GetSRV().lock()) {
                gfx_queue->GetGpuHeap().StageDesctriptorInTable(bi_g_buffer_tex_table, tto_roughness, srv);
            }
        }

        gfx_queue->GetGpuHeap().CommitRootSignature(command_list);
        
        if (m_constant_buffer) {
            const ITechniques::Technique* tech = gFrontend->GetTechniqueById(m_tech_id);
            gFrontend->SetModelCB(m_constant_buffer.get());
            gFrontend->SetUint32(Constants::cVertexType, tech->vertex_type);
            gFrontend->SetMatrix4Constant(Constants::cM, parent_xform_mx);
            gFrontend->SetUint32(Constants::cMat, m_material_id);
            if (std::shared_ptr<GpuDataManager> gpu_res_mgr = gFrontend->GetGpuDataManager().lock()) {
                const uint64_t base = gpu_res_mgr->GetBase();
                gFrontend->SetUint32(Constants::cVertexBufferOffset, uint32_t(m_vertex_buffer_start - base));
                gpu_res_mgr->GetVertexBuffer();
            }
            
            gFrontend->CommitCB(command_list, cb_model);
        }

        //TODO("Major! DrawIndexed should be at upper level where you know there are few such meshes to render");
        command_list->DrawIndexedInstanced(m_mesh->GetIndicesNum(), m_instance_num, 0, 0, 0);
    }
    DirectX::XMFLOAT4X4 new_parent_xform;
    DirectX::XMStoreFloat4x4(&new_parent_xform, parent_xform_mx);

    for (auto &child : m_children){
        child->Render(command_list, new_parent_xform);
    }
}

void RenderModel::LoadDataToGpu(ICommandList* command_list){
    if (m_mesh && m_mesh->GetIndicesNum() > 0){
        if (m_dirty & db_vertex){
            FormVertexes();
            m_dirty &= (~db_vertex);
        }
        const ITechniques::Technique * tech = gFrontend->GetTechniqueById(m_tech_id);
        LoadIndexDataOnGpu(command_list);
        LoadTextures(command_list);
        LoadConstantData(command_list);
    }

    for (auto &child : m_children){
            child->LoadDataToGpu(command_list);
    }
}

void RenderModel::SetTexture(ITextureLoader::TextureData * texture_data, TextureType type){
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

IGpuResource* RenderModel::GetTexture(TextureType type)
{
    switch (type) {
	case RenderObject::DiffuseTexture:
		return m_diffuse_tex.get();
	default:
		assert(false);
        return nullptr;
    }
}

void RenderModel::LoadConstantData(ICommandList* command_list){
    if (m_dirty & db_rt_cbv){
        m_constant_buffer.reset(CreateGpuResource());
        uint32_t cb_size = calc_cb_size(sizeof(ConstantBufferManager::ModelCB));
        HeapType h_type = HeapType(HeapType::ht_upload | HeapType::ht_buff_uniform_buffer);
        m_constant_buffer->CreateBuffer(h_type, cb_size, ResourceState::rs_resource_state_generic_read, std::wstring(L"models_cbv_").append(m_name));
        CBVdesc desc;
        desc.size_in_bytes = cb_size;
        m_constant_buffer->Create_CBV(desc);
		if (std::shared_ptr<IHeapBuffer> buff = m_constant_buffer->GetBuffer().lock()) {
			buff->Map();
		}
        m_dirty &= (~db_rt_cbv);
    }
}