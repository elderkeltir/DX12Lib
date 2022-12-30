#include "constant_buffers.hlsl"

static const float2 g_text_coords[6] =
{
    float2(0.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 0.0f),
    float2(0.0f, 1.0f),
    float2(1.0f, 0.0f),
    float2(1.0f, 1.0f)
};

struct VertexShaderOutput
{
	float2 tex_coord        : TEXCOORD0;
    float3 position_view    : POSITION;
    float4 position         : SV_POSITION;
};

VertexShaderOutput main(uint vertex_id : SV_VertexID)
{
    VertexShaderOutput output;
	
    output.tex_coord = g_text_coords[vertex_id];

    // Quad covering screen in NDC space.
    output.position = float4(2.0f * output.tex_coord.x - 1.0f, 1.0f - 2.0f * output.tex_coord.y, 0.0f, 1.0f);
	
	// Transform quad corners to view space near plane.
    float4 ph = mul(output.position, Pinv);
    output.position_view = ph.xyz / ph.w;
	
	return output;
}