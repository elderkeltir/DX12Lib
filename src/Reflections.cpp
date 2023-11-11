#include "Reflections.h"
#include "Frontend.h"
#include "defines.h"
#include "IGpuResource.h"


extern Frontend* gFrontend;

void Reflections::Initialize()
{
	if (m_dirty & df_init) {
		const uint32_t width = gFrontend->GetWidth();
		const uint32_t height = gFrontend->GetHeight();

		for (uint32_t i = 0; i < rt_num; i++) {
			m_reflection_map[i].reset(CreateGpuResource(gFrontend->GetBackendType()));
			auto& res = *m_reflection_map[i];

			HeapType h_type = HeapType(HeapType::ht_default | HeapType::ht_image_storage | HeapType::ht_image_sampled | HeapType::ht_aspect_color_bit);
			ResourceDesc res_desc = ResourceDesc::tex_2d(ResourceFormat::rf_r16g16b16a16_float, width, height, 1, 0, 1, 0, ResourceDesc::ResourceFlags::rf_allow_unordered_access);
			res.CreateTexture(h_type, res_desc, ResourceState::rs_resource_state_pixel_shader_resource, nullptr, std::optional<std::wstring>().value_or(L"reflection_map").append(std::to_wstring(i).append(L"-")).append(std::to_wstring(i)));

			SRVdesc srv_desc = {};
			srv_desc.format = ResourceFormat::rf_r16g16b16a16_float;
			srv_desc.dimension = SRVdesc::SRVdimensionType::srv_dt_texture2d;
			srv_desc.texture2d.most_detailed_mip = 0;
			srv_desc.texture2d.mip_levels = 1;
			srv_desc.texture2d.res_min_lod_clamp = 0.0f;
			res.Create_SRV(srv_desc);

			UAVdesc uavDesc = {};
			uavDesc.format = ResourceFormat::rf_r16g16b16a16_float;
			uavDesc.dimension = UAVdesc::UAVdimensionType::uav_dt_texture2d;
			uavDesc.texture2d.mip_slice = 0;
			res.Create_UAV(uavDesc);
		}

		m_dirty &= (~df_init);
	}
}

void Reflections::GenerateReflections(ICommandList& command_list)
{
	// TODO: what for?
}
