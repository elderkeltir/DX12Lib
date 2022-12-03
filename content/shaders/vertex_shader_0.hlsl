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
    float4 Position : SV_Position;
};


VertexShaderOutput main(VertexPosColor IN)
{
    VertexShaderOutput OUT;
    matrix MVP = mul(M, V);
    MVP = mul(MVP, P);
    OUT.Position = mul(float4(IN.Position, 1.0f), MVP);
    OUT.Normal = mul(float4(IN.Normal, 0.0f), M);
    OUT.WorldPos = float4(mul(float4(IN.Position, 1.0), M).xyz, OUT.Position.z);
    OUT.Color = float4(IN.Color, 1.0f);
 
    return OUT;
}