#include "shader_defs.hlsl"
#include "constant_buffers.hlsl"

struct PixelShaderInput
{
    float4 world_position       : POSITION;
    float3 normal               : NORMAL;
    float3x3 TBN                : TBN;
    float4 color                : COLOR0;
    float3 tex_coord            : TEXCOORD;
    float4 position             : SV_Position;
};

struct ps_output
{
	float4 albedo : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 pos : SV_TARGET2;
    float4 material : SV_TARGET3;
};

Texture2D    gDiffuseMap : register(t0);
Texture2D    gNormalMap : register(t1);
Texture2D    metallicness : register(t2);
Texture2D    roughness : register(t3);

ps_output main( PixelShaderInput IN )
{
    ps_output output;
    
    const uint vertex_type = uint(IN.tex_coord.z);
    
    if (vertex_type == 0)
    {
        output.albedo = IN.color;

        // normal mapping
        output.normal = float4(normalize(IN.normal), 0);
        output.pos = float4(IN.world_position.xyz, (IN.world_position.w - NearFarZ.x) / (NearFarZ.y - NearFarZ.x));
        output.material = float4(materials[material_id].metal, materials[material_id].rough, 0, 0);
        //output.pos.w = 1 - IN.Position.z / IN.Position.w;
    }
    else if (vertex_type == 1)
    {
        output.albedo = gDiffuseMap.Sample(anisotropicClamp, IN.tex_coord.xy);

        // normal mapping
        float3 normal = gNormalMap.Sample(linearClamp, IN.tex_coord.xy).xyz;
        //normal.x = normal.x * 2 - 1;
        //normal.y = -normal.y * 2 + 1;
        normal = normal * 2.0 - 1.0;
        normal = normalize(mul(normal, IN.TBN));
        output.normal = float4(normal, 1.0);
        output.pos = float4(IN.world_position.xyz, (IN.world_position.w - NearFarZ.x) / (NearFarZ.y - NearFarZ.x));
    
        float met = metallicness.Sample(linearClamp, IN.tex_coord.xy).r;
        float rough = roughness.Sample(linearClamp, IN.tex_coord.xy).r;
        output.material = float4(met, rough, 1, 0);
    }

    return output;
}