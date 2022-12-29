#pragma once

#include <memory>
#include <directx/d3dx12.h>

using Microsoft::WRL::ComPtr;

class GpuResource;
class GfxCommandQueue;

class Sun {
public:
	void Initialize(ComPtr<ID3D12GraphicsCommandList6>& command_list, float width, float height);
	void Generate(ComPtr<ID3D12GraphicsCommandList6>& command_list, std::shared_ptr<GfxCommandQueue>& queue);
	GpuResource* GetShadow_map() { return m_shadow_map.get(); }

private:
	std::unique_ptr<GpuResource> m_shadow_map;
};