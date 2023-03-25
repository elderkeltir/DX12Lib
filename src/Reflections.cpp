#include "Reflections.h"
#include "DXAppImplementation.h"


extern DXAppImplementation* gD3DApp;

void Reflections::Initialize()
{
	if (m_dirty & df_init) {
		const uint32_t width = gD3DApp->GetWidth();
		const uint32_t height = gD3DApp->GetHeight();

		m_reflection_map = std::make_unique<GpuResource[]>(rt_num);

		for (uint32_t i = 0; i < rt_num; i++) {
			auto& res = m_reflection_map[i];
			

			D3D12_RESOURCE_FLAGS res_flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			D3D12_RESOURCE_STATES res_state = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

			CD3DX12_RESOURCE_DESC res_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, width, height, 1, 0, 1, 0, res_flags);
			res.CreateTexture(HeapBuffer::BufferType::bt_default, res_desc, res_state, nullptr, std::optional<std::wstring>().value_or(L"reflection_map").append(std::to_wstring(i).append(L"-")).append(std::to_wstring(i)));

			D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
			srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srv_desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srv_desc.Texture2D.MostDetailedMip = 0;
			srv_desc.Texture2D.MipLevels = 1;
			srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
			res.Create_SRV(srv_desc);

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = 0;
			res.Create_UAV(uavDesc);
		}

		m_dirty &= (~df_init);
	}
}

void Reflections::GenerateReflections(CommandList& command_list)
{

}
