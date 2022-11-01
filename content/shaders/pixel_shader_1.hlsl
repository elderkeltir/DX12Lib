struct PixelShaderInput
{
    float2 TexC : TEXCOORD;
};

Texture2D    gDiffuseMap : register(t0);
SamplerState gsamLinear  : register(s0);
 
float4 main( PixelShaderInput IN ) : SV_Target
{
    float4 diffuseAlbedo = gDiffuseMap.Sample(gsamLinear, IN.TexC);
    return diffuseAlbedo;
}