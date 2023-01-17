#include "constant_buffers.hlsl"
#include "vertex_data.hlsl"

struct VertexShaderOutput
{
    float4 position : SV_Position;
};

VertexShaderOutput main(uint vert_id : SV_VertexID)
{
    VertexShaderOutput output;

    const uint vertex_size = get_vertex_size(vertex_type);
    uint vertex_offset_current = vertex_offset + (vertex_size * vert_id);
    VertexData v_data = unpack_vertex_buffer_data(vertex_data, vertex_offset_current, vertex_type);
    
    matrix MVP = mul(M, SunV);
    MVP = mul(MVP, SunP);
    output.position = mul(float4(v_data.position, 1.0f), MVP);
 
    return output;
}