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
    float3 Pos : POSITION;
    float3x3 TBN : TBN0;
    float4 Position : SV_Position;
};

VertexShaderOutput main(VertexPosColor IN)
{
    VertexShaderOutput OUT;
    matrix MVP = mul(P, V);
    MVP = mul(MVP, M);
    OUT.Position = mul(MVP, float4(IN.Position, 1.0f));
    OUT.TexC = IN.TexC;
    OUT.Pos = mul(M, float4(IN.Position, 1.0f)).xyz;

    float3 T = normalize(mul(M, float4(IN.tangents, 0.0f)).xyz);
    float3 B = normalize(mul(M, float4(IN.bitangents, 0.0f)).xyz);
    float3 N = normalize(mul(M, float4(IN.Normal, 0.0f)).xyz);
    float3x3 TBN = float3x3(T, B, N);
    OUT.TBN = TBN;
 
    return OUT;
}