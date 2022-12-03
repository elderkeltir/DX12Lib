#include "constant_buffers.ihlsl"

struct PixelShaderInput
{
    float4 Color    : COLOR0;
    float4 Normal : NORMAL;
    float4 WorldPos : POSITION;
    float4 Position : SV_Position;
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
    output.normal = normalize(IN.Normal);
    output.pos = float4(IN.WorldPos.xyz, (IN.WorldPos.w - 0.1) / (100.0 - 0.1));
    output.material = float4(materials[material_id].metal, materials[material_id].rough, 0, 0);
    //output.pos.w = 1 - IN.Position.z / IN.Position.w;

    return output;
}