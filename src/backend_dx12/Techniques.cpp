#include "Techniques.h"
#include "dx12_helper.h"
#include "ShaderManager.h"
#include <DirectXMath.h>
#include "defines.h"
#include "RootSignature.h"

#include <directx/d3dx12.h>
#include "DxBackend.h"
#include "DxDevice.h"

extern DxBackend* gBackend;

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers()
{
	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        0,
        16,
        D3D12_COMPARISON_FUNC_NEVER); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC depthMapSam(
		6, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,
		0,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE);

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp, 
        depthMapSam };
}

//TODO("Minor. Implement PSO composition and serialization later. maybe.")
// g-buffer color
static Techniques::TechniqueDx CreateTechnique_0(ComPtr<ID3D12Device2> &device, RootSignature& root_sign, std::optional<std::wstring> dbg_name = std::nullopt){
    Techniques::TechniqueDx tech;
    tech.vs = L"g_buffer_vs.hlsl";
    tech.ps = L"g_buffer_ps.hlsl";
    tech.vertex_type = 0;
    tech.root_signature = root_sign.GetRSId();

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT };
    rtvFormats.NumRenderTargets = 4;

    CD3DX12_SHADER_BYTECODE vs;
    CD3DX12_SHADER_BYTECODE ps;
    if (ShaderManager* shader_mgr = gBackend->GetShaderManager()){
        ShaderManager::ShaderBlob* vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
        ShaderManager::ShaderBlob* ps_blob = shader_mgr->Load(tech.ps, L"main", ShaderManager::ShaderType::st_pixel);
        vs = CD3DX12_SHADER_BYTECODE((const void*)vs_blob->data.data(), vs_blob->data.size());
        ps = CD3DX12_SHADER_BYTECODE((const void*)ps_blob->data.data(), ps_blob->data.size());
    }
    else{
        assert(false);
    }

    pipelineStateStream.pRootSignature = root_sign.GetRootSignature().Get();
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS = vs;
    pipelineStateStream.PS = ps;
    pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStream.RTVFormats = rtvFormats;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
        sizeof(PipelineStateStream), &pipelineStateStream
    };
    ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&tech.pipeline_state)));
    SetName(tech.pipeline_state, dbg_name.value_or(L"").append(L"_pso_0").c_str());

    return tech;
}
// g-buffer texture
static Techniques::TechniqueDx CreateTechnique_1(ComPtr<ID3D12Device2> &device, RootSignature& root_sign, std::optional<std::wstring> dbg_name = std::nullopt){
    Techniques::TechniqueDx tech;
    tech.vs = L"g_buffer_vs.hlsl";
    tech.ps = L"g_buffer_ps.hlsl";
    tech.vertex_type = 1;
    tech.root_signature = root_sign.GetRSId();

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT };
    rtvFormats.NumRenderTargets = 4;

    CD3DX12_SHADER_BYTECODE vs;
    CD3DX12_SHADER_BYTECODE ps;
    if (ShaderManager* shader_mgr = gBackend->GetShaderManager()){
        ShaderManager::ShaderBlob* vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
        ShaderManager::ShaderBlob* ps_blob = shader_mgr->Load(tech.ps, L"main", ShaderManager::ShaderType::st_pixel);
        vs = CD3DX12_SHADER_BYTECODE(vs_blob->data.data(), vs_blob->data.size());
        ps = CD3DX12_SHADER_BYTECODE(ps_blob->data.data(), ps_blob->data.size());
    }
    else{
        assert(false);
    }

    pipelineStateStream.pRootSignature = root_sign.GetRootSignature().Get();
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS = vs;
    pipelineStateStream.PS = ps;
    pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStream.RTVFormats = rtvFormats;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
        sizeof(PipelineStateStream), &pipelineStateStream
    };
    ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&tech.pipeline_state)));
    SetName(tech.pipeline_state, dbg_name.value_or(L"").append(L"_pso_1").c_str());
    return tech;
}
// post-processing
static Techniques::TechniqueDx CreateTechnique_2(ComPtr<ID3D12Device2> &device, RootSignature& root_sign, std::optional<std::wstring> dbg_name = std::nullopt){
    Techniques::TechniqueDx tech;
    tech.vs = L"quad_screen_vs.hlsl";
    tech.ps = L"post_processing_ps.hlsl";
    tech.vertex_type = 2;
    tech.root_signature = root_sign.GetRSId();

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    CD3DX12_SHADER_BYTECODE vs;
    CD3DX12_SHADER_BYTECODE ps;
    if (ShaderManager* shader_mgr = gBackend->GetShaderManager()){
        ShaderManager::ShaderBlob* vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
        ShaderManager::ShaderBlob* ps_blob = shader_mgr->Load(tech.ps, L"main", ShaderManager::ShaderType::st_pixel);
        vs = CD3DX12_SHADER_BYTECODE((const void*)vs_blob->data.data(), vs_blob->data.size());
        ps = CD3DX12_SHADER_BYTECODE((const void*)ps_blob->data.data(), ps_blob->data.size());
    }
    else{
        assert(false);
    }

    pipelineStateStream.pRootSignature = root_sign.GetRootSignature().Get();
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS = vs;
    pipelineStateStream.PS = ps;
    pipelineStateStream.RTVFormats = rtvFormats;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
        sizeof(PipelineStateStream), &pipelineStateStream
    };
    ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&tech.pipeline_state)));
    SetName(tech.pipeline_state, dbg_name.value_or(L"").append(L"_pso_2").c_str());

    return tech;
}
// def shading
static Techniques::TechniqueDx CreateTechnique_3(ComPtr<ID3D12Device2> &device, RootSignature& root_sign, std::optional<std::wstring> dbg_name = std::nullopt){
    Techniques::TechniqueDx tech;
    tech.vs = L"quad_screen_vs.hlsl";
    tech.ps = L"def_shading_ps.hlsl";
    tech.vertex_type = 2;
    tech.root_signature = root_sign.GetRSId();

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = { DXGI_FORMAT_R16G16B16A16_FLOAT };
    rtvFormats.NumRenderTargets = 1;

    CD3DX12_SHADER_BYTECODE vs;
    CD3DX12_SHADER_BYTECODE ps;
    if (ShaderManager* shader_mgr = gBackend->GetShaderManager()){
        ShaderManager::ShaderBlob* vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
        ShaderManager::ShaderBlob* ps_blob = shader_mgr->Load(tech.ps, L"main", ShaderManager::ShaderType::st_pixel);
        vs = CD3DX12_SHADER_BYTECODE((const void*)vs_blob->data.data(), vs_blob->data.size());
        ps = CD3DX12_SHADER_BYTECODE((const void*)ps_blob->data.data(), ps_blob->data.size());
    }
    else{
        assert(false);
    }

    pipelineStateStream.pRootSignature = root_sign.GetRootSignature().Get();
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS = vs;
    pipelineStateStream.PS = ps;
    pipelineStateStream.RTVFormats = rtvFormats;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
        sizeof(PipelineStateStream), &pipelineStateStream
    };
    ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&tech.pipeline_state)));
    SetName(tech.pipeline_state, dbg_name.value_or(L"").append(L"_pso_3").c_str());

    return tech;
}
// skybox
static Techniques::TechniqueDx CreateTechnique_4(ComPtr<ID3D12Device2> &device, RootSignature& root_sign, std::optional<std::wstring> dbg_name = std::nullopt){
    Techniques::TechniqueDx tech;
    tech.vs = L"skybox_vs.hlsl";
    tech.ps = L"skybox_ps.hlsl";
    tech.vertex_type = 3;
    tech.root_signature = root_sign.GetRSId();

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL ds_desc;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER raster_dec;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT };
    rtvFormats.NumRenderTargets = 4;

    CD3DX12_SHADER_BYTECODE vs;
    CD3DX12_SHADER_BYTECODE ps;
    if (ShaderManager* shader_mgr = gBackend->GetShaderManager()){
        ShaderManager::ShaderBlob* vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
        ShaderManager::ShaderBlob* ps_blob = shader_mgr->Load(tech.ps, L"main", ShaderManager::ShaderType::st_pixel);
        vs = CD3DX12_SHADER_BYTECODE((const void*)vs_blob->data.data(), vs_blob->data.size());
        ps = CD3DX12_SHADER_BYTECODE((const void*)ps_blob->data.data(), ps_blob->data.size());
    }
    else{
        assert(false);
    }
    
    CD3DX12_DEPTH_STENCIL_DESC dsd(CD3DX12_DEFAULT{});
    dsd.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    CD3DX12_RASTERIZER_DESC rd(CD3DX12_DEFAULT{});
    rd.CullMode = D3D12_CULL_MODE_NONE;

    pipelineStateStream.pRootSignature = root_sign.GetRootSignature().Get();
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS = vs;
    pipelineStateStream.PS = ps;
    pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStream.RTVFormats = rtvFormats;
    pipelineStateStream.ds_desc = dsd;
    pipelineStateStream.raster_dec = rd;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
        sizeof(PipelineStateStream), &pipelineStateStream
    };
    ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&tech.pipeline_state)));
    SetName(tech.pipeline_state, dbg_name.value_or(L"").append(L"_pso_4").c_str());

    return tech;
}
// ssao
static Techniques::TechniqueDx CreateTechnique_5(ComPtr<ID3D12Device2>& device, RootSignature& root_sign, std::optional<std::wstring> dbg_name = std::nullopt) {
	Techniques::TechniqueDx tech;
	tech.cs = L"ssao_cs.hlsl";
	tech.root_signature = root_sign.GetRSId();

	D3D12_COMPUTE_PIPELINE_STATE_DESC ssaoPSO = {};
    ssaoPSO.pRootSignature = root_sign.GetRootSignature().Get();
	if (ShaderManager* shader_mgr = gBackend->GetShaderManager()) {
		ShaderManager::ShaderBlob* cs_blob = shader_mgr->Load(tech.cs, L"main", ShaderManager::ShaderType::st_compute);
        ssaoPSO.CS = CD3DX12_SHADER_BYTECODE((const void*)cs_blob->data.data(), cs_blob->data.size());
	}
    ssaoPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	ThrowIfFailed(device->CreateComputePipelineState(&ssaoPSO, IID_PPV_ARGS(&tech.pipeline_state)));
    SetName(tech.pipeline_state, dbg_name.value_or(L"").append(L"_pso_5").c_str());

	return tech;
}
// gaussian blur
static Techniques::TechniqueDx CreateTechnique_6(ComPtr<ID3D12Device2>& device, RootSignature& root_sign, std::optional<std::wstring> dbg_name = std::nullopt) {
	Techniques::TechniqueDx tech;
	tech.cs = L"gaussian_blur_cs.hlsl";
	tech.root_signature = root_sign.GetRSId();

	D3D12_COMPUTE_PIPELINE_STATE_DESC blurPSO = {};
	blurPSO.pRootSignature = root_sign.GetRootSignature().Get();
    if (ShaderManager* shader_mgr = gBackend->GetShaderManager()) {
		ShaderManager::ShaderBlob* cs_blob = shader_mgr->Load(tech.cs, L"main", ShaderManager::ShaderType::st_compute);
        blurPSO.CS = CD3DX12_SHADER_BYTECODE((const void*)cs_blob->data.data(), cs_blob->data.size());
    }
	blurPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    ThrowIfFailed(device->CreateComputePipelineState(&blurPSO, IID_PPV_ARGS(&tech.pipeline_state)));
    SetName(tech.pipeline_state, dbg_name.value_or(L"").append(L"_pso_6").c_str());

    return tech;
}
// terrain
static Techniques::TechniqueDx CreateTechnique_7(ComPtr<ID3D12Device2>& device, RootSignature& root_sign, std::optional<std::wstring> dbg_name = std::nullopt) {
	Techniques::TechniqueDx tech;
	tech.vs = L"terrain_vs.hlsl";
	tech.ps = L"terrain_ps.hlsl";
	tech.root_signature = root_sign.GetRSId();

	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_VS VS;
		CD3DX12_PIPELINE_STATE_STREAM_PS PS;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL ds_desc;
		CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER raster_dec;
	} pipelineStateStream;

	D3D12_RT_FORMAT_ARRAY rtvFormats = { DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT };
	rtvFormats.NumRenderTargets = 4;

	CD3DX12_SHADER_BYTECODE vs;
	CD3DX12_SHADER_BYTECODE ps;
	if (ShaderManager* shader_mgr = gBackend->GetShaderManager()) {
		ShaderManager::ShaderBlob* vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
		ShaderManager::ShaderBlob* ps_blob = shader_mgr->Load(tech.ps, L"main", ShaderManager::ShaderType::st_pixel);
		vs = CD3DX12_SHADER_BYTECODE((const void*)vs_blob->data.data(), vs_blob->data.size());
		ps = CD3DX12_SHADER_BYTECODE((const void*)ps_blob->data.data(), ps_blob->data.size());
	}
	else {
		assert(false);
	}

	CD3DX12_DEPTH_STENCIL_DESC dsd(CD3DX12_DEFAULT{});
	dsd.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	CD3DX12_RASTERIZER_DESC rd(CD3DX12_DEFAULT{});
	rd.CullMode = D3D12_CULL_MODE_NONE;

	pipelineStateStream.pRootSignature = root_sign.GetRootSignature().Get();
	pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateStream.VS = vs;
	pipelineStateStream.PS = ps;
	pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateStream.RTVFormats = rtvFormats;
	pipelineStateStream.ds_desc = dsd;
	pipelineStateStream.raster_dec = rd;

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof(PipelineStateStream), &pipelineStateStream
	};
	ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&tech.pipeline_state)));
	SetName(tech.pipeline_state, dbg_name.value_or(L"").append(L"_pso_7").c_str());

	return tech;
}
// water
static Techniques::TechniqueDx CreateTechnique_8(ComPtr<ID3D12Device2>& device, RootSignature& root_sign, std::optional<std::wstring> dbg_name = std::nullopt) {
	Techniques::TechniqueDx tech;
	tech.vs = L"water_vs.hlsl";
	tech.ps = L"water_ps.hlsl";
	tech.root_signature = root_sign.GetRSId();

	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_VS VS;
		CD3DX12_PIPELINE_STATE_STREAM_PS PS;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL ds_desc;
		CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER raster_dec;
        CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC blend_desc;
	} pipelineStateStream;

	D3D12_RT_FORMAT_ARRAY rtvFormats = { DXGI_FORMAT_R16G16B16A16_FLOAT };
	rtvFormats.NumRenderTargets = 1;

	CD3DX12_SHADER_BYTECODE vs;
	CD3DX12_SHADER_BYTECODE ps;
	if (ShaderManager* shader_mgr = gBackend->GetShaderManager()) {
		ShaderManager::ShaderBlob* vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
		ShaderManager::ShaderBlob* ps_blob = shader_mgr->Load(tech.ps, L"main", ShaderManager::ShaderType::st_pixel);
		vs = CD3DX12_SHADER_BYTECODE((const void*)vs_blob->data.data(), vs_blob->data.size());
		ps = CD3DX12_SHADER_BYTECODE((const void*)ps_blob->data.data(), ps_blob->data.size());
	}
	else {
		assert(false);
	}

	CD3DX12_DEPTH_STENCIL_DESC dsd(CD3DX12_DEFAULT{});
	dsd.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	CD3DX12_RASTERIZER_DESC rd(CD3DX12_DEFAULT{});
	rd.CullMode = D3D12_CULL_MODE_NONE;

    CD3DX12_BLEND_DESC bs(CD3DX12_DEFAULT{});
	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    bs.RenderTarget[0] = transparencyBlendDesc;

	pipelineStateStream.pRootSignature = root_sign.GetRootSignature().Get();
	pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateStream.VS = vs;
	pipelineStateStream.PS = ps;
	pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateStream.RTVFormats = rtvFormats;
	pipelineStateStream.ds_desc = dsd;
	pipelineStateStream.raster_dec = rd;
    pipelineStateStream.blend_desc = bs;

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof(PipelineStateStream), &pipelineStateStream
	};
	ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&tech.pipeline_state)));
	SetName(tech.pipeline_state, dbg_name.value_or(L"").append(L"_pso_8").c_str());

	return tech;
}

