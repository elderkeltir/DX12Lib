#include "constant_buffers.ihlsl"

struct VertexPosColor
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Color    : COLOR;
};

struct VertexShaderOutput
{
    float4 Color    : COLOR0;
    float4 Normal : NORMAL;
    float4 WorldPos : POSITION;
    float4 material : COLOR1;
    float4 Position : SV_Position;
};


VertexShaderOutput main(VertexPosColor IN)
{
    VertexShaderOutput OUT;
    matrix MVP = mul(P, V);
    MVP = mul(MVP, M);
    OUT.Position = mul(MVP, float4(IN.Position, 1.0f));
    OUT.Normal = mul(M, float4(IN.Normal, 0.0f));
    OUT.WorldPos = mul(M, float4(IN.Position, 1.0f));
    OUT.Color = float4(IN.Color, 1.0f);
    OUT.material = float4(materials[material_id].metal, materials[material_id].rough, 0, 0);
 
    return OUT;
}