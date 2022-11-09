#include "Techniques.h"
#include "DXHelper.h"
#include "ShaderManager.h"
#include <DirectXMath.h>

#include <directx/d3dx12.h>
#include "DXAppImplementation.h"

extern DXAppImplementation *gD3DApp;

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers()
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
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

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

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}

TODO("Minor. Implement PSO composition and serialization later. maybe.")
static Techniques::Technique CreateTechnique_0(ComPtr<ID3D12Device2> &device, std::optional<std::wstring> dbg_name = std::nullopt){
    Techniques::Technique tech;
    tech.vs = L"vertex_shader_0.hlsl";
    tech.ps = L"pixel_shader_0.hlsl";
    tech.vertex_type = 0;

    // Create the vertex input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // Create a root signature.
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)));

    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

    // A single 32-bit constant root parameter that is used by the vertex shader.
    CD3DX12_ROOT_PARAMETER1 rootParameters[3];
    rootParameters[0].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX); // M
    rootParameters[1].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX); // V
    rootParameters[2].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 2, 0, D3D12_SHADER_VISIBILITY_VERTEX); // P

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

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
        rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&tech.root_signature)));
    SetName(tech.root_signature, dbg_name.value_or(L"").append(L"_root_signature_0").c_str());

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    CD3DX12_SHADER_BYTECODE vs;
    CD3DX12_SHADER_BYTECODE ps;
    if (std::shared_ptr<ShaderManager> shader_mgr = gD3DApp->GetShaderManager().lock()){
        ShaderManager::ShaderBlob* vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
        ShaderManager::ShaderBlob* ps_blob = shader_mgr->Load(tech.ps, L"main", ShaderManager::ShaderType::st_pixel);
        vs = CD3DX12_SHADER_BYTECODE((const void*)vs_blob->data.data(), vs_blob->data.size());
        ps = CD3DX12_SHADER_BYTECODE((const void*)ps_blob->data.data(), ps_blob->data.size());
    }
    else{
        assert(false);
    }

    pipelineStateStream.pRootSignature = tech.root_signature.Get();
    pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
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

static Techniques::Technique CreateTechnique_1(ComPtr<ID3D12Device2> &device, std::optional<std::wstring> dbg_name = std::nullopt){
    Techniques::Technique tech;
    tech.vs = L"vertex_shader_1.hlsl";
    tech.ps = L"pixel_shader_1.hlsl";
    tech.vertex_type = 1;

    // Create the vertex input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BITANGENTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

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
    CD3DX12_DESCRIPTOR_RANGE1 texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0);

    auto staticSamplers = GetStaticSamplers();

    const uint32_t rootParameters_cnt = 4;
    CD3DX12_ROOT_PARAMETER1 rootParameters[rootParameters_cnt];
    rootParameters[0].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX); // M
    rootParameters[1].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX); // V
    rootParameters[2].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 2, 0, D3D12_SHADER_VISIBILITY_VERTEX); // P
    rootParameters[3].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL); // Texture
    assert(_countof(rootParameters) == rootParameters_cnt);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, (uint32_t)staticSamplers.size(), staticSamplers.data(), rootSignatureFlags);

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
        rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&tech.root_signature)));
        SetName(tech.root_signature, dbg_name.value_or(L"").append(L"_root_signature_1").c_str());

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R8G8B8A8_SNORM, DXGI_FORMAT_R16G16B16A16_FLOAT };
    rtvFormats.NumRenderTargets = 3;

    CD3DX12_SHADER_BYTECODE vs;
    CD3DX12_SHADER_BYTECODE ps;
    if (std::shared_ptr<ShaderManager> shader_mgr = gD3DApp->GetShaderManager().lock()){
        ShaderManager::ShaderBlob* vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
        ShaderManager::ShaderBlob* ps_blob = shader_mgr->Load(tech.ps, L"main", ShaderManager::ShaderType::st_pixel);
        vs = CD3DX12_SHADER_BYTECODE(vs_blob->data.data(), vs_blob->data.size());
        ps = CD3DX12_SHADER_BYTECODE(ps_blob->data.data(), ps_blob->data.size());
    }
    else{
        assert(false);
    }

    pipelineStateStream.pRootSignature = tech.root_signature.Get();
    pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
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

static Techniques::Technique CreateTechnique_2(ComPtr<ID3D12Device2> &device, std::optional<std::wstring> dbg_name = std::nullopt){
    Techniques::Technique tech;
    tech.vs = L"vertex_shader_2.hlsl";
    tech.ps = L"pixel_shader_2.hlsl";
    tech.vertex_type = 2;

    // Create the vertex input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // Create a root signature.
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)));

    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;

    // A single 32-bit constant root parameter that is used by the vertex shader.
    CD3DX12_DESCRIPTOR_RANGE1 texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

    auto staticSamplers = GetStaticSamplers();

    const uint32_t rootParameters_cnt = 1;
    CD3DX12_ROOT_PARAMETER1 rootParameters[rootParameters_cnt];
    rootParameters[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL); // Texture
    assert(_countof(rootParameters) == rootParameters_cnt);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, (uint32_t)staticSamplers.size(), staticSamplers.data(), rootSignatureFlags);

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
        rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&tech.root_signature)));
    SetName(tech.root_signature, dbg_name.value_or(L"").append(L"_root_signature_2").c_str());

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    CD3DX12_SHADER_BYTECODE vs;
    CD3DX12_SHADER_BYTECODE ps;
    if (std::shared_ptr<ShaderManager> shader_mgr = gD3DApp->GetShaderManager().lock()){
        ShaderManager::ShaderBlob* vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
        ShaderManager::ShaderBlob* ps_blob = shader_mgr->Load(tech.ps, L"main", ShaderManager::ShaderType::st_pixel);
        vs = CD3DX12_SHADER_BYTECODE((const void*)vs_blob->data.data(), vs_blob->data.size());
        ps = CD3DX12_SHADER_BYTECODE((const void*)ps_blob->data.data(), ps_blob->data.size());
    }
    else{
        assert(false);
    }

    pipelineStateStream.pRootSignature = tech.root_signature.Get();
    pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS = vs;
    pipelineStateStream.PS = ps;
    pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStream.RTVFormats = rtvFormats;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
        sizeof(PipelineStateStream), &pipelineStateStream
    };
    ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&tech.pipeline_state)));
    SetName(tech.pipeline_state, dbg_name.value_or(L"").append(L"_pso_2").c_str());

    return tech;
}

static Techniques::Technique CreateTechnique_3(ComPtr<ID3D12Device2> &device, std::optional<std::wstring> dbg_name = std::nullopt){
    Techniques::Technique tech;
    tech.vs = L"vertex_shader_2.hlsl";
    tech.ps = L"pixel_shader_3.hlsl";
    tech.vertex_type = 2;

    // Create the vertex input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // Create a root signature.
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)));

    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;

    // A single 32-bit constant root parameter that is used by the vertex shader.
    CD3DX12_DESCRIPTOR_RANGE1 texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0);

    auto staticSamplers = GetStaticSamplers();

    const uint32_t rootParameters_cnt = 2;
    CD3DX12_ROOT_PARAMETER1 rootParameters[rootParameters_cnt];
    rootParameters[0].InitAsConstants(sizeof(DirectX::XMFLOAT3) / 4, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL); // CamPos
    rootParameters[1].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL); // Texture
    assert(_countof(rootParameters) == rootParameters_cnt);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, (uint32_t)staticSamplers.size(), staticSamplers.data(), rootSignatureFlags);

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
        rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&tech.root_signature)));
    SetName(tech.root_signature, dbg_name.value_or(L"").append(L"_root_signature_2").c_str());

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = { DXGI_FORMAT_R16G16B16A16_FLOAT };
    rtvFormats.NumRenderTargets = 1;

    CD3DX12_SHADER_BYTECODE vs;
    CD3DX12_SHADER_BYTECODE ps;
    if (std::shared_ptr<ShaderManager> shader_mgr = gD3DApp->GetShaderManager().lock()){
        ShaderManager::ShaderBlob* vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
        ShaderManager::ShaderBlob* ps_blob = shader_mgr->Load(tech.ps, L"main", ShaderManager::ShaderType::st_pixel);
        vs = CD3DX12_SHADER_BYTECODE((const void*)vs_blob->data.data(), vs_blob->data.size());
        ps = CD3DX12_SHADER_BYTECODE((const void*)ps_blob->data.data(), ps_blob->data.size());
    }
    else{
        assert(false);
    }

    pipelineStateStream.pRootSignature = tech.root_signature.Get();
    pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS = vs;
    pipelineStateStream.PS = ps;
    pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStream.RTVFormats = rtvFormats;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
        sizeof(PipelineStateStream), &pipelineStateStream
    };
    ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&tech.pipeline_state)));
    SetName(tech.pipeline_state, dbg_name.value_or(L"").append(L"_pso_2").c_str());

    return tech;
}

void Techniques::OnInit(ComPtr<ID3D12Device2> &device, std::optional<std::wstring> dbg_name){
    uint32_t id = 0;
    id = m_techniques.push_back(CreateTechnique_0(device, dbg_name));
    m_techniques[id].id = id;
    id = m_techniques.push_back(CreateTechnique_1(device, dbg_name));
    m_techniques[id].id = id;
    id = m_techniques.push_back(CreateTechnique_2(device, dbg_name));
    m_techniques[id].id = id;
    id = m_techniques.push_back(CreateTechnique_3(device, dbg_name));
    m_techniques[id].id = id;
}