// shadow_map
static Techniques::TechniqueDx CreateTechnique_9(ComPtr<ID3D12Device2>& device, RootSignature& root_sign, std::optional<std::wstring> dbg_name = std::nullopt) {
	Techniques::TechniqueDx tech;
	tech.vs = L"shadow_vs.hlsl";
	tech.ps = L"";
	tech.root_signature = root_sign.GetRSId();

	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_VS VS;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL ds_desc;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER raster_dec;
	} pipelineStateStream;

	CD3DX12_SHADER_BYTECODE vs;
	if (ShaderManager* shader_mgr = gBackend->GetShaderManager()) {
		ShaderManager::ShaderBlob* vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
		vs = CD3DX12_SHADER_BYTECODE((const void*)vs_blob->data.data(), vs_blob->data.size());
	}
	else {
		assert(false);
	}

    CD3DX12_RASTERIZER_DESC rd(CD3DX12_DEFAULT{});
    rd.CullMode = D3D12_CULL_MODE_NONE;
    rd.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rd.DepthBiasClamp = 0.f;
    rd.SlopeScaledDepthBias = 1.f;

	CD3DX12_DEPTH_STENCIL_DESC dsd(CD3DX12_DEFAULT{});
	dsd.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	pipelineStateStream.pRootSignature = root_sign.GetRootSignature().Get();
	pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateStream.VS = vs;
	pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateStream.ds_desc = dsd;
    pipelineStateStream.raster_dec = rd;

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof(PipelineStateStream), &pipelineStateStream
	};
	ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&tech.pipeline_state)));
	SetName(tech.pipeline_state, dbg_name.value_or(L"").append(L"_pso_9").c_str());

	return tech;
}
// refl
static Techniques::TechniqueDx CreateTechnique_10(ComPtr<ID3D12Device2>& device, RootSignature& root_sign, std::optional<std::wstring> dbg_name = std::nullopt) {
    Techniques::TechniqueDx tech;
    tech.cs = L"refl_cs.hlsl";
    tech.root_signature = root_sign.GetRSId();

    D3D12_COMPUTE_PIPELINE_STATE_DESC reflPSO = {};
    reflPSO.pRootSignature = root_sign.GetRootSignature().Get();
    if (ShaderManager* shader_mgr = gBackend->GetShaderManager()) {
        ShaderManager::ShaderBlob* cs_blob = shader_mgr->Load(tech.cs, L"main", ShaderManager::ShaderType::st_compute);
        reflPSO.CS = CD3DX12_SHADER_BYTECODE((const void*)cs_blob->data.data(), cs_blob->data.size());
    }
    reflPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    ThrowIfFailed(device->CreateComputePipelineState(&reflPSO, IID_PPV_ARGS(&tech.pipeline_state)));
    SetName(tech.pipeline_state, dbg_name.value_or(L"").append(L"_pso_5").c_str());

    return tech;
}

