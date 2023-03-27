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

bool RenderQuad::CreateQuadTexture(uint32_t width, uint32_t height, const std::vector<ResourceFormat> &formats, uint32_t texture_num, uint32_t uavs, std::optional<std::wstring> dbg_name) {
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

                uint32_t res_flags = ResourceDesc::ResourceFlags::rf_allow_render_target;

                if (uavs << m)
                    res_flags |= ResourceDesc::ResourceFlags::rf_allow_unordered_access;
                ResourceDesc res_desc = ResourceDesc::tex_2d(formats[m], width, height, 1, 0, 1, 0, (ResourceDesc::ResourceFlags)res_flags);
                res->CreateTexture(HeapType::ht_default, res_desc, ResourceState::rs_resource_state_pixel_shader_resource, nullptr, dbg_name.value_or(L"quad_tex_").append(std::to_wstring(n).append(L"-")).append(std::to_wstring(m)));
                res->CreateRTV();

                SRVdesc srv_desc = {};
                srv_desc.format = formats[m];
                srv_desc.dimension = SRVdesc::SRVdimensionType::srv_dt_texture2d;
                srv_desc.texture2d.most_detailed_mip = 0;
                srv_desc.texture2d.mip_levels = 1;
                srv_desc.texture2d.res_min_lod_clamp = 0.0f;
                res->Create_SRV(srv_desc);

                if (uavs & 1 << m) {
					UAVdesc uavDesc = {};
					uavDesc.format = formats[m];
					uavDesc.dimension = UAVdesc::UAVdimensionType::uav_dt_texture2d;
					uavDesc.texture2d.mip_slice = 0;
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
    command_list.IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    command_list.DrawInstanced(6, 1, 0, 0);
}