#include "constant_buffers.ihlsl"

struct PixelShaderInput
{
    float4 Color    : COLOR0;
    float4 Normal : NORMAL;
    float4 WorldPos : POSITION;
    float4 material : COLOR1;
};

struct ps_output
{
	float4 albedo : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 pos : SV_TARGET2;
    float4 material : SV_TARGET3;
};

 
ps_output main( PixelShaderInput IN )
{
    ps_output output;
    output.albedo = IN.Color;

    // normal mapping
    output.normal = IN.Normal;
    output.pos = IN.WorldPos;
    output.material = IN.material;

    return output;
}