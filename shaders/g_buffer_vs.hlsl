#include "constant_buffers.hlsl"
#include "vertex_data.hlsl"

struct VertexShaderOutput
{
    float4 world_position       : POSITION;
    float3 normal               : NORMAL;
    float3x3 TBN                : TBN;
    float4 color                : COLOR0;
    float3 tex_coord            : TEXCOORD;
    float4 position             : SV_Position;
};

VertexShaderOutput main(uint vert_id : SV_VertexID)
{
    VertexShaderOutput output;

    const uint vertex_size = get_vertex_size(vertex_type);
    uint vertex_offset_current = vertex_offset + (vertex_size * vert_id);
    VertexData v_data = unpack_vertex_buffer_data(vertex_data, vertex_offset_current, vertex_type);
    
    matrix MVP = mul(M, V);
    MVP = mul(MVP, P);
    output.position = mul(float4(v_data.position, 1.0f), MVP);
    output.world_position = float4(mul(float4(v_data.position, 1), M).xyz, output.position.z);
    output.tex_coord.z = vertex_type;
    
    if (vertex_type == 0)
    {
        output.normal.xyz = mul(float4(v_data.normal, 0.0f), M).xyz;
        output.color = float4(v_data.color, 1.0f);
    }
    if (vertex_type == 1)
    {
        output.tex_coord.xy = v_data.tex_coords.xy;

        float3 T = normalize(mul(float4(v_data.tangents, 0.0f), M).xyz);
        float3 B = normalize(mul(float4(v_data.bitangents, 0.0f), M).xyz);
        float3 N = normalize(mul(float4(v_data.normal, 0.0f), M).xyz);
        float3x3 TBN = float3x3(T, B, N);
        output.TBN = TBN;
    }
 
    return output;
}