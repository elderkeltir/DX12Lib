struct VS_IN
{
	float3 position : POSITION;
	float2 textCoord : TEXCOORD;
};

struct VS_OUT
{
	float2 textCoord : TEXCOORD0;
	float4 projPos : SV_POSITION;
};

VS_OUT main(VS_IN input)
{
	VS_OUT output;

	output.textCoord = input.textCoord;
	output.projPos = float4(input.position, 1);

	return output;
}