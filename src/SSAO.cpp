#include "SSAO.h"
#include <DirectXPackedVector.h>
#include <DirectXMath.h>
#include "RenderQuad.h"
#include "IGpuResource.h"
#include "RenderHelper.h"
#include "Level.h"
#include "FreeCamera.h"
#include "ICommandList.h"
#include "ConstantBufferManager.h"


SSAO::SSAO() :
	m_ssao_quad_random_vals(CreateGpuResource()),
	m_ssao_cb(CreateGpuResource()),
	m_cbuffer_cpu(std::make_unique<SsaoConstants>())
{

}

void SSAO::Initialize(uint32_t width, uint32_t height, std::optional<std::wstring> dbg_name)
{
	if (m_dirty & df_init) {
		for (uint32_t i = 0; i < 3; i++) {

			auto& res = m_ssao_resurces[i];
			res.reset(CreateGpuResource());

			ResourceState res_state = ResourceState::rs_resource_state_non_pixel_shader_resource;
			if (i == 2) {
				res_state = ResourceState::rs_resource_state_pixel_shader_resource;
			}

			ResourceDesc res_desc = ResourceDesc::tex_2d(ResourceFormat::rf_r8_unorm, width, height, 1, 0, 1, 0, ResourceDesc::ResourceFlags::rf_allow_unordered_access);
			res->CreateTexture(HeapType::ht_default, res_desc, res_state, nullptr, dbg_name.value_or(L"quad_tex_").append(std::to_wstring(i).append(L"-")).append(std::to_wstring(i)));

			SRVdesc srv_desc = {};
			srv_desc.format = ResourceFormat::rf_r8_unorm;
			srv_desc.dimension = SRVdesc::SRVdimensionType::srv_dt_texture2d;
			srv_desc.texture2d.most_detailed_mip = 0;
			srv_desc.texture2d.mip_levels = 1;
			srv_desc.texture2d.res_min_lod_clamp = 0.0f;
			res->Create_SRV(srv_desc);

			UAVdesc uavDesc = {};
			uavDesc.format = ResourceFormat::rf_r8_unorm;
			uavDesc.dimension = UAVdesc::UAVdimensionType::uav_dt_texture2d;
			uavDesc.texture2d.mip_slice = 0;
			res->Create_UAV(uavDesc);
		}

		uint32_t cb_size = calc_cb_size(sizeof(SsaoConstants));
		m_ssao_cb->CreateBuffer(HeapType::ht_default, cb_size, ResourceState::rs_resource_state_vertex_and_constant_buffer, std::wstring(L"ssao_const_buffer"));
		CBVdesc desc;
		desc.size_in_bytes = cb_size;
		m_ssao_cb->Create_CBV(desc);

		m_dirty &= ~df_init;
	}
}

void SSAO::GenerateSSAO(ICommandList* command_list, bool gfx)
{
	UpdateSsaoCB();
	GenerateRandomValuesTex(command_list);
	ConstantBufferManager::SyncCpuDataToCB(command_list, m_ssao_cb.get(), m_cbuffer_cpu.get(), sizeof(SsaoConstants), bi_ssao_cb, gfx);
}

void SSAO::GenerateRandomValuesTex(ICommandList* command_list)
{
	if (m_dirty & df_generate) {
		const uint32_t noise_dim = 4;

		ResourceDesc res_desc = ResourceDesc::tex_2d(ResourceFormat::rf_r8g8b8a8_unorm, noise_dim, noise_dim, 1, 0, 1, 0, ResourceDesc::ResourceFlags::rf_none);
		m_ssao_quad_random_vals->CreateTexture(HeapType::ht_default, res_desc, ResourceState::rs_resource_state_copy_dest, nullptr, L"m_ssao_quad_random_vals");

		SRVdesc srv_desc = {};
		srv_desc.format = ResourceFormat::rf_r8g8b8a8_unorm;
		srv_desc.dimension = SRVdesc::SRVdimensionType::srv_dt_texture2d;
		srv_desc.texture2d.most_detailed_mip = 0;
		srv_desc.texture2d.mip_levels = 1;
		srv_desc.texture2d.res_min_lod_clamp = 0.0f;
		m_ssao_quad_random_vals->Create_SRV(srv_desc);

		// generate random values
		DirectX::PackedVector::XMCOLOR initData[noise_dim * noise_dim];
		for (int i = 0; i < noise_dim; ++i)
		{
			for (int j = 0; j < noise_dim; ++j)
			{
				DirectX::XMFLOAT4 fl4(
					rand_fp_unorm(),
					rand_fp_unorm(),
					0,
					0
				);
				DirectX::XMVECTOR v = DirectX::XMVector4Normalize(DirectX::XMLoadFloat4(&fl4));

				DirectX::PackedVector::XMStoreColor(&(initData[i * noise_dim + j]), v);
			}
		}

		// upload data to texture
		SubresourceData subResourceData = {};
		subResourceData.data = initData;
		subResourceData.row_pitch = noise_dim * sizeof(DirectX::PackedVector::XMCOLOR);
		subResourceData.slice_pitch = subResourceData.row_pitch * noise_dim;
		m_ssao_quad_random_vals->LoadBuffer(command_list, 0, 1, &subResourceData);

		command_list->ResourceBarrier(*m_ssao_quad_random_vals.get(), ResourceState::rs_resource_state_non_pixel_shader_resource);

		m_dirty &= ~df_generate;
	}
}


void SSAO::UpdateSsaoCB(uint32_t k_size, float r, float bs, uint32_t noise_size)
{
	if (m_dirty & df_generate) {
		m_cbuffer_cpu->BuildOffsetVectors();
		m_cbuffer_cpu->ComputeWeights();
	}

	m_cbuffer_cpu->kernelSize = k_size;
	m_cbuffer_cpu->radius = r;
	m_cbuffer_cpu->bias = bs;
	m_cbuffer_cpu->noise_dim = noise_size;
}

void SSAO::SsaoConstants::BuildOffsetVectors()
{
	for (uint32_t i = 0; i < kernelSize; ++i)
	{
		DirectX::XMFLOAT4 p (
			rand_fp(-1.f,1.f),
			rand_fp(-1.f, 1.f),
			rand_fp(0, 1.f) * 3,
			0
		);
		DirectX::XMVECTOR v = DirectX::XMVector4Normalize(DirectX::XMLoadFloat4(&p));
		DirectX::XMVectorScale(v, rand_fp(0, 1.f));
		float scale = float(i) / float(kernelSize);
		scale = lerp(0.1f, 1.0f, scale * scale);
		DirectX::XMVectorScale(v, scale);

		DirectX::XMStoreFloat4(&OffsetVectors[i], v);
	}
}

void SSAO::SsaoConstants::ComputeWeights(float sigma)
{
	float twoSigma2 = 2.0f * sigma * sigma;
	int blurRadius = (int)ceil(2.0f * sigma);
	float weightSum = 0.0f;

	for (int i = -blurRadius; i <= blurRadius; ++i)
	{
		float x = (float)i;
		weights[i + blurRadius] = expf(-x * x / twoSigma2);
		weightSum += weights[i + blurRadius];
	}

	for (int i = 0; i < 11; ++i)
	{
		weights[i] /= weightSum;
	}
	
	pass_type = 1;
}
