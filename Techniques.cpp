#include "Techniques.h"
#include "DXHelper.h"
#include "ShaderManager.h"
#include <DirectXMath.h>
#include "d3dx12.h"
#include "DXAppImplementation.h"

extern DXAppImplementation *gD3DApp;

TODO("Minor. Implement PSO composition and serialization later. maybe.")
static Techniques::Technique CreateTechnique_0(ComPtr<ID3D12Device2> &device){
    Techniques::Technique tech;
    tech.vs = L"vertex_shader.hlsl";
    tech.ps = L"pixel_shader.hlsl";

    if (std::shared_ptr<ShaderManager> shaderMgr = gD3DApp->GetShaderManager().lock()){

    }

    // Create the vertex input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        //{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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
        ComPtr<IDxcBlob> vs_blob = shader_mgr->Load(tech.vs, L"main", ShaderManager::ShaderType::st_vertex);
        ComPtr<IDxcBlob> ps_blob = shader_mgr->Load(tech.ps, L"main", ShaderManager::ShaderType::st_pixel);
        vs = CD3DX12_SHADER_BYTECODE(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize());
        ps = CD3DX12_SHADER_BYTECODE(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize());
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

    return tech;
}

void Techniques::OnInit(ComPtr<ID3D12Device2> &device){
    m_techniques[m_actual_techniques_count++] = CreateTechnique_0(device);
}