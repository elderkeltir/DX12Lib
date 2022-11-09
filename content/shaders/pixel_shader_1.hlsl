struct PixelShaderInput
{
    float2 TexC : TEXCOORD;
    float3 Pos : POSITION;
    float3x3 TBN : TBN0;
};

struct ps_output
{
	float4 albedo : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 pos : SV_TARGET2;
};

Texture2D    gDiffuseMap : register(t0);
Texture2D    gNormalMap : register(t1);

SamplerState pointWrap  : register(s0);
SamplerState pointClamp  : register(s1);
SamplerState linearWrap  : register(s2);
SamplerState linearClamp  : register(s3);
SamplerState anisotropicWrap  : register(s4);
SamplerState anisotropicClamp  : register(s5);
 
ps_output main( PixelShaderInput IN )
{
    ps_output output;
    output.albedo = gDiffuseMap.Sample(linearWrap, IN.TexC);

    // normal mapping
    float3 normal = gNormalMap.Sample(linearClamp, IN.TexC).xyz;
    normal = normal * 2.0 - 1.0;
    normal = normalize(mul(IN.TBN, normal)); 
    output.normal = float4(normal, 0);
    output.pos = float4(IN.Pos, 1);

    return output;
}