#include "Sun.h"

#include "GpuResource.h"
#include "GfxCommandQueue.h"
#include "DXAppImplementation.h"

extern DXAppImplementation* gD3DApp;

void Sun::Initialize(CommandList& command_list, float width, float height)
{
	m_shadow_map = std::make_unique<GpuResource>();

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;
	CD3DX12_RESOURCE_DESC res_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, (uint64_t)width, (uint32_t)height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	m_shadow_map->CreateTexture(HeapBuffer::BufferType::bt_default, res_desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthOptimizedClearValue, L"sun_shadow_map");

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	m_shadow_map->Create_DSV(depthStencilDesc);

	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
	srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MostDetailedMip = 0;
	srv_desc.Texture2D.MipLevels = 1;
	srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
	m_shadow_map->Create_SRV(srv_desc);
}

void Sun::Generate(CommandList& command_list, std::shared_ptr<GfxCommandQueue> & queue)
{
	// set ps
	const Techniques::Technique* tech = gD3DApp->GetTechniqueById(Techniques::tt_shadow_map);
	if (queue->GetPSO() != Techniques::tt_shadow_map) {
		queue->SetPSO(Techniques::tt_shadow_map);
	}
	if (queue->GetRootSign() != tech->root_signature) {
		queue->SetRootSign(tech->root_signature);
		queue->GetGpuHeap().CacheRootSignature(gD3DApp->GetRootSignById(tech->root_signature));
	}

	// set cvars(sun pos, dir)

	// render level entities + terrain
}

