#include "constant_buffers.ihlsl"

struct VertexShaderOutput
{
    float4 Position : SV_Position;
    float3 tex_coord : TEXCOORD;
};

static float3 gVertices[4] =
{
    float3(0, 0, 0),
    float3(1, 0, 0),
    float3(0, 0, 1),
    float3(1, 0, 1)
};

VertexShaderOutput main(uint vid : SV_VertexID, uint iid : SV_InstanceID)
{
    VertexShaderOutput OUT;
    
    uint vertex_id = (vid == 0 && iid % 2 == 0) ? 3 : vid;
    float3 v_pos = gVertices[vertex_id];
    
    float4 posW = mul(float4(v_pos, 1.0f), M);
    OUT.Position = posW;
    
    return OUT;
}