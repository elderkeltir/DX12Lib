#include "constant_buffers.hlsl"

ByteAddressBuffer vertex_data : register(t5);

struct VertexPos
{
    float3 Position : POSITION;
};

struct VertexShaderOutput
{
    float4 Position : SV_Position;
    float3 tex_coord    : TEXCOORD;
};

float3 unpack_vertex_float3(uint offset)
{
    float3 output;
    output.x = asfloat(vertex_data.Load(offset));
    output.y = asfloat(vertex_data.Load(offset + 4));
    output.z = asfloat(vertex_data.Load(offset + 8));
    
    return output;
}

VertexShaderOutput main(uint vert_id : SV_VertexID)
{
    VertexShaderOutput OUT;
    
    VertexPos IN;
    uint vertex_offset_current = vertex_offset + (12 * vert_id);
    IN.Position.xyz = unpack_vertex_float3(vertex_offset_current);
    
    OUT.tex_coord = IN.Position;

    // proj pos
    float4 posW = mul(float4(IN.Position, 1.0f), M);

    // Always center sky about camera.
    posW.xyz += CamPos.xyz;

    // Set z = w so that z/w = 1 (i.e., skydome always on far plane).
    matrix viewProj = mul(V, P);
    OUT.Position = mul(posW, viewProj).xyww;
 
    return OUT;
}