#include "shader_defs.ihlsl"
#include "constant_buffers.ihlsl"

struct PixelShaderInput
{
    float2 TexC : TEXCOORD;
};

Texture2D    albedo_tx : register(t0);
Texture2D    normals : register(t1);
Texture2D    positions : register(t2);
Texture2D    material : register(t3);

static float ao = 1;
  
float DistributionGGX(float3 N, float3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(float3 N, float3 V, float3 L, float roughness);
float3 fresnelSchlick(float cosTheta, float3 F0);

float4 main(PixelShaderInput IN ) : SV_Target
{

    float metallic = material.Sample(linearClamp, IN.TexC).x;
    float roughness = material.Sample(linearClamp, IN.TexC).y;

    float3 albedo = albedo_tx.Sample(linearClamp, IN.TexC).xyz;
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
            // calculate per-light radiance
            float3 pos = light.Position;
            float3 L = normalize(pos - WorldPos);
            float3 H = normalize(V + L);
            float distance    = length(pos - WorldPos);
            float attenuation = 1.0 / (distance * distance);
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

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}
