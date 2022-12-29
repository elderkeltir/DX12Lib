
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

float3 CalcReflectionSkyboxVec(float3 cam_pos, float3 world_pos, float3 normal)
{
    float3 V = normalize(cam_pos - world_pos);
    float3 r = reflect(-V, normal);
    
    return r;
}

float3 CalcRetractionSkyboxVec(float3 cam_pos, float3 world_pos, float3 normal, float ref_idx)
{
    float refraction_ratio = 1.00 / ref_idx;
    float3 I = normalize(world_pos - cam_pos);
    float3 R = refract(I, normal, refraction_ratio);
    
    return R;
}

float3 FogColor(float dist, float fog_start, float fog_range, float3 fog_color, float3 pix_color)
{
    float fogAmount = saturate((dist - fog_start) / fog_range);
    float3 color = lerp(pix_color, fog_color, fogAmount);
    
    return color;
}