#include "constant_buffers.ihlsl"

struct VertexPosColor
{
    float3 Position : POSITION;
};

struct VertexShaderOutput
{
    float4 Position : SV_Position;
    float3 tex_coord    : TEXCOORD;
};

VertexShaderOutput main(VertexPosColor IN)
{
    VertexShaderOutput OUT;
    OUT.tex_coord = IN.Position;

    // proj pos
    float4 posW = mul(M, float4(IN.Position, 1.0f));

    // Always center sky about camera.
    posW.xyz += CamPos.xyz;

    // Set z = w so that z/w = 1 (i.e., skydome always on far plane).
    matrix viewProj = mul(P, V);
    OUT.Position = mul(viewProj, posW).xyww;
 
    return OUT;
}