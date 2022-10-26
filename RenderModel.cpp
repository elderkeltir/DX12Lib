#include "RenderModel.h"
#include "Transformations.h"

RenderModel::RenderModel() :
    m_transformations(std::make_unique<Transformations>())
{

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

void RenderModel::SetVertices(std::unique_ptr<DirectX::XMFLOAT3[]> vertices, uint32_t size){
	m_vertices.swap(vertices);
	m_vertexCnt = size;

	m_vertexDataBuffer.resize(m_vertexCnt);
}

void RenderModel::SetIndices(std::unique_ptr<uint32_t[]> indices, uint32_t size){
	m_indices.swap(indices);
	m_indexCnt = size;
}

void RenderModel::SetNormals(std::unique_ptr<DirectX::XMFLOAT3[]> normals){
    m_normals.swap(normals);
}

void RenderModel::SetTangents(std::unique_ptr<DirectX::XMFLOAT3[]> tangents, std::unique_ptr<DirectX::XMFLOAT3[]> bitangents){
    m_tangents.swap(tangents);
	m_bitangents.swap(bitangents);
}

void RenderModel::SetTextureCoords(std::unique_ptr<DirectX::XMFLOAT2[]> textCoords){
    m_textCoords.swap(textCoords);
}

void RenderModel::SetTexturePath(const char* path, TextureType type){
    m_textures[type] = path;
}