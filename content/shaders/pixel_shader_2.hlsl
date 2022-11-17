#include "shader_defs.ihlsl"

Texture2D offScreenTexture : register(t0);

static const float3 GrayScaleIntensity = { 0.299f, 0.587f, 0.114f };
static const float3x3 SepiaFilter = { 0.393f, 0.349f, 0.272f,
                                      0.769f, 0.686f, 0.534f,
									  0.189f, 0.168f, 0.131f };

struct PS_INPUT
{
	float2 textCoord : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 pix_color = offScreenTexture.Sample(pointWrap, input.textCoord);
    //return color;

	// grayscale //
	
	//float intensity = dot(color.rgb, GrayScaleIntensity);
	//return float4(intensity.rrr, color.a);

	// iverse color //
	// return float4(1 - color.rgb, color.a);

	// sepia //
	// return float4(mul(color.rgb, SepiaFilter), color.a);

    // reinhard tone mapping
    pix_color.rgb = pix_color.rgb / (pix_color.rgb + float3(1.0, 1.0, 1.0));
    pix_color.a = 1;

    // gamma correction
    float gamma = 2.2;
    pix_color.rgb = pow(pix_color.rgb, float3(1.0/gamma, 1.0/gamma, 1.0/gamma));

    return pix_color;
}