// root sign for g-buffer
void Techniques::CreateRootSignature_0(ComPtr<ID3D12Device2> &device, RootSignature* root_sign, std::optional<std::wstring> dbg_name){
    // Create a root signature.
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)));

    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    // A single 32-bit constant root parameter that is used by the vertex shader.
    CD3DX12_DESCRIPTOR_RANGE1 &tex_table_srv = m_desc_ranges[m_desc_ranges.push_back()];
	tex_table_srv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

    auto staticSamplers = GetStaticSamplers();

    auto &root_params_vec = root_sign->GetRootParams();
    root_params_vec.resize(5);
    root_params_vec[bi_model_cb].InitAsConstantBufferView     (cb_model, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
    root_params_vec[bi_g_buffer_tex_table].InitAsDescriptorTable        (1, &tex_table_srv, D3D12_SHADER_VISIBILITY_PIXEL);
    root_params_vec[bi_scene_cb].InitAsConstantBufferView     (cb_scene, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE);
    root_params_vec[bi_materials_cb].InitAsConstantBufferView (cb_materials, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
    root_params_vec[bi_vertex_buffer].InitAsShaderResourceView(tto_vertex_buffer);
    

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1((uint32_t)root_params_vec.size(), root_params_vec.data(), (uint32_t)staticSamplers.size(), staticSamplers.data(), rootSignatureFlags);

        // Serialize the root signature.
    ComPtr<ID3DBlob> rootSignatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    auto hres = D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
        featureData.HighestVersion, &rootSignatureBlob, &errorBlob);
        if (errorBlob.Get()){
            char* error = (char* )errorBlob->GetBufferPointer();
            assert(false);
            ThrowIfFailed(hres);
        }
    // Create the root signature.
    ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
        rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(root_sign->GetRootSignature().GetAddressOf())));
    SetName(root_sign->GetRootSignature(), dbg_name.value_or(L"").append(L"_root_signature_0").c_str());
}

