#include "constant_buffers.hlsl"

struct VS_IN
{
	float3 position : POSITION;
	float2 textCoord : TEXCOORD;
};

struct VS_OUT
{
	float2 textCoord : TEXCOORD0;
	float4 projPos : SV_POSITION;
    float3 positionV : POSITION;
};

VS_OUT main(VS_IN input)
{
	VS_OUT output;

	output.textCoord = input.textCoord;
	output.projPos = float4(input.position, 1);
	
	// Transform quad corners to view space near plane.
    float4 ph = mul(output.projPos, Pinv);
    output.positionV = ph.xyz / ph.w;
	
	return output;
}