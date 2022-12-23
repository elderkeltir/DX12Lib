#pragma once

#include <memory>
#include <array>
#include <optional>
#include <DirectXMath.h>
#include <directx/d3dx12.h>

using Microsoft::WRL::ComPtr;

class RenderQuad;
class GpuResource;

class SSAO {
public:
	SSAO();
	void Initialize(uint32_t width, uint32_t height, std::optional<std::wstring> dbg_name);
	GpuResource* GetSSAOres(uint32_t id) { assert(id < m_ssao_resurces.size()); return m_ssao_resurces[id].get(); }
	GpuResource* GetRandomVals() { return m_ssao_quad_random_vals.get(); }
	void GenerateSSAO(ComPtr<ID3D12GraphicsCommandList6>& command_list, bool gfx = true);

private:
	void GenerateRandomValuesTex(ComPtr<ID3D12GraphicsCommandList6>& command_list);
	void UpdateSsaoCB(bool m_dirty, UINT k_size = 14, float r = 0.5f, float bs = 0.05f, float noise_size = 256.f);
	struct SsaoConstants
	{
		DirectX::XMFLOAT4  OffsetVectors[14];
		uint32_t kernelSize = 14;
		float radius = 0.5f;
		float bias = 0.05f;
		float noise_dim = 256.f;

		void BuildOffsetVectors();

		// blur
		int32_t pass_type = 0; // 0 = hor, 1 = vert
		float weights[11];
		void ComputeWeights(float sigma = 2.5f);
	};

	std::array<std::unique_ptr<GpuResource>, 3> m_ssao_resurces;

	std::unique_ptr<GpuResource> m_ssao_quad_random_vals;
	std::unique_ptr<GpuResource> m_ssao_cb;
	std::unique_ptr<SsaoConstants> m_cbuffer_cpu;
	enum dirty_flag { df_init = 1, df_generate = 1 << 1 };
	uint8_t m_dirty{ uint8_t(-1) };
};