#include "constant_buffers.ihlsl"

struct VertexPosColor
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 tangents : TANGENTS;
    float3 bitangents : BITANGENTS;
	float2 TexC : TEXCOORD;
};

struct VertexShaderOutput
{
    float2 TexC : TEXCOORD;
    float4 Pos : POSITION;
    float3x3 TBN : TBN0;
    float4 Position : SV_Position;
};

VertexShaderOutput main(VertexPosColor IN)
{
    VertexShaderOutput OUT;
    matrix MVP = mul(M, V);
    MVP = mul(MVP, P);
    OUT.Position = mul(float4(IN.Position, 1.0f), MVP);
    OUT.TexC = IN.TexC;
    OUT.Pos = float4(mul(float4(IN.Position, 1), M).xyz, OUT.Position.z);

    float3 T = normalize(mul(float4(IN.tangents, 0.0f), M).xyz);
    float3 B = normalize(mul(float4(IN.bitangents, 0.0f), M).xyz);
    float3 N = normalize(mul(float4(IN.Normal, 0.0f), M).xyz);
    float3x3 TBN = float3x3(T, B, N);
    OUT.TBN = TBN;
 
    return OUT;
}