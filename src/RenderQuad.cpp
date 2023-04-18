#include "RenderQuad.h"
#include "IGpuResource.h"
#include "FileManager.h"
#include "Frontend.h"
#include "RenderHelper.h"
#include "VertexFormats.h"
#include "ICommandList.h"

extern Frontend* gFrontend;

static const uint32_t vert_num = 4;

void RenderQuad::Initialize() {
    if (std::shared_ptr<FileManager> fileMgr = gFrontend->GetFileManager().lock()){
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
            std::vector<std::shared_ptr<IGpuResource>> &set_resources = m_textures[n];
            set_resources.resize(formats.size());
            for (uint32_t m = 0; m < formats.size(); m++){
                set_resources[m].reset(CreateGpuResource());
                std::shared_ptr<IGpuResource>& res = set_resources[m];

                uint32_t res_flags = ResourceDesc::ResourceFlags::rf_allow_render_target;

                HeapType h_type = HeapType(HeapType::ht_default | HeapType::ht_image_sampled | HeapType::ht_image_color_attach);
                if (uavs << m) {
                    res_flags |= ResourceDesc::ResourceFlags::rf_allow_unordered_access;
                    h_type = HeapType(h_type | HeapType::ht_image_storage);
                }
                ResourceDesc res_desc = ResourceDesc::tex_2d(formats[m], width, height, 1, 0, 1, 0, (ResourceDesc::ResourceFlags)res_flags);
                res->CreateTexture(h_type, res_desc, ResourceState::rs_resource_state_pixel_shader_resource, nullptr, dbg_name.value_or(L"quad_tex_").append(std::to_wstring(n).append(L"-")).append(std::to_wstring(m)));
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

std::weak_ptr<IGpuResource> RenderQuad::GetRt(uint32_t set_idx, uint32_t idx_in_set) {
    return m_textures.at(set_idx).at(idx_in_set);
}

void RenderQuad::Render(ICommandList* command_list) {
    command_list->SetPrimitiveTopology(PrimitiveTopology::pt_trianglelist);
    command_list->DrawInstanced(6, 1, 0, 0);
}

void RenderQuad::CreateFrameBuffer(IGpuResource* depth, uint32_t tech_id) {
    for (uint32_t i = 0; i < m_textures.size(); i++) {
        std::vector<std::shared_ptr<IGpuResource>> &resources = m_textures[i];
        std::vector<IGpuResource*> rts(resources.size());
        for (uint32_t j = 0; j < resources.size(); j++){
            rts[j] = resources[j].get();
        }

        gFrontend->CreateFrameBuffer(rts, depth, tech_id);
    }
}