// root sign for post-process
void Techniques::CreateRootSignature_1(ComPtr<ID3D12Device2> &device, RootSignature* root_sign, std::optional<std::wstring> dbg_name){
    // Create a root signature.
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)));

    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    // A single 32-bit constant root parameter that is used by the vertex shader.
    CD3DX12_DESCRIPTOR_RANGE1 &texTable = m_desc_ranges[m_desc_ranges.push_back()];
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 6, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

    auto staticSamplers = GetStaticSamplers();

    auto &root_params_vec = root_sign->GetRootParams();
    root_params_vec.resize(3);
    root_params_vec[bi_post_proc_input_tex_table].InitAsDescriptorTable        (1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
    root_params_vec[1].InitAsConstants(1, 0);
    root_params_vec[bi_scene_cb].InitAsConstantBufferView(cb_scene, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE); // sceneCB

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1((uint32_t)root_params_vec.size(), root_params_vec.data(), (uint32_t)staticSamplers.size(), staticSamplers.data(), rootSignatureFlags);

        // Serialize the root signature.
    ComPtr<ID3DBlob> rootSignatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
        featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
        if (errorBlob.Get()){
            auto errors = (char*)errorBlob->GetBufferPointer();
            assert(false);
        }
    // Create the root signature.
    ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
        rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(root_sign->GetRootSignature().GetAddressOf())));
    SetName(root_sign->GetRootSignature(), dbg_name.value_or(L"").append(L"_root_signature_1").c_str());
}

