#include "RenderQuad.h"
#include "GpuResource.h"
#include "ResourceDescriptor.h"
#include "DXHelper.h"
#include "FileManager.h"
#include "DXAppImplementation.h"
#include "DXHelper.h"
#include "RenderMesh.h"
#include "VertexFormats.h"
#include "GfxCommandQueue.h"

extern DXAppImplementation *gD3DApp;

static const uint32_t vert_num = 4;

void RenderQuad::Initialize() {
    if (std::shared_ptr<FileManager> fileMgr = gD3DApp->GetFileManager().lock()){
        RenderObject* obj = (RenderObject*)this;
        fileMgr->CreateModel(L"", FileManager::Geom_type::gt_quad, obj);
    }

    m_dirty |= db_rt_tx;

    Initialized();
}

RenderQuad::~RenderQuad() = default;

void RenderQuad::FormVertex() {
    if (m_dirty & db_vertex){
        using Vertex = Vertex2;
		AllocateVertexBuffer(vert_num * sizeof(Vertex));
		Vertex* vertex_data_buffer = (Vertex*)m_vertex_buffer_start;

		for (uint32_t i = 0; i < vert_num; i++) {
			vertex_data_buffer[i] = Vertex{ m_mesh->GetVertex(i), m_mesh->GetTexCoord(i) };
		}
	}
}

void RenderQuad::LoadDataToGpu(CommandList& command_list) {
    using Vertex = Vertex2;
    FormVertex();
    LoadVertexDataOnGpu(command_list, (const void*)m_vertex_buffer_start, (uint32_t)sizeof(Vertex), vert_num);
    LoadIndexDataOnGpu(command_list);
}

bool RenderQuad::CreateQuadTexture(uint32_t width, uint32_t height, const std::vector<DXGI_FORMAT> &formats, uint32_t texture_num, uint32_t uavs, std::optional<std::wstring> dbg_name) {
    if (m_dirty & db_rt_tx){
        // Create a RTV for each frame.
        m_textures.resize(texture_num);
        for (uint32_t n = 0; n < texture_num; n++)
        {
            std::vector<std::shared_ptr<GpuResource>> &set_resources = m_textures[n];
            set_resources.resize(formats.size());
            for (uint32_t m = 0; m < formats.size(); m++){
                set_resources[m] = std::make_shared<GpuResource>();
                std::shared_ptr<GpuResource>& res = set_resources[m];

                D3D12_RESOURCE_FLAGS res_flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
                D3D12_RESOURCE_STATES res_state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
                if (uavs << m)
                    res_flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
                CD3DX12_RESOURCE_DESC res_desc = CD3DX12_RESOURCE_DESC::Tex2D(formats[m], width, height, 1, 0, 1, 0, res_flags);
                res->CreateTexture(HeapBuffer::BufferType::bt_default, res_desc, res_state, nullptr, dbg_name.value_or(L"quad_tex_").append(std::to_wstring(n).append(L"-")).append(std::to_wstring(m)));
                res->CreateRTV();

                D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
                srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                srv_desc.Format = formats[m];
                srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                srv_desc.Texture2D.MostDetailedMip = 0;
                srv_desc.Texture2D.MipLevels = 1;
                srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
                res->Create_SRV(srv_desc);

                if (uavs & 1 << m) {
					D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
					uavDesc.Format = formats[m];
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
					uavDesc.Texture2D.MipSlice = 0;
                    res->Create_UAV(uavDesc);
                }
            }
        }
        m_dirty &= (~db_rt_tx);

        return false;
    }

    return true;
}

std::weak_ptr<GpuResource> RenderQuad::GetRt(uint32_t set_idx, uint32_t idx_in_set) {
    return m_textures.at(set_idx).at(idx_in_set);
}

void RenderQuad::Render(CommandList& command_list) {
    if (m_mesh->GetIndicesNum() > 0){
        if (std::shared_ptr<D3D12_VERTEX_BUFFER_VIEW> vert_view = m_VertexBuffer->Get_Vertex_View().lock()){
            command_list.IASetVertexBuffers(0, 1, vert_view.get());
        }
        else{
            assert(false);
        }

        if (std::shared_ptr<D3D12_INDEX_BUFFER_VIEW> ind_view = m_IndexBuffer->Get_Index_View().lock()){
            command_list.IASetIndexBuffer(ind_view.get());
        }
        else {
            assert(false);
        }
        command_list.IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        command_list.DrawIndexedInstanced((UINT)m_mesh->GetIndicesNum(), 1, 0, 0, 0);
    }
}