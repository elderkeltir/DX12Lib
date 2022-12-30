#include "shader_defs.hlsl"
#include "constant_buffers.hlsl"
#include "pbr_light.hlsl"

struct PixelShaderInput
{
    float2 TexC : TEXCOORD;
};

Texture2D    albedo_tx : register(t0);
Texture2D    normals : register(t1);
Texture2D    positions : register(t2);
Texture2D    material : register(t3);
Texture2D    ssao : register(t4);


float4 main(PixelShaderInput IN ) : SV_Target
{

    float metallic = material.Sample(linearClamp, IN.TexC).x;
    float roughness = material.Sample(linearClamp, IN.TexC).y;
    float ao = ssao.Sample(linearClamp, IN.TexC).r;

    float3 albedo = albedo_tx.Sample(anisotropicClamp, IN.TexC).xyz;
    float3 WorldPos = positions.Sample(linearClamp, IN.TexC).xyz;
    float3 normal = normals.Sample(linearClamp, IN.TexC).xyz;
    float3 N = normalize(normal);
    float3 V = normalize(CamPos.xyz - WorldPos);

    if (positions.Sample(linearClamp, IN.TexC).w == 0) {
        return float4(albedo.xyz, 1);
    }

    float3 F0 = float3(0.04, 0.04, 0.04); 
    F0 = lerp(F0, albedo, metallic);
	           
    // reflectance equation
    float3 Lo = float3(0.0, 0.0, 0.0);
    for(int i = 0; i < LIGHTS_NUM; ++i) 
    {
        Light light = lights[i];
        uint tp = light.type;
        if (tp > 0){
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
            float3 radiance     = l_color * attenuation;        
            
            // cook-torrance brdf
            float NDF = DistributionGGX(N, H, roughness);        
            float G   = GeometrySmith(N, V, L, roughness);      
            float3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
            
            float3 kS = F;
            float3 kD = float3(1.0, 1.0, 1.0) - kS;
            kD *= 1.0 - metallic;	  
            
            float3 numerator    = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
            float3 specular     = numerator / denominator;  
                
            // add to outgoing radiance Lo
            float NdotL = max(dot(N, L), 0.0);                
            Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
        }
    }   
  
    float3 ambient = float3(0.03, 0.03, 0.03) * albedo * ao;
    float3 color = ambient + Lo;
   
    float4 FragColor = float4(color, 1.0);

    return FragColor;
}

