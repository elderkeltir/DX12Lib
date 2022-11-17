#include "shader_defs.ihlsl"

struct PixelShaderInput
{
    float4 Color    : COLOR;
    float4 Normal : NORMAL;
    float4 WorldPos : POSITION;
};

struct ps_output
{
	float4 albedo : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 pos : SV_TARGET2;
    float4 material : SV_TARGET3;
};

cbuffer Model_cb : register(b4)
{
    uint material_id;
};

cbuffer materials_cb : register(b5)
{
    Material materials[MATERIALS_NUM];
}
 
ps_output main( PixelShaderInput IN )
{
    ps_output output;
    output.albedo = IN.Color;

    // normal mapping
    output.normal = IN.Normal;
    output.pos = IN.WorldPos;
    output.material = float4(materials[material_id].metal, materials[material_id].rough, material_id, 0);

    return output;
}