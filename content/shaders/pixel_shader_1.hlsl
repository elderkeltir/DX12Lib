#include "shader_defs.ihlsl"

struct PixelShaderInput
{
    float2 TexC : TEXCOORD;
    float4 Pos : POSITION;
    float3x3 TBN : TBN0;
    float4 Position : SV_Position;
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
    output.albedo = gDiffuseMap.Sample(anisotropicClamp, IN.TexC);

    // normal mapping
    float3 normal = gNormalMap.Sample(linearClamp, IN.TexC).xyz;
    //normal.x = normal.x * 2 - 1;
    //normal.y = -normal.y * 2 + 1;
    normal = normal * 2.0 - 1.0;
    normal = normalize(mul(normal, IN.TBN));
    output.normal = float4(normal, 1.0);
    output.pos = float4(IN.Pos.xyz, (IN.Pos.w - 0.1) / (100.0 - 0.1));
    
    float met = metallicness.Sample(linearClamp, IN.TexC).r;
    float rough = roughness.Sample(linearClamp, IN.TexC).r;
    output.material = float4(met, rough, 1, 0);

    return output;
}