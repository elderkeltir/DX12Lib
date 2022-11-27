#pragma once

#include <memory>
#include <DirectXMath.h>
#include <directx/d3dx12.h>

using Microsoft::WRL::ComPtr;

class RenderQuad;
class GpuResource;

class SSAO {
public:
	SSAO();
	void Initialize();
	RenderQuad* GetRenderQuad() { return m_ssao_quad.get(); }
	GpuResource* GetRandomVals() { return m_ssao_quad_random_vals.get(); }
	void GenerateSSAO(ComPtr<ID3D12GraphicsCommandList6>& command_list);
	void SyncGpuData(ComPtr<ID3D12GraphicsCommandList6>& command_list);

private:
	void GenerateRandomValuesTex(ComPtr<ID3D12GraphicsCommandList6>& command_list);
	void UpdateSsaoCB(bool m_dirty, UINT k_size = 64, float r = 0.5f, float bs = 0.025f, float noise_size = 256.f);
	struct SsaoConstants
	{
		DirectX::XMFLOAT4   OffsetVectors[64];
		uint32_t kernelSize = 64;
		float radius = 0.5f;
		float bias = 0.025f;
		float noise_dim = 256.f;

		void BuildOffsetVectors();
	};

	std::unique_ptr<RenderQuad> m_ssao_quad;
	std::unique_ptr<GpuResource> m_ssao_quad_random_vals;
	std::unique_ptr<GpuResource> m_ssao_cb;
	std::unique_ptr<SsaoConstants> m_cbuffer_cpu;

	bool m_dirty{ true };
};