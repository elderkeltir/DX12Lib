#include "RenderMesh.h"
#include "Transformations.h"

RenderMesh::RenderMesh() :
    m_transformations(std::make_unique<Transformations>())
{

}

RenderMesh::~RenderMesh() = default;

void RenderMesh::Load(const std::wstring &name){

}

void RenderMesh::SetName(const std::wstring &name){
    m_name = name;
}

const std::wstring& RenderMesh::GetName() const{
    return m_name;
}

void RenderMesh::Move(const DirectX::XMFLOAT3 &pos){
    m_transformations->Move(pos);
}

void RenderMesh::Rotate(const DirectX::XMFLOAT3 &angles){
    m_transformations->Rotate(angles);
}

void RenderMesh::Scale(const DirectX::XMFLOAT3 &scale){
    m_transformations->Scale(scale);
}

void RenderMesh::SetVertices(std::vector<DirectX::XMFLOAT3> vertices){
	m_vertices.swap(vertices);

	m_vertexDataBuffer.resize(m_vertices.size());
}

void RenderMesh::SetIndices(std::vector<uint32_t> indices){
	m_indices.swap(indices);
}

void RenderMesh::SetNormals(std::vector<DirectX::XMFLOAT3> normals){
    m_normals.swap(normals);
}

void RenderMesh::SetTangents(std::vector<DirectX::XMFLOAT3> tangents, std::vector<DirectX::XMFLOAT3> bitangents){
    m_tangents.swap(tangents);
	m_bitangents.swap(bitangents);
}

void RenderMesh::SetTextureCoords(std::vector<DirectX::XMFLOAT2> textCoords){
    m_textCoords.swap(textCoords);
}

void RenderMesh::SetTexturePath(const char* path, TextureType type){
    m_textures[type] = path;
}