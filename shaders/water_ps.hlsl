#include "shader_defs.hlsl"
#include "constant_buffers.hlsl"
#include "pbr_light.hlsl"

TextureCube SkyMap : register(t0);

struct PS_INPUT
{
    float4 sv_pos : SV_Position;
    float4 pos_world : COLOR0;
    float4 color : COLOR1;
    float4 Normal : NORMAL;
};

float4 main(PS_INPUT input) : SV_TARGET0
{    
    float metallic = 0.6;
    float roughness = 0.1;
    float ao = 1;

    float3 albedo = input.color.rgb;
    float3 WorldPos = input.pos_world.xyz;
    float3 normal = input.Normal.xyz;
    float3 N = normalize(normal);
    float3 V = normalize(CamPos.xyz - WorldPos);

    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);
	           
    // reflectance equation
    float3 Lo = float3(0.0, 0.0, 0.0);
    for (int i = 0; i < LIGHTS_NUM; ++i)
    {
        Light light = lights[i];
        uint tp = light.type;
        if (tp > 0)
        {
            float3 L;
            float attenuation;
            if (tp == 3)
            {
                float3 pos = light.Position;
                L = normalize(pos - WorldPos);
                float distance = length(pos - WorldPos);
                attenuation = 1.0 / (distance * distance);
            }
            else if (tp == 2)
            {
                attenuation = 1;
                L = normalize(-light.Direction);
            }
            // calculate per-light radiance
            
            float3 H = normalize(V + L);
            float3 l_color = light.Color;
            float3 radiance = l_color * attenuation;
            
            // cook-torrance brdf
            float NDF = DistributionGGX(N, H, roughness);
            float G = GeometrySmith(N, V, L, roughness);
            float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
            
            float3 kS = F;
            float3 kD = float3(1.0, 1.0, 1.0) - kS;
            kD *= 1.0 - metallic;
            
            float3 numerator = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
            float3 specular = numerator / denominator;
                
            // add to outgoing radiance Lo
            float NdotL = max(dot(N, L), 0.0);
            Lo += (kD * albedo / PI + specular) * radiance * NdotL;
        }
    }
    
    // skybox reflection
    float3 r = CalcReflectionSkyboxVec(CamPos.xyz, WorldPos, N);
    float4 reflectionColor = SkyMap.Sample(linearWrap, r);
  
    float3 ambient = float3(0.03, 0.03, 0.03) * albedo * ao ;
    float3 color = ambient + Lo;
   
    float4 FragColor = float4(color + (reflectionColor.rgb * 0.2 * (float3(1, 1, 1) - F0)), 0.8);

    return FragColor;
}

