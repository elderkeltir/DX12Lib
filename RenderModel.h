#pragma once

#include <directx/d3d12.h>
#include <DirectXMath.h>
#include <variant>
#include <memory>
#include <vector>
#include <array>
#include <string>
#include <wrl.h>
#include "TextureData.h"

using Microsoft::WRL::ComPtr;

class Transformations;
class GpuResource;

class RenderModel : protected pro_game_containers::object {
public:
    struct Vertex
	{
		DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 TextCoord;
		
		//DirectX::XMFLOAT3 Tangents;
		//DirectX::XMFLOAT3 Bitangents;

		//Vertex() {}
		//Vertex(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT2 textCoord) : Position(pos), TextCoord(textCoord) {}
	};
public:
    enum TextureType { DiffuseTexture = 0, NormalTexture, SpecularTexture, TextureCount };
public:
    RenderModel();
    ~RenderModel();

    void Load(const std::wstring &name);
    void Render(ComPtr<ID3D12GraphicsCommandList6> &commandList, const DirectX::XMFLOAT4X4 &parent_xform);
    void LoadDataToGpu(ComPtr<ID3D12GraphicsCommandList6> &commandList);

    void SetId(uint32_t id) { m_id = id; }
    uint32_t GetId() const { return m_id; }
    void SetName(const std::wstring &name);
    const std::wstring& GetName() const;

    void Initialized() { m_is_initialized = true; }
    bool IsInitialized() const { return m_is_initialized; }
    void AddChild(RenderModel * child) { m_children.push_back(child); }
    const std::vector<RenderModel*>& GetChildren() const { return m_children; }

    void Move(const DirectX::XMFLOAT3 &pos);
    void Rotate(const DirectX::XMFLOAT3 &angles);
    void Scale(const DirectX::XMFLOAT3 &scale);

    void SetVertices(std::vector<DirectX::XMFLOAT3> vertices);
    void SetIndices(std::vector<uint16_t> indices);
    void SetNormals(std::vector<DirectX::XMFLOAT3> normals);
    void SetTangents(std::vector<DirectX::XMFLOAT3> tangents, std::vector<DirectX::XMFLOAT3> bitangents);

    void SetTextureCoords(std::vector<DirectX::XMFLOAT2> textCoords);
    void SetTexture(TextureData * texture_data, TextureType type);
private:
    inline void FormVertexes();
    inline void LoadVertexDataOnGpu(ComPtr<ID3D12GraphicsCommandList6> &commandList);
    inline void LoadIndexDataOnGpu(ComPtr<ID3D12GraphicsCommandList6> &commandList);
    inline void LoadTextures(ComPtr<ID3D12GraphicsCommandList6> & commandList);

    inline std::variant GetDataByAssignment(data_assignment ass, uint32_t idx){
        std::variant<float, float2, float3, float4> var;
        switch(ass){
            case pos:
                var = m_vertexDataBuffer[idx]; // assign float3
            break;
        }

        return var;
    }
#define VariantGetFloat(valiant) variant.get<float>()
#define VariantGetFloat3(valiant) variant.get<DirectX::XMFLOAT3>()
    void build_vertex(){
        for(i in vert.size()){
            for (j in assingments){
                // pos
                vertex vet;
                SetVertParam(vert, GetDataByAssignment(asses[j], i), j);
                switch (ass){
                    case pos:
                        vertex.position = VariantGetFloat3(variant);
                        break;
                    case normals:
                        vertex.normal = VariantGetFloat3(variant);
                        break;
                    case tex_coords:
                        vertex.tex_coords = VariantGetFloat2(variant);
                        break;
                    case tangents:
                        vertex.tangents = VariantGetFloat2(variant);
                        break;
                    case bitangents:
                        vertex.bitangents = VariantGetFloat2(variant);
                        break;
                    default:
                        assert(false);
                        break;
                }
            }
        }
    }

    enum dirty_bits {
        db_vertex       = 1 << 0,
        db_index        = 1 << 1,
        db_diffuse_tx   = 1 << 2,
        db_normals_tx   = 1 << 3,
        db_specular_tx  = 1 << 4
    };

    std::vector<DirectX::XMFLOAT3> m_vertices;
    std::vector<uint16_t> m_indices;
    std::vector<DirectX::XMFLOAT3> m_normals;
    std::vector<DirectX::XMFLOAT3> m_tangents;
    std::vector<DirectX::XMFLOAT3> m_bitangents;
    std::vector<DirectX::XMFLOAT2> m_textCoords;

    std::unique_ptr<GpuResource> m_VertexBuffer;
    std::unique_ptr<GpuResource> m_IndexBuffer;
    std::vector<Vertex> m_vertexDataBuffer;

    std::unique_ptr<GpuResource> m_diffuse_tex;
    std::unique_ptr<GpuResource> m_normals_tex;
    std::unique_ptr<GpuResource> m_specular_tex;
    std::array<TextureData*, TextureCount> m_textures_data;

    uint8_t m_dirty{0};

    std::unique_ptr<Transformations> m_transformations;

    std::vector<RenderModel*> m_children;
    std::wstring m_name;
    bool m_is_initialized{false};
    uint32_t m_id{uint32_t(-1)};
};