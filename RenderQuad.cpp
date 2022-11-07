#include "RenderQuad.h"
#include "GpuResource.h"
#include "ResourceDescriptor.h"
#include "DXHelper.h"

void RenderQuad::Initialize(uint32_t texture_num) {
    m_texture_num = texture_num;

    std::vector<DirectX::XMFLOAT3> vertices;
    vertices.push_back(DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f));
    vertices.push_back(DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f));
    vertices.push_back(DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f));
    vertices.push_back(DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f));
    SetVertices(vertices);

    std::vector<DirectX::XMFLOAT2> tex_coords;
    tex_coords.push_back(DirectX::XMFLOAT2(0.0f, 1.0f));
    tex_coords.push_back(DirectX::XMFLOAT2(0.0f, 0.0f));
    tex_coords.push_back(DirectX::XMFLOAT2(1.0f, 0.0f));
    tex_coords.push_back(DirectX::XMFLOAT2(1.0f, 1.0f));
    SetTextureCoords(tex_coords);

    std::vector<uint16_t> indices;
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);
    SetIndices(indices);

    m_dirty |= db_rt_tx;

    Initialized();
}

RenderQuad::~RenderQuad() = default;

void RenderQuad::FormVertex() {
    const uint32_t vert_num = 4;
    m_vertexDataBuffer.resize(vert_num);
    for (uint32_t i = 0; i < vert_num; i++){
        m_vertexDataBuffer.at(i) = Vertex{m_vertices.at(i), m_textCoords.at(i)};
    }
}

void RenderQuad::LoadDataToGpu(ComPtr<ID3D12GraphicsCommandList6> &command_list) {
    FormVertex();
    LoadVertexDataOnGpu(command_list, m_vertexDataBuffer.data(), (uint32_t)sizeof(Vertex), (uint32_t)m_vertexDataBuffer.size());
    LoadIndexDataOnGpu(command_list);
}

void RenderQuad::CreateQuadTexture(uint32_t width, uint32_t height) {
    if (m_dirty & db_rt_tx){
        // Create a RTV for each frame.
        for (uint32_t n = 0; n < m_texture_num; n++)
        {
            std::shared_ptr<GpuResource> res(std::make_shared<GpuResource>());
            CD3DX12_RESOURCE_DESC res_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
            res->CreateTexture(HeapBuffer::BufferType::bt_default, res_desc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, L"Quad's texture");
            res->CreateRTV();

            D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
            srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srv_desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
            srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srv_desc.Texture2D.MostDetailedMip = 0;
            srv_desc.Texture2D.MipLevels = 1;
            srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
            res->Create_SRV(srv_desc, true);

            m_textures.push_back(std::move(res));
        }
        m_dirty &= (~db_rt_tx);
    }
}

void RenderQuad::SetSrv(ComPtr<ID3D12GraphicsCommandList6> &command_list, uint32_t idx) {
    if (std::shared_ptr<ResourceDescriptor> srv = m_textures.at(idx)->GetSRV().lock()){
        command_list->SetGraphicsRootDescriptorTable(0, srv->GetGPUhandle());
    }
}

std::weak_ptr<GpuResource> RenderQuad::GetRt(uint32_t idx) {
    return m_textures.at(idx);
}

void RenderQuad::Render(ComPtr<ID3D12GraphicsCommandList6> &command_list) {
    if (!m_indices.empty()){
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
        
        TODO("Major! DrawIndexed should be at upper level where you know there are few such meshes to render")
        command_list->DrawIndexedInstanced((UINT)m_indices.size(), 1, 0, 0, 0);
    }
}