// root sign for shading
void Techniques::CreateRootSignature_2(ComPtr<ID3D12Device2> &device, RootSignature* root_sign, std::optional<std::wstring> dbg_name){
    // Create a root signature.
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)));

    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    // A single 32-bit constant root parameter that is used by the vertex shader.
    CD3DX12_DESCRIPTOR_RANGE1 &texTable = m_desc_ranges[m_desc_ranges.push_back()];
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 6, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

    auto staticSamplers = GetStaticSamplers();

    auto &root_params_vec = root_sign->GetRootParams();
    root_params_vec.resize(4);

    root_params_vec[bi_model_cb].InitAsConstantBufferView(cb_model, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE);
    root_params_vec[bi_fwd_tex].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL); // Texture
    root_params_vec[bi_scene_cb].InitAsConstantBufferView(cb_scene, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE); // sceneCB
    root_params_vec[bi_lights_cb].InitAsConstantBufferView(cb_lights, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL); // lights

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1((uint32_t)root_params_vec.size(), root_params_vec.data(), (uint32_t)staticSamplers.size(), staticSamplers.data(), rootSignatureFlags);

        // Serialize the root signature.
    ComPtr<ID3DBlob> rootSignatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
        featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
        if (errorBlob.Get()){
            assert(false);
        }
    // Create the root signature.
    ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
        rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(root_sign->GetRootSignature().GetAddressOf())));
    SetName(root_sign->GetRootSignature(), dbg_name.value_or(L"").append(L"_root_signature_2").c_str());
}
// root sign for terrain
void Techniques::CreateRootSignature_3(ComPtr<ID3D12Device2>& device, RootSignature* root_sign, std::optional<std::wstring> dbg_name) {
	// Create a root signature.
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)));

	// Allow input layout and deny unnecessary access to certain pipeline stages.
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	// A single 32-bit constant root parameter that is used by the vertex shader.
	CD3DX12_DESCRIPTOR_RANGE1& texTable = m_desc_ranges[m_desc_ranges.push_back()];
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

	auto staticSamplers = GetStaticSamplers();

	auto& root_params_vec = root_sign->GetRootParams();
	root_params_vec.resize(5);

    root_params_vec[bi_model_cb].InitAsConstantBufferView(cb_model, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE);
    root_params_vec[bi_terrain_hm].InitAsDescriptorTable(1, &texTable); // Textures
	root_params_vec[bi_scene_cb].InitAsConstantBufferView(cb_scene, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE); // sceneCB
    root_params_vec[bi_lights_cb].InitAsConstantBufferView(cb_lights, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL); // lights
    root_params_vec[bi_vertex_buffer].InitAsShaderResourceView(tto_vertex_buffer);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
	rootSignatureDescription.Init_1_1((uint32_t)root_params_vec.size(), root_params_vec.data(), (uint32_t)staticSamplers.size(), staticSamplers.data(), rootSignatureFlags);

	// Serialize the root signature.
	ComPtr<ID3DBlob> rootSignatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
		featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
	if (errorBlob.Get()) {
		assert(false);
	}
	// Create the root signature.
	ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(root_sign->GetRootSignature().GetAddressOf())));
	SetName(root_sign->GetRootSignature(), dbg_name.value_or(L"").append(L"_root_signature_3").c_str());
}

// blur
void Techniques::CreateRootSignature_4(ComPtr<ID3D12Device2>& device, RootSignature* root_sign, std::optional<std::wstring> dbg_name) {
	// Create a root signature.
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)));

	CD3DX12_DESCRIPTOR_RANGE1& srvTable = m_desc_ranges[m_desc_ranges.push_back()];
	srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

	CD3DX12_DESCRIPTOR_RANGE1& uavTable = m_desc_ranges[m_desc_ranges.push_back()];
	uavTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 5, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

	auto& root_params_vec = root_sign->GetRootParams();
	root_params_vec.resize(4);

	root_params_vec[bi_ssao_cb].InitAsConstantBufferView(cb_ssao, 0); // sceneCB
	root_params_vec[bi_ssao_input_tex].InitAsDescriptorTable(1, &srvTable);
	root_params_vec[bi_scene_cb].InitAsConstantBufferView(cb_scene, 0); // sceneCB
	root_params_vec[bi_ssao_uav_tex].InitAsDescriptorTable(1, &uavTable);

	auto staticSamplers = GetStaticSamplers();

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
	rootSignatureDescription.Init_1_1((uint32_t)root_params_vec.size(), root_params_vec.data(), (uint32_t)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

	// Serialize the root signature.
	ComPtr<ID3DBlob> rootSignatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
		featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
	if (errorBlob.Get()) {
		assert(false);
	}

	// Create the root signature.
	ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(root_sign->GetRootSignature().GetAddressOf())));
	SetName(root_sign->GetRootSignature(), dbg_name.value_or(L"").append(L"_root_signature_4").c_str());
}

