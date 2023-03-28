#pragma once

#include <memory>
#include <directx/d3dx12.h>
#include <DirectXMath.h>

using Microsoft::WRL::ComPtr;

class GpuResource;
class CommandQueue;
class CommandList;

class Sun {
public:
	void Initialize(CommandList& command_list);
	void Update(float dt);
	void SetupShadowMap(CommandList& command_list);
	GpuResource& GetShadowMap() { return m_shadow_map[m_current_id]; }

	Sun();

private:
	static constexpr uint32_t rt_num = 2;

	std::unique_ptr<GpuResource[]> m_shadow_map;
	DirectX::XMFLOAT4X4 m_sun_view;
	DirectX::XMFLOAT4X4 m_sun_projection;
	CD3DX12_VIEWPORT m_sun_viewport;
	CD3DX12_RECT m_sun_scissorRect;

	uint32_t m_current_id{ 0 };
	enum dirty_flags { df_upd = 1, df_init = 2};
	uint8_t m_dirty{ 0 };
};