#include "SSAO.h"
#include <DirectXPackedVector.h>
#include <DirectXMath.h>
#include "RenderQuad.h"
#include "GpuResource.h"
#include "DXHelper.h"
#include "Level.h"
#include "FreeCamera.h"
#include "GfxCommandQueue.h"
#include "DXAppImplementation.h"

extern DXAppImplementation* gD3DApp;


SSAO::SSAO() :
	m_ssao_quad_random_vals(std::make_unique<GpuResource>()),
	m_ssao_cb(std::make_unique<GpuResource>()),
	m_cbuffer_cpu(std::make_unique<SsaoConstants>())
{

}

void SSAO::Initialize(uint32_t width, uint32_t height, std::optional<std::wstring> dbg_name)
{
	if (m_dirty & df_init) {
		for (uint32_t i = 0; i < 3; i++) {
			auto& res = m_ssao_resurces[i];
			res.swap(std::make_unique<GpuResource>());

			D3D12_RESOURCE_FLAGS res_flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			D3D12_RESOURCE_STATES res_state = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			if (i == 2) {
				res_state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			}

			CD3DX12_RESOURCE_DESC res_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8_UNORM, width, height, 1, 0, 1, 0, res_flags);
			res->CreateTexture(HeapBuffer::BufferType::bt_default, res_desc, res_state, nullptr, dbg_name.value_or(L"quad_tex_").append(std::to_wstring(i).append(L"-")).append(std::to_wstring(i)));

			D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
			srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srv_desc.Format = DXGI_FORMAT_R8_UNORM;
			srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srv_desc.Texture2D.MostDetailedMip = 0;
			srv_desc.Texture2D.MipLevels = 1;
			srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
			res->Create_SRV(srv_desc);

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_R8_UNORM;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = 0;
			res->Create_UAV(uavDesc);
		}

		uint32_t cb_size = calc_cb_size(sizeof(SsaoConstants));
		m_ssao_cb->CreateBuffer(HeapBuffer::BufferType::bt_default, cb_size, HeapBuffer::UseFlag::uf_none, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, std::wstring(L"ssao_const_buffer"));
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
		desc.SizeInBytes = cb_size;
		m_ssao_cb->Create_CBV(desc);

		m_dirty &= ~df_init;
	}
}

void SSAO::GenerateSSAO(ComPtr<ID3D12GraphicsCommandList6>& command_list, bool gfx)
{
	UpdateSsaoCB();
	GenerateRandomValuesTex(command_list);
	ConstantBufferManager::SyncCpuDataToCB(command_list, m_ssao_cb.get(), m_cbuffer_cpu.get(), sizeof(SsaoConstants), bi_ssao_cb, gfx);
}

void SSAO::GenerateRandomValuesTex(ComPtr<ID3D12GraphicsCommandList6>& command_list)
{
	if (m_dirty & df_generate) {
		const uint32_t noise_dim = 4;
		CD3DX12_RESOURCE_DESC res_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, noise_dim, noise_dim, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_NONE);
		m_ssao_quad_random_vals->CreateTexture(HeapBuffer::BufferType::bt_default, res_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, L"m_ssao_quad_random_vals");

		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MostDetailedMip = 0;
		srv_desc.Texture2D.MipLevels = 1;
		srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
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
		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = initData;
		subResourceData.RowPitch = noise_dim * sizeof(DirectX::PackedVector::XMCOLOR);
		subResourceData.SlicePitch = subResourceData.RowPitch * noise_dim;
		m_ssao_quad_random_vals->LoadBuffer(command_list, 0, 1, &subResourceData);

		if (std::shared_ptr<GfxCommandQueue> queue = gD3DApp->GetComputeQueue().lock()) {
			queue->ResourceBarrier(*m_ssao_quad_random_vals.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}

		m_dirty &= ~df_generate;
	}
}


void SSAO::UpdateSsaoCB(UINT k_size, float r, float bs, uint32_t noise_size)
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