void Techniques::OnInit(std::optional<std::wstring> dbg_name){
    auto device = gBackend->GetDevice()->GetNativeObject();

    {
        uint32_t id = 0;
        id = m_root_signatures.push_back();
        CreateRootSignature_0(device, &m_root_signatures[id], dbg_name);
        m_root_signatures[id].SetRSId(id);
        id = m_root_signatures.push_back();
        CreateRootSignature_1(device, &m_root_signatures[id], dbg_name);
        m_root_signatures[id].SetRSId(id);
        id = m_root_signatures.push_back();
        CreateRootSignature_2(device, &m_root_signatures[id], dbg_name);
        m_root_signatures[id].SetRSId(id);
        id = m_root_signatures.push_back();
		CreateRootSignature_3(device, &m_root_signatures[id], dbg_name);
		m_root_signatures[id].SetRSId(id);
        id = m_root_signatures.push_back();
		CreateRootSignature_4(device, &m_root_signatures[id], dbg_name);
		m_root_signatures[id].SetRSId(id);
    }

    {
        uint32_t id = 0;
        id = m_techniques.push_back(CreateTechnique_0(device, m_root_signatures[0], dbg_name));
        m_techniques[id].id = id;
        id = m_techniques.push_back(CreateTechnique_1(device, m_root_signatures[0], dbg_name));
        m_techniques[id].id = id;
        id = m_techniques.push_back(CreateTechnique_2(device, m_root_signatures[1], dbg_name));
        m_techniques[id].id = id;
        id = m_techniques.push_back(CreateTechnique_3(device, m_root_signatures[2], dbg_name));
        m_techniques[id].id = id;
        id = m_techniques.push_back(CreateTechnique_4(device, m_root_signatures[0], dbg_name));
        m_techniques[id].id = id;
		id = m_techniques.push_back(CreateTechnique_5(device, m_root_signatures[4], dbg_name));
		m_techniques[id].id = id;
		id = m_techniques.push_back(CreateTechnique_6(device, m_root_signatures[4], dbg_name));
		m_techniques[id].id = id;
		id = m_techniques.push_back(CreateTechnique_7(device, m_root_signatures[3], dbg_name));
		m_techniques[id].id = id;
		id = m_techniques.push_back(CreateTechnique_8(device, m_root_signatures[3], dbg_name));
		m_techniques[id].id = id;
        id = m_techniques.push_back(CreateTechnique_9(device, m_root_signatures[0], dbg_name));
        m_techniques[id].id = id;
        id = m_techniques.push_back(CreateTechnique_10(device, m_root_signatures[4], dbg_name));
        m_techniques[id].id = id;
    }
}

