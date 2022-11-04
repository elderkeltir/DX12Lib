#include "RenderModel.h"
#include "Transformations.h"
#include "GpuResource.h"
#include "DXHelper.h"
#include "DXAppImplementation.h"
#include "ResourceDescriptor.h"

extern DXAppImplementation *gD3DApp;

RenderModel::RenderModel() :
    m_transformations(std::make_unique<Transformations>())
{

}

RenderModel::~RenderModel() = default;

void RenderModel::Load(const std::wstring &name){

}

void RenderModel::SetName(const std::wstring &name){
    m_name = name;
}

const std::wstring& RenderModel::GetName() const{
    return m_name;
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

void RenderModel::SetVertices(std::vector<DirectX::XMFLOAT3> vertices){
	m_vertices.swap(vertices);
    m_dirty |= db_vertex;
	m_vertexDataBuffer.resize(m_vertices.size());
}

void RenderModel::SetIndices(std::vector<uint16_t> indices){
    m_dirty |= db_index;
	m_indices.swap(indices);
}

void RenderModel::SetNormals(std::vector<DirectX::XMFLOAT3> normals){
    m_normals.swap(normals);
}

void RenderModel::SetTangents(std::vector<DirectX::XMFLOAT3> tangents, std::vector<DirectX::XMFLOAT3> bitangents){
    m_tangents.swap(tangents);
	m_bitangents.swap(bitangents);
}

void RenderModel::SetTextureCoords(std::vector<DirectX::XMFLOAT2> textCoords){
    m_textCoords.swap(textCoords);
}

void RenderModel::FormVertexes(){
    TODO("Normal! set a way to process different vertex format")
    for (uint32_t i = 0; i < m_vertices.size(); i++) {
        m_vertexDataBuffer[i] = Vertex{m_vertices.at(i), m_normals.at(i), m_tangents.at(i), m_bitangents.at(i), m_textCoords.at(i)};
    }
}

void RenderModel::LoadVertexDataOnGpu(ComPtr<ID3D12GraphicsCommandList6> &commandList){
    if (m_dirty & db_vertex){
        FormVertexes();
        m_VertexBuffer = std::make_unique<GpuResource>();
        m_VertexBuffer->CreateBuffer(HeapBuffer::BufferType::bt_default, ((uint32_t)m_vertexDataBuffer.size() * sizeof(Vertex)), HeapBuffer::UseFlag::uf_none, D3D12_RESOURCE_STATE_COPY_DEST, std::wstring(L"vertex_buffer").append(m_name));
        m_VertexBuffer->LoadBuffer(commandList, (uint32_t)m_vertexDataBuffer.size(), sizeof(Vertex), m_vertexDataBuffer.data());
        if (std::shared_ptr<HeapBuffer> buff = m_VertexBuffer->GetBuffer().lock()){
            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buff->GetResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
        }
        m_VertexBuffer->Create_Vertex_View(((uint32_t)m_vertexDataBuffer.size() * sizeof(Vertex)), sizeof(Vertex));
        m_dirty &= (~db_vertex);
    }
}

void RenderModel::LoadIndexDataOnGpu(ComPtr<ID3D12GraphicsCommandList6> &commandList){
    if (m_dirty & db_index){
        m_IndexBuffer = std::make_unique<GpuResource>();
        m_IndexBuffer->CreateBuffer(HeapBuffer::BufferType::bt_default, ((uint32_t)m_indices.size() * sizeof(uint16_t)), HeapBuffer::UseFlag::uf_none, D3D12_RESOURCE_STATE_COPY_DEST, std::wstring(L"index_buffer").append(m_name));
        m_IndexBuffer->LoadBuffer(commandList, (uint32_t)m_indices.size(), sizeof(uint16_t), m_indices.data());
        if (std::shared_ptr<HeapBuffer> buff = m_IndexBuffer->GetBuffer().lock()){
            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buff->GetResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));
        }
        m_IndexBuffer->Create_Index_View(DXGI_FORMAT_R16_UINT, ((uint32_t)m_indices.size() * sizeof(uint16_t)));
        m_dirty &= (~db_index);
    }
}

void RenderModel::LoadTextures(ComPtr<ID3D12GraphicsCommandList6> & commandList){
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
            D3D12_SRV_DIMENSION svr_dim = D3D12_SRV_DIMENSION_TEXTURE2D;

            CD3DX12_RESOURCE_DESC tex_desc = {};
            switch (m_textures_data[idx]->meta_data.dimension)
            {
                case DirectX::TEX_DIMENSION_TEXTURE1D:
                    tex_desc = CD3DX12_RESOURCE_DESC::Tex1D(m_textures_data[idx]->meta_data.format, static_cast<UINT64>( m_textures_data[idx]->meta_data.width ), static_cast<UINT16>(m_textures_data[idx]->meta_data.arraySize) );
                    svr_dim = D3D12_SRV_DIMENSION_TEXTURE1D;
                    break;
                case DirectX::TEX_DIMENSION_TEXTURE2D:
                    tex_desc = CD3DX12_RESOURCE_DESC::Tex2D(m_textures_data[idx]->meta_data.format, static_cast<UINT64>( m_textures_data[idx]->meta_data.width ), static_cast<UINT>(m_textures_data[idx]->meta_data.height), static_cast<UINT16>(m_textures_data[idx]->meta_data.arraySize) );
                    svr_dim = D3D12_SRV_DIMENSION_TEXTURE2D;
                    break;
                case DirectX::TEX_DIMENSION_TEXTURE3D:
                    svr_dim = D3D12_SRV_DIMENSION_TEXTURE3D;
                    tex_desc = CD3DX12_RESOURCE_DESC::Tex3D(m_textures_data[idx]->meta_data.format, static_cast<UINT64>( m_textures_data[idx]->meta_data.width ), static_cast<UINT>(m_textures_data[idx]->meta_data.height), static_cast<UINT16>(m_textures_data[idx]->meta_data.depth) );
                    break;
                default:
                    throw std::exception( "Invalid texture dimension." );
                    break;
            }
            
            D3D12_RESOURCE_STATES initial_state = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST;
            res->CreateTexture(HeapBuffer::BufferType::bt_default, tex_desc, initial_state, nullptr, std::wstring(m_name).append(L"model_srv_").append(std::to_wstring(idx)).c_str());

            const uint32_t image_count = (uint32_t)m_textures_data[idx]->scratch_image.GetImageCount();
            std::vector<D3D12_SUBRESOURCE_DATA> subresources(image_count);
            const DirectX::Image* pImages = m_textures_data[idx]->scratch_image.GetImages();
            for (uint32_t i = 0; i < (uint32_t)image_count; ++i) {
                auto& subresource = subresources[i];
                subresource.RowPitch = pImages[i].rowPitch;
                subresource.SlicePitch = pImages[i].slicePitch;
                subresource.pData = pImages[i].pixels;
            }
            res->LoadBuffer(commandList, 0, (uint32_t)subresources.size(), subresources.data());
            if (std::shared_ptr<HeapBuffer> buff = res->GetBuffer().lock()){
                commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buff->GetResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
            }

            D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
            srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srv_desc.Format = m_textures_data[idx]->meta_data.format;
            srv_desc.ViewDimension = svr_dim;
            srv_desc.Texture2D.MostDetailedMip = 0;
            srv_desc.Texture2D.MipLevels = (UINT)m_textures_data[idx]->meta_data.mipLevels;
            srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;

            res->Create_SRV(srv_desc, false);
            m_dirty &= (~flag);
        }
    }
}

void RenderModel::Render(ComPtr<ID3D12GraphicsCommandList6> &commandList, const DirectX::XMFLOAT4X4 &parent_xform){
    DirectX::XMMATRIX parent_xform_mx = DirectX::XMLoadFloat4x4(&parent_xform);
    parent_xform_mx = DirectX::XMMatrixMultiply(m_transformations->GetModel(), parent_xform_mx);

    if (!m_indices.empty()){
        if (std::shared_ptr<D3D12_VERTEX_BUFFER_VIEW> vert_view = m_VertexBuffer->Get_Vertex_View().lock()){
            commandList->IASetVertexBuffers(0, 1, vert_view.get());
        }
        else{
            assert(false);
        }

        if (std::shared_ptr<D3D12_INDEX_BUFFER_VIEW> ind_view = m_IndexBuffer->Get_Index_View().lock()){
            commandList->IASetIndexBuffer(ind_view.get());
        }
        else {
            assert(false);
        }
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        gD3DApp->SetMatrix4Constant(Constants::cM, parent_xform_mx, commandList);
        if (std::shared_ptr<ResourceDescriptor> srv = m_diffuse_tex->GetSRV().lock()){
            commandList->SetGraphicsRootDescriptorTable(4, srv->GetGPUhandle());
        }
        TODO("Major! DrawIndexed should be at upper level where you know there are few such meshes to render")
        commandList->DrawIndexedInstanced((UINT)m_indices.size(), 1, 0, 0, 0);
    }
    DirectX::XMFLOAT4X4 new_parent_xform;
    DirectX::XMStoreFloat4x4(&new_parent_xform, parent_xform_mx);

    for (auto child : m_children){
        child->Render(commandList, new_parent_xform);
    }
}

void RenderModel::LoadDataToGpu(ComPtr<ID3D12GraphicsCommandList6> &commandList){
    if (!m_indices.empty()){
        LoadVertexDataOnGpu(commandList);
        LoadIndexDataOnGpu(commandList);
        LoadTextures(commandList);
    }
    for (auto child : m_children){
            child->LoadDataToGpu(commandList);
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
