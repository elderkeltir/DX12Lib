#pragma once

#include <memory>
#include <string>
#include <wrl.h>
#include <directx/d3dx12.h>

using Microsoft::WRL::ComPtr;

class RenderModel;
class GpuResource;

class Terrain {
public:
	void Load(const std::wstring& hm_name);
	void Render(ComPtr<ID3D12GraphicsCommandList6>& command_list);
	uint32_t GetTerrainDim() const { return m_terrain_dim; }
	uint32_t GetTerrainTechId() const { return m_tech_id; }
private:
	const uint32_t m_terrain_dim = 512;
	const uint32_t m_tech_id = 7;
	RenderModel* m_model;
	std::unique_ptr<GpuResource> m_terrain_hm;
};