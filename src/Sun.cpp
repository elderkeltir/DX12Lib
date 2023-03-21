#include "Sun.h"
#include "Level.h"
#include "GpuResource.h"
#include "GfxCommandQueue.h"
#include "DXAppImplementation.h"
#include "FreeCamera.h"
#include "ResourceDescriptor.h"

extern DXAppImplementation* gD3DApp;

void Sun::Initialize(CommandList& command_list)
{
	if (m_dirty & df_init) {
		const float width = (float)gD3DApp->GetWidth();
		const float height = (float)gD3DApp->GetHeight();

		m_sun_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
		m_sun_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));

		m_shadow_map = std::make_unique<GpuResource[]>(rt_num);

		for (uint32_t i = 0; i < rt_num; i++) {

			D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
			depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
			depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
			depthOptimizedClearValue.DepthStencil.Stencil = 0;
			CD3DX12_RESOURCE_DESC res_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, (uint64_t)width, (uint32_t)height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

			m_shadow_map[i].CreateTexture(HeapBuffer::BufferType::bt_default, res_desc, D3D12_RESOURCE_STATE_DEPTH_READ, &depthOptimizedClearValue, L"sun_shadow_map");

			D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
			depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
			depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

			m_shadow_map[i].Create_DSV(depthStencilDesc);

			D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
			srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
			srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srv_desc.Texture2D.MostDetailedMip = 0;
			srv_desc.Texture2D.MipLevels = 1;
			srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
			m_shadow_map[i].Create_SRV(srv_desc);
		}

		m_dirty &= (~df_init);
	}
}

void Sun::Update(float dt)
{
	// update view/proj for sun
	if (1 || m_dirty & df_upd) {
		if (std::shared_ptr<Level> level = gD3DApp->GetLevel().lock()) {
			const LevelLight &sun_params = level->GetSunParams();

			DirectX::XMFLOAT3 pos = sun_params.dir;
			pos.x *= -1;
			pos.y *= -1;
			pos.z *= -1;

			if (std::shared_ptr<FreeCamera> camera = level->GetCamera().lock()) {
				const DirectX::XMFLOAT3 &cam_pos = camera->GetPosition();
				const float sun_dist = 20;
				pos.x = cam_pos.x + (pos.x * sun_dist);
				pos.y = cam_pos.y + (pos.y * sun_dist);
				pos.z = cam_pos.z + (pos.z * sun_dist);

				DirectX::XMVECTOR sun_dir = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&sun_params.dir));

				DirectX::XMFLOAT3 up3(0, 1,0);
				DirectX::XMVECTOR up = DirectX::XMLoadFloat3(&up3);

				up = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(up, DirectX::XMVectorScale(sun_dir, DirectX::XMVector3Dot(sun_dir, up).m128_f32[0])));

				if (up.m128_f32[1] < 0) {
					up.m128_f32[1] *= -1;
				}

				// V
				DirectX::XMMATRIX sun_v = DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&pos), sun_dir, up);
				DirectX::XMStoreFloat4x4(&m_sun_view, sun_v);

				// P
				DirectX::XMMATRIX sun_p = DirectX::XMMatrixOrthographicLH(((float)gD3DApp->GetWidth() / (float)gD3DApp->GetHeight()) * 100, 100, 0, 100);
				DirectX::XMStoreFloat4x4(&m_sun_projection, sun_p);
			}
		}

		m_dirty &= (~df_upd);
	}
}

void Sun::SetupShadowMap(CommandList& command_list)
{
	m_current_id = (m_current_id+1) % rt_num;
	Initialize(command_list);

	command_list.GetQueue()->ResourceBarrier(m_shadow_map[m_current_id], D3D12_RESOURCE_STATE_DEPTH_WRITE);
	
	// set dsv and viewport
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	if (std::shared_ptr<ResourceDescriptor> depth_view = m_shadow_map[m_current_id].GetDSV().lock()) {
		dsvHandle = depth_view->GetCPUhandle();
		command_list.ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}
	else {
		assert(false);
	}

	command_list.OMSetRenderTargets(0, NULL, FALSE, &dsvHandle);

	const Techniques::Technique* tech = gD3DApp->GetTechniqueById(Techniques::tt_shadow_map);
	GfxCommandQueue* queue = command_list.GetQueue();
	if (queue->GetPSO() != Techniques::tt_shadow_map) {
		queue->SetPSO(Techniques::tt_shadow_map);
	}
	if (queue->GetRootSign() != tech->root_signature) {
		queue->SetRootSign(tech->root_signature);
		queue->GetGpuHeap().CacheRootSignature(gD3DApp->GetRootSignById(tech->root_signature));
	}

	gD3DApp->SetMatrix4Constant(Constants::cSunV, m_sun_view);
	gD3DApp->SetMatrix4Constant(Constants::cSunP, m_sun_projection);
	gD3DApp->CommitCB(command_list, cb_scene);

	// set constants needed to render shadow map eg sun proj & view mx

	// render
	//if (std::shared_ptr<Level> level = gD3DApp->GetLevel().lock()) {
	//	level->RenderShadowMap(command_list);
	//}

	// restore viewport
}

Sun::Sun() 
{
	m_dirty |= (df_upd | df_init);
}

