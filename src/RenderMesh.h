#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <DirectXMath.h>

class RenderMesh {
public:
    void SetName(const std::wstring &name) { m_name = name; }
    const std::wstring& GetName() const { return m_name; }
    void SetId(uint32_t id) { m_id = id; }
    uint32_t GetId() const { return m_id; }
    uint32_t GetIndicesNum() const { return (uint32_t)m_indices.size(); }
    uint32_t GetVerticesNum() const { return (uint32_t)m_vertices.size(); }
    void* GetIndicesData() const { return (void*)m_indices.data(); }
    const DirectX::XMFLOAT3& GetVertex(uint32_t idx) const { return m_vertices[idx]; }
    const DirectX::XMFLOAT2& GetTexCoord(uint32_t idx) const { return m_textCoords[idx]; }
    const DirectX::XMFLOAT3& GetNormal(uint32_t idx) const { return m_normals[idx]; }
    const DirectX::XMFLOAT3& GetTangent(uint32_t idx) const { return m_tangents[idx]; }
    const DirectX::XMFLOAT3& GetBiTangent(uint32_t idx) const { return m_bitangents[idx]; }

    virtual void SetVertices(std::vector<DirectX::XMFLOAT3> vertices) {
        m_vertices.swap(vertices);
    }

    virtual void SetIndices(std::vector<uint16_t> indices) {
        m_indices.swap(indices);
    }

    virtual void SetTextureCoords(std::vector<DirectX::XMFLOAT2> textCoords) {
        m_textCoords.swap(textCoords);
    }

    void SetNormals(std::vector<DirectX::XMFLOAT3> normals){
        m_normals.swap(normals);
    }

    void SetTangents(std::vector<DirectX::XMFLOAT3> tangents, std::vector<DirectX::XMFLOAT3> bitangents){
        m_tangents.swap(tangents);
        m_bitangents.swap(bitangents);
    }

private:
    std::vector<DirectX::XMFLOAT3> m_vertices;
    std::vector<uint16_t> m_indices;
    std::vector<DirectX::XMFLOAT2> m_textCoords;
    std::vector<DirectX::XMFLOAT3> m_normals;
    std::vector<DirectX::XMFLOAT3> m_tangents;
    std::vector<DirectX::XMFLOAT3> m_bitangents;

    std::wstring m_name;
    uint32_t m_id{uint32_t(-1)};
};