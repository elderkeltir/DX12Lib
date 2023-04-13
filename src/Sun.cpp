#include "Sun.h"
#include "Level.h"
#include "IGpuResource.h"
#include "ICommandQueue.h"
#include "ICommandList.h"
#include "Frontend.h"
#include "FreeCamera.h"
#include "IDynamicGpuHeap.h"
#include "ITechniques.h"

extern Frontend* gFrontend;

void Sun::Initialize(ICommandList* command_list)
{
	if (m_dirty & df_init) {
		const float width = (float)gFrontend->GetWidth();
		const float height = (float)gFrontend->GetHeight();

		for (uint32_t i = 0; i < rt_num; i++) {
			m_shadow_map[i].reset(CreateGpuResource());

			ClearColor depthOptimizedClearValue = {};
			depthOptimizedClearValue.format = ResourceFormat::rf_d32_float;
			depthOptimizedClearValue.isDepth = true;
			depthOptimizedClearValue.depth_tencil.depth = 1.0f;
			depthOptimizedClearValue.depth_tencil.stencil = 0;
			ResourceDesc res_desc = ResourceDesc::tex_2d(ResourceFormat::rf_d32_float, (uint64_t)width, (uint32_t)height, 1, 0, 1, 0, ResourceDesc::rf_allow_depth_stencil);

			m_shadow_map[i]->CreateTexture(HeapType::ht_default, res_desc, ResourceState::rs_resource_state_depth_read, &depthOptimizedClearValue, L"sun_shadow_map");

			DSVdesc depthStencilDesc = {};
			depthStencilDesc.format = ResourceFormat::rf_d32_float;
			depthStencilDesc.dimension = DSVdesc::DSVdimensionType::dsv_dt_texture2d;

			m_shadow_map[i]->Create_DSV(depthStencilDesc);

			SRVdesc srv_desc = {};
			srv_desc.format = ResourceFormat::rf_r32_float;
			srv_desc.dimension = SRVdesc::SRVdimensionType::srv_dt_texture2d;
			srv_desc.texture2d.most_detailed_mip = 0;
			srv_desc.texture2d.mip_levels = 1;
			srv_desc.texture2d.res_min_lod_clamp = 0.0f;
			m_shadow_map[i]->Create_SRV(srv_desc);

			// FB
			std::vector<IGpuResource*> rts;
			gFrontend->CreateFrameBuffer(rts, m_shadow_map[i].get(), ITechniques::TecnhinueType::tt_shadow_map);
		}

		m_dirty &= (~df_init);
	}
}

void Sun::Update(float dt)
{
	// update view/proj for sun
	if (1 || m_dirty & df_upd) {
		if (std::shared_ptr<Level> level = gFrontend->GetLevel().lock()) {
			const LevelLight &sun_params = level->GetSunParams();

			DirectX::XMFLOAT3 pos = sun_params.dir;
			pos.x *= -1;
			pos.y *= -1;
			pos.z *= -1;

			if (std::shared_ptr<FreeCamera> camera = level->GetCamera().lock()) {
				const DirectX::XMFLOAT3 &cam_pos = camera->GetPosition();
				const float sun_dist = 120;
				pos.x = cam_pos.x + (pos.x * sun_dist);
				pos.y = cam_pos.y + (pos.y * sun_dist);
				pos.z = cam_pos.z + (pos.z * sun_dist);

				DirectX::XMVECTOR sun_dir = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&sun_params.dir));

				DirectX::XMFLOAT3 up3(0, 1,0);
				DirectX::XMVECTOR up = DirectX::XMLoadFloat3(&up3);
				DirectX::XMVECTOR dot_vec = DirectX::XMVector3Dot(sun_dir, up);
				DirectX::XMFLOAT3 dot_vec_f3;
				DirectX::XMStoreFloat3(&dot_vec_f3, dot_vec);
				up = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(up, DirectX::XMVectorScale(sun_dir, dot_vec_f3.x)));
				
				DirectX::XMFLOAT3 up_vec_f3;
				DirectX::XMStoreFloat3(&up_vec_f3, up);
				if (up_vec_f3.y < 0) {
					up_vec_f3.y *= -1;
					up = DirectX::XMLoadFloat3(&up_vec_f3);
				}

				// V
				DirectX::XMMATRIX sun_v = DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&pos), sun_dir, up);
				DirectX::XMStoreFloat4x4(&m_sun_view, sun_v);

				// P
				DirectX::XMMATRIX sun_p = DirectX::XMMatrixOrthographicLH(((float)gFrontend->GetWidth() / (float)gFrontend->GetHeight()) * 100, 100, 10, 500);
				DirectX::XMStoreFloat4x4(&m_sun_projection, sun_p);
			}
		}

		m_dirty &= (~df_upd);
	}
}

void Sun::SetupShadowMap(ICommandList* command_list)
{
	m_current_id = (m_current_id+1) % rt_num;
	Initialize(command_list);

	command_list->ResourceBarrier(*(m_shadow_map[m_current_id]), ResourceState::rs_resource_state_depth_write);
	command_list->ClearDepthStencilView(m_shadow_map[m_current_id].get(), ClearFlagsDsv::cfdsv_depth, 1.0f, 0, 0, nullptr);
	std::vector<IGpuResource*> rtvs;
	command_list->SetRenderTargets(rtvs, m_shadow_map[m_current_id].get());

	const ITechniques::Technique* tech = gFrontend->GetTechniqueById(ITechniques::tt_shadow_map);
	ICommandQueue* queue = command_list->GetQueue();
	if (command_list->GetPSO() != ITechniques::tt_shadow_map) {
		command_list->SetPSO(ITechniques::tt_shadow_map);
	}
	if (command_list->GetRootSign() != tech->root_signature) {
		command_list->SetRootSign(tech->root_signature);
		queue->GetGpuHeap().CacheRootSignature(gFrontend->GetRootSignById(tech->root_signature));
	}

	gFrontend->SetMatrix4Constant(Constants::cSunV, m_sun_view);
	gFrontend->SetMatrix4Constant(Constants::cSunP, m_sun_projection);
	gFrontend->CommitCB(command_list, cb_scene);
}

Sun::Sun() 
{
	m_dirty |= (df_upd | df_init);
}

