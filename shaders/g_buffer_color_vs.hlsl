#include "constant_buffers.hlsl"

ByteAddressBuffer vertex_data : register(t5);

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

float3 unpack_vertex_float3(uint offset)
{
    float3 output;
    output.x = asfloat(vertex_data.Load(offset    ));
    output.y = asfloat(vertex_data.Load(offset + 4));
    output.z = asfloat(vertex_data.Load(offset + 8));
    
    return output;
}


VertexShaderOutput main(uint vert_id : SV_VertexID)
{
    uint vertex_offset_current = vertex_offset + ((36) * vert_id);
    VertexPosColor test_v;
    test_v.Position.xyz = unpack_vertex_float3(vertex_offset_current);
    test_v.Normal.xyz = unpack_vertex_float3(vertex_offset_current + 12);
    test_v.Color.xyz = unpack_vertex_float3(vertex_offset_current + 24);
    
    VertexShaderOutput OUT;
    matrix MVP = mul(M, V);
    MVP = mul(MVP, P);
    OUT.Position = mul(float4(test_v.Position, 1.0f), MVP);
    OUT.Normal = mul(float4(test_v.Normal, 0.0f), M);
    OUT.WorldPos = float4(mul(float4(test_v.Position, 1.0), M).xyz, OUT.Position.z);
    OUT.Color = float4(test_v.Color, 1.0f);
 
    return OUT;
}