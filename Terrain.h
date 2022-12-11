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
	void Load(const std::wstring &hm_name);
	void Render(ComPtr<ID3D12GraphicsCommandList6>& command_list);
private:
	
	RenderModel* m_model;
	std::unique_ptr<GpuResource> m_terrain_hm;
};