#include "constant_buffers.hlsl"
#include "vertex_data.hlsl"

struct VertexShaderOutput
{
    float4 position : SV_Position;
    float3 tex_coord    : TEXCOORD;
};

VertexShaderOutput main(uint vert_id : SV_VertexID)
{
    VertexShaderOutput output;
    const uint vertex_offset_current = vertex_offset + (12 * vert_id);
    VertexData v_data = unpack_vertex_buffer_data(vertex_data, vertex_offset_current, vertex_type);

    // proj pos
    float4 posW = mul(float4(v_data.position, 1.0f), M);

    // Always center sky about camera.
    posW.xyz += CamPos.xyz;

    // Set z = w so that z/w = 1 (i.e., skydome always on far plane).
    matrix viewProj = mul(V, P);
    output.position = mul(posW, viewProj).xyww;
    output.tex_coord = v_data.tex_coords;
 
    return output;
}