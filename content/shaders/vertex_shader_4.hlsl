#include "shader_defs.ihlsl"
struct VertexPosColor
{
    float3 Position : POSITION;
};

struct VertexShaderOutput
{
    float4 Position : SV_Position;
    float3 tex_coord    : TEXCOORD;
};

ConstantBuffer<ModelViewProjection> ModelCB : register(b1);
ConstantBuffer<ModelViewProjection> ViewCB : register(b2);
ConstantBuffer<ModelViewProjection> ProjectionCB : register(b3);
ConstantBuffer<CameraPos> CamPos : register(b0);

VertexShaderOutput main(VertexPosColor IN)
{
    VertexShaderOutput OUT;
    OUT.tex_coord = IN.Position;

    // proj pos
    float4 posW = mul(ModelCB.mx, float4(IN.Position, 1.0f));

    // Always center sky about camera.
    posW.xyz += CamPos.vec.xyz;

    // Set z = w so that z/w = 1 (i.e., skydome always on far plane).
    matrix viewProj = mul(ProjectionCB.mx, ViewCB.mx);
    OUT.Position = mul(viewProj, posW).xyww;
 
    return OUT;
}