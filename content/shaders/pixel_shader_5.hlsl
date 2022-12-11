#include "shader_defs.ihlsl"



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
    float4 pos_world : COLOR0;
    float4 color : COLOR1;
    float4 Normal : NORMAL;
};

ps_output main(PS_INPUT input)
{
    ps_output OUT = (ps_output) 0;
    //OUT.albedo = height_map.Sample(linearWrap, input.tex_coord);
    OUT.albedo = input.color;
    
    OUT.pos.xyz = input.pos_world.xyz;
    OUT.pos.w = 1;
    
    OUT.normal = float4(normalize(input.Normal.xyz), 1);
    
    OUT.material = float4(0.0, 0.9, 0, 0);
    
    return OUT;
}