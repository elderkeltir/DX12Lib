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
	m_ssao_quad(std::make_unique<RenderQuad>()),
	m_ssao_quad_random_vals(std::make_unique<GpuResource>()),
	m_ssao_cb(std::make_unique<GpuResource>()),
	m_cbuffer_cpu(std::make_unique<SsaoConstants>()),
	m_blur_cbuffer(std::make_unique<BlurConstants>())
{

}

void SSAO::Initialize()
{
	m_ssao_quad->Initialize();

	uint32_t cb_size = calc_cb_size(sizeof(SsaoConstants));
	m_ssao_cb->CreateBuffer(HeapBuffer::BufferType::bt_default, cb_size, HeapBuffer::UseFlag::uf_none, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, std::wstring(L"ssao_const_buffer"));
	D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
	desc.SizeInBytes = cb_size;
	m_ssao_cb->Create_CBV(desc);
}

void SSAO::GenerateSSAO(ComPtr<ID3D12GraphicsCommandList6>& command_list)
{
	UpdateSsaoCB(m_dirty);
	GenerateRandomValuesTex(command_list);
	ConstantBufferManager::SyncCpuDataToCB(command_list, m_ssao_cb.get(), m_cbuffer_cpu.get(), sizeof(SsaoConstants), bi_ssao_cb);
}

void SSAO::BindBluerConstants(ComPtr<ID3D12GraphicsCommandList6>& command_list, uint32_t pass_type)
{
	m_blur_cbuffer->pass_type = pass_type;
	command_list->SetComputeRoot32BitConstants(bo_ssao_blur_constants, sizeof(BlurConstants) / 4, m_blur_cbuffer.get(), 0);
}

void SSAO::GenerateRandomValuesTex(ComPtr<ID3D12GraphicsCommandList6>& command_list)
{
	if (!m_dirty) {
		return;
	}
	const uint32_t noise_dim = 256;
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
				rand_fp_unorm(),
				0
			);
			DirectX::XMVECTOR v = DirectX::XMLoadFloat4(&fl4);

			DirectX::PackedVector::XMStoreColor(&(initData[i * noise_dim + j]), v);
		}
	}

	// upload data to texture
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = noise_dim * sizeof(DirectX::PackedVector::XMCOLOR);
	subResourceData.SlicePitch = subResourceData.RowPitch * noise_dim;
	m_ssao_quad_random_vals->LoadBuffer(command_list, 0, 1, &subResourceData);

	if (std::shared_ptr<GfxCommandQueue> queue = gD3DApp->GetGfxQueue().lock()) {
		queue->ResourceBarrier(*m_ssao_quad_random_vals.get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	m_dirty = false;
}


void SSAO::UpdateSsaoCB(bool m_dirty, UINT k_size, float r, float bs, float noise_size)
{
	if (m_dirty)
		m_cbuffer_cpu->BuildOffsetVectors();

	if (m_dirty)
		m_blur_cbuffer->ComputeWeights();

	m_cbuffer_cpu->kernelSize = k_size;
	m_cbuffer_cpu->radius = r;
	m_cbuffer_cpu->bias = bs;
	m_cbuffer_cpu->noise_dim = noise_size;
}

void SSAO::SsaoConstants::BuildOffsetVectors()
{
	// Start with 14 uniformly distributed vectors.  We choose the 8 corners of the cube
// and the 6 center points along each cube face.  We always alternate the points on 
// opposites sides of the cubes.  This way we still get the vectors spread out even
// if we choose to use less than 14 samples.

// 8 cube corners
	OffsetVectors[0] = DirectX::XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
	OffsetVectors[1] = DirectX::XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);

	OffsetVectors[2] = DirectX::XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
	OffsetVectors[3] = DirectX::XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);

	OffsetVectors[4] = DirectX::XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
	OffsetVectors[5] = DirectX::XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);

	OffsetVectors[6] = DirectX::XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
	OffsetVectors[7] = DirectX::XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);

	// 6 centers of cube faces
	OffsetVectors[8] = DirectX::XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
	OffsetVectors[9] = DirectX::XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);

	OffsetVectors[10] = DirectX::XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	OffsetVectors[11] = DirectX::XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);

	OffsetVectors[12] = DirectX::XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
	OffsetVectors[13] = DirectX::XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

	for (int i = 0; i < 14; ++i)
	{
		// Create random lengths in [0.25, 1.0].
		float s = rand_fp(0.25f, 1.0f);
		assert(s < 1.1 && s > 0.24);

		DirectX::XMVECTOR v = DirectX::XMVectorScale(DirectX::XMVector4Normalize(XMLoadFloat4(&OffsetVectors[i])), s);

		DirectX::XMStoreFloat4(&OffsetVectors[i], v);
	}
	//for (unsigned int i = 0; i < 64; ++i)
	//{
	//	DirectX::XMFLOAT4 sample(
	//		rand_fp_unorm() * 2.0f - 1.0f,
	//		rand_fp_unorm() * 2.0f - 1.0f,
	//		rand_fp_unorm(),
	//		0
	//	);
	//	float scale = (float)i / 64.0f;
	//	scale = lerp(0.1f, 1.0f, scale * scale);
	//	DirectX::XMVECTOR v = DirectX::XMVectorScale(DirectX::XMVector4Normalize(DirectX::XMLoadFloat4(&sample)), scale);
	//	
	//	DirectX::XMStoreFloat4(&OffsetVectors[i], v);
	//}
}

void SSAO::BlurConstants::ComputeWeights(float sigma)
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