bool Techniques::TechHasColor(uint32_t tech_id) {
    if (tech_id == 0) {
        return true;
    }
    else {
        return false;
    }
}

void Techniques::RebuildShaders(std::optional<std::wstring> dbg_name)
{
    if (ShaderManager* shader_mgr = gBackend->GetShaderManager()) {
        shader_mgr->ResetCache();
    }

    auto device = gBackend->GetDevice()->GetNativeObject();

    m_techniques[0] = CreateTechnique_0(device, m_root_signatures[0], dbg_name);
    m_techniques[0].id = 0;
	m_techniques[1] = CreateTechnique_1(device, m_root_signatures[0], dbg_name);
	m_techniques[1].id = 1;
	m_techniques[2] = CreateTechnique_2(device, m_root_signatures[1], dbg_name);
	m_techniques[2].id = 2;
	m_techniques[3] = CreateTechnique_3(device, m_root_signatures[2], dbg_name);
	m_techniques[3].id = 3;
	m_techniques[4] = CreateTechnique_4(device, m_root_signatures[0], dbg_name);
	m_techniques[4].id = 4;
	m_techniques[5] = CreateTechnique_5(device, m_root_signatures[4], dbg_name);
	m_techniques[5].id = 5;
	m_techniques[6] = CreateTechnique_6(device, m_root_signatures[4], dbg_name);
	m_techniques[6].id = 6;
	m_techniques[7] = CreateTechnique_7(device, m_root_signatures[3], dbg_name);
	m_techniques[7].id = 7;
	m_techniques[8] = CreateTechnique_8(device, m_root_signatures[3], dbg_name);
	m_techniques[8].id = 8;
    m_techniques[9] = CreateTechnique_9(device, m_root_signatures[0], dbg_name);
    m_techniques[9].id = 9;
    m_techniques[10] = CreateTechnique_10(device, m_root_signatures[4], dbg_name);
    m_techniques[10].id = 10;
}
