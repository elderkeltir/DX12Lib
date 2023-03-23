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
Texture2D    sun_sm : register(t5);

bool in_shadow(float3 world_pos)
{
    matrix MVP = mul(SunV, SunP);
    float4 shadow_pos = mul(float4(world_pos, 1.0f), MVP);
    shadow_pos /= shadow_pos.w;
    
    float2 shadow_uv;
    shadow_uv.x = (shadow_pos.x + 1) / 2;
    shadow_uv.y = (1 - shadow_pos.y) / 2;
    
    float depth_actual = sun_sm.Sample(depthMapSam, shadow_uv).r;
    float depth_expected = shadow_pos.z;
    
    return (depth_actual < depth_expected + 0.0000001);
    
}

#define NB_STEPS 128
#define G_SCATTERING 0.05

float ComputeScattering(float lightDotView)
{
    float result = 1.0f - G_SCATTERING * G_SCATTERING;
    result /= (4.0f * PI * pow(1.0f + G_SCATTERING * G_SCATTERING - (2.0f * G_SCATTERING) * lightDotView, 1.5f));
    return result;
}

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
    
    float shadowed = in_shadow(WorldPos);
    
    //
    float3 startPosition = CamPos.xyz;

    float3 rayVector = WorldPos.xyz - startPosition;

    float rayLength = length(rayVector);
    float3 rayDirection = rayVector / rayLength;

    float stepLength = rayLength / NB_STEPS;

    float3 step = rayDirection * stepLength;

    float3 currentPosition = startPosition;

    float3 accumFog = 0.0f.xxx;
    matrix MVP_sun = mul(SunV, SunP);
    
    for (int j = 0; j < NB_STEPS; j++)
    {
        float4 worldInShadowCameraSpace = mul(float4(currentPosition, 1.0f), MVP_sun);
        worldInShadowCameraSpace /= worldInShadowCameraSpace.w;
        float2 shadow_uv;
        shadow_uv.x = (worldInShadowCameraSpace.x + 1) / 2;
        shadow_uv.y = (1 - worldInShadowCameraSpace.y) / 2;

        float shadowMapValue = sun_sm.Sample(depthMapSam, shadow_uv).r;

        if (shadowMapValue > worldInShadowCameraSpace.z + 0.0000001)
        {
            Light light = lights[0];
            accumFog += ComputeScattering(dot(rayDirection, normalize(-light.Direction))).xxx * light.Color;

        }
        currentPosition += step;
    }
    accumFog /= NB_STEPS;
    //
  
    float3 ambient = float3(0.03, 0.03, 0.03) * albedo * ao;
    //float3 ambient = accumFog;
    float3 color = ambient + Lo + accumFog;
   
    float4 FragColor = float4(color, 1.0);
    if (shadowed)
    {
        FragColor *= 0.2;
    }

    return FragColor;
}

