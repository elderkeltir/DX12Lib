#include "shader_defs.ihlsl"

TextureCube height_map : register(t0);

struct ps_output
{
    float4 albedo : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 pos : SV_TARGET2;
    float4 material : SV_TARGET3;
};

struct PS_INPUT
{
    float4 sv_pos : SV_Position;
    float3 tex_coord : TEXCOORD;
};

ps_output main(PS_INPUT input)
{
    ps_output OUT = (ps_output) 0;
    //OUT.albedo = height_map.Sample(linearWrap, input.tex_coord);
    OUT.pos.w = 0;
    return OUT;
}