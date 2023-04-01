#pragma once

#include <memory>
#include <array>
#include <DirectXMath.h>

class IGpuResource;
class ICommandList;

class Sun {
public:
	void Initialize(ICommandList* command_list);
	void Update(float dt);
	void SetupShadowMap(ICommandList* command_list);
	IGpuResource& GetShadowMap() { return *(m_shadow_map[m_current_id]); }

	Sun();

private:
	static constexpr uint32_t rt_num = 2;

	std::array<std::unique_ptr<IGpuResource>, rt_num> m_shadow_map;
	DirectX::XMFLOAT4X4 m_sun_view;
	DirectX::XMFLOAT4X4 m_sun_projection;

	uint32_t m_current_id{ 0 };
	enum dirty_flags { df_upd = 1, df_init = 2};
	uint8_t m_dirty{ 0 };
};