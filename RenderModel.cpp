#include "RenderModel.h"
#include "Transformations.h"
#include "GpuResource.h"
#include "DXHelper.h"

struct VertexPosColor
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Color;
};

static VertexPosColor g_Vertices[8] = {
    { DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0
    { DirectX::XMFLOAT3(-1.0f,  1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f) }, // 1
    { DirectX::XMFLOAT3(1.0f,  1.0f, -1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 2
    { DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 3
    { DirectX::XMFLOAT3(-1.0f, -1.0f,  1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 4
    { DirectX::XMFLOAT3(-1.0f,  1.0f,  1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 1.0f) }, // 5
    { DirectX::XMFLOAT3(1.0f,  1.0f,  1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f) }, // 6
    { DirectX::XMFLOAT3(1.0f, -1.0f,  1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 1.0f) }  // 7
};

static WORD g_Indicies[36] =
{
    0, 1, 2, 0, 2, 3,
    4, 6, 5, 4, 7, 6,
    4, 5, 1, 4, 1, 0,
    3, 2, 6, 3, 6, 7,
    1, 5, 6, 1, 6, 2,
    4, 0, 3, 4, 3, 7
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

void RenderModel::SetIndices(std::vector<uint32_t> indices){
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
        m_VertexBuffer->CreateBuffer(HeapBuffer::BufferType::bt_deafult, (_countof(g_Vertices) * sizeof(VertexPosColor)), HeapBuffer::UseFlag::uf_none, D3D12_RESOURCE_STATE_COPY_DEST);
        m_VertexBuffer->LoadBuffer(commandList, _countof(g_Vertices), sizeof(VertexPosColor), g_Vertices);
        if (std::shared_ptr<HeapBuffer> buff = m_VertexBuffer->GetBuffer().lock()){
            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buff->GetResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
        }
        m_VertexBuffer->Create_Vertex_View(sizeof(g_Vertices), sizeof(VertexPosColor));
        m_dirty &= (~db_vertex);
    }
}

void RenderModel::LoadIndexDataOnGpu(ComPtr<ID3D12GraphicsCommandList6> &commandList){
    if (m_dirty & db_index){
        m_IndexBuffer = std::make_unique<GpuResource>();
        m_IndexBuffer->CreateBuffer(HeapBuffer::BufferType::bt_deafult, (_countof(g_Indicies) * sizeof(WORD)), HeapBuffer::UseFlag::uf_none, D3D12_RESOURCE_STATE_COPY_DEST);
        m_IndexBuffer->LoadBuffer(commandList, _countof(g_Indicies), sizeof(WORD), g_Indicies);
        if (std::shared_ptr<HeapBuffer> buff = m_IndexBuffer->GetBuffer().lock()){
            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buff->GetResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));
        }
        m_IndexBuffer->Create_Index_View(DXGI_FORMAT_R16_UINT, sizeof(g_Indicies));
        m_dirty &= (~db_index);
    }
}

void RenderModel::Render(ComPtr<ID3D12GraphicsCommandList6> &commandList){
TODO("Minor: avoid every frame call to function")
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
    
    TODO("DrawIndexed should be at upper level where you know there are few such meshes to render")
    commandList->DrawIndexedInstanced(_countof(g_Indicies), 1, 0, 0, 0);
}
