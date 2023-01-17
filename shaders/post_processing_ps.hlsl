#include "shader_defs.hlsl"
#include "constant_buffers.hlsl"

Texture2D offScreenTexture : register(t0);
Texture2D gui_texture : register(t1);
Texture2D fwd_tex : register(t2);
Texture2D ssao_tex : register(t3);
Texture2D sun_sm_tex : register(t4);

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
    float4 gui_pixel = gui_texture.Sample(pointWrap, input.textCoord);
    float4 fwd_pix_color = fwd_tex.Sample(pointWrap, input.textCoord);
    //return color;
    
    if (fwd_pix_color.a > 0)
    {
        pix_color.rgb = (fwd_pix_color.rgb * fwd_pix_color.a) + (pix_color.rgb * (pix_color.a - fwd_pix_color.a));
    }

	// grayscale //
    float intensity = dot(pix_color.rgb, GrayScaleIntensity);
	//pix_color.rgb =  intensity.rrr;

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
    
    if (uint(NearFarZ.w) == 1)
    {
        pix_color = float4(ssao_tex.Sample(pointWrap, input.textCoord).r, 0, 0, 1);
    }
    else if (uint(NearFarZ.w) == 2)
    {
        float z = sun_sm_tex.Sample(pointWrap, input.textCoord).r;
        z = (z - 0.99) / (1 - 0.99);
        pix_color = float4(z, 0, 0, 1);
    }
	
    if (gui_pixel.a > 0.01)
    {
        pix_color.rgb = gui_pixel.rgb;
    }
    
    return pix_color;
}