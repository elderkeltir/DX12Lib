#include "RenderModel.h"
#include "Transformations.h"
#include "GpuResource.h"
#include "DXHelper.h"
#include "DXAppImplementation.h"

extern DXAppImplementation *gD3DApp;

struct VertexPos{
    DirectX::XMFLOAT3 Position;
};


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

	//m_vertexDataBuffer.resize(m_vertices.size());
}

void RenderModel::SetIndices(std::vector<uint16_t> indices){
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

void RenderModel::SetTexturePath(const char* path, TextureType type){
    m_textures[type] = path;
}

void RenderModel::LoadVertexDataOnGpu(ComPtr<ID3D12GraphicsCommandList6> &commandList){
    if (m_dirty & db_vertex){
        m_VertexBuffer = std::make_unique<GpuResource>();
        m_VertexBuffer->CreateBuffer(HeapBuffer::BufferType::bt_deafult, ((uint32_t)m_vertices.size() * sizeof(VertexPos)), HeapBuffer::UseFlag::uf_none, D3D12_RESOURCE_STATE_COPY_DEST);
        m_VertexBuffer->LoadBuffer(commandList, (uint32_t)m_vertices.size(), sizeof(VertexPos), m_vertices.data());
        if (std::shared_ptr<HeapBuffer> buff = m_VertexBuffer->GetBuffer().lock()){
            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buff->GetResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
        }
        m_VertexBuffer->Create_Vertex_View(((uint32_t)m_vertices.size() * sizeof(VertexPos)), sizeof(VertexPos));
        m_dirty &= (~db_vertex);
    }
}

void RenderModel::LoadIndexDataOnGpu(ComPtr<ID3D12GraphicsCommandList6> &commandList){
    if (m_dirty & db_index){
        m_IndexBuffer = std::make_unique<GpuResource>();
        m_IndexBuffer->CreateBuffer(HeapBuffer::BufferType::bt_deafult, ((uint32_t)m_indices.size() * sizeof(uint16_t)), HeapBuffer::UseFlag::uf_none, D3D12_RESOURCE_STATE_COPY_DEST);
        m_IndexBuffer->LoadBuffer(commandList, (uint32_t)m_indices.size(), sizeof(uint16_t), m_indices.data());
        if (std::shared_ptr<HeapBuffer> buff = m_IndexBuffer->GetBuffer().lock()){
            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buff->GetResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));
        }
        m_IndexBuffer->Create_Index_View(DXGI_FORMAT_R16_UINT, ((uint32_t)m_indices.size() * sizeof(uint16_t)));
        m_dirty &= (~db_index);
    }
}

void RenderModel::Render(ComPtr<ID3D12GraphicsCommandList6> &commandList, const DirectX::XMFLOAT4X4 &parent_xform){
TODO("Minor: avoid every frame call to function")
    DirectX::XMMATRIX parent_xform_mx = DirectX::XMLoadFloat4x4(&parent_xform);
    parent_xform_mx = DirectX::XMMatrixMultiply(m_transformations->GetModel(), parent_xform_mx);

    if (!m_indices.empty()){
        LoadVertexDataOnGpu(commandList);
        LoadIndexDataOnGpu(commandList);

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
        TODO("Major! DrawIndexed should be at upper level where you know there are few such meshes to render")
        commandList->DrawIndexedInstanced((UINT)m_indices.size(), 1, 0, 0, 0);
    }
    DirectX::XMFLOAT4X4 new_parent_xform;
    DirectX::XMStoreFloat4x4(&new_parent_xform, parent_xform_mx);

    for (auto child : m_children){
        child->Render(commandList, new_parent_xform);
    }
}
