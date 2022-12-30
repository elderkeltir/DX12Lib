#include "constant_buffers.hlsl"

ByteAddressBuffer vertex_data : register(t5);

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

float3 unpack_vertex_float3(uint offset)
{
    float3 output;
    output.x = asfloat(vertex_data.Load(offset));
    output.y = asfloat(vertex_data.Load(offset + 4));
    output.z = asfloat(vertex_data.Load(offset + 8));
    
    return output;
}

float2 unpack_vertex_float2(uint offset)
{
    float2 output;
    output.x = asfloat(vertex_data.Load(offset));
    output.y = asfloat(vertex_data.Load(offset + 4));
    
    return output;
}

VertexShaderOutput main(uint vert_id : SV_VertexID)
{
    VertexShaderOutput OUT;
    VertexPosColor IN;
    uint vertex_offset_current = vertex_offset + ((56) * vert_id);
    IN.Position.xyz = unpack_vertex_float3(vertex_offset_current);
    IN.Normal.xyz = unpack_vertex_float3(vertex_offset_current + 12);
    IN.tangents.xyz = unpack_vertex_float3(vertex_offset_current + 24);
    IN.bitangents.xyz = unpack_vertex_float3(vertex_offset_current + 36);
    IN.TexC.xy = unpack_vertex_float2(vertex_offset_current + 48);
    
    
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