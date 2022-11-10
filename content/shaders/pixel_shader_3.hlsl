struct PixelShaderInput
{
    float2 TexC : TEXCOORD;
};

struct Light
{
    float3 Color;
    float FalloffStart; // point/spot light only
    float3 Direction;   // directional/spot light only
    float FalloffEnd;   // point/spot light only
    float3 Position;    // point light only
    float SpotPower;    // spot light only
};

struct CameraPos
{
    float3 vec;
};

ConstantBuffer<CameraPos> CamPos : register(b0);

Texture2D    albedo_tx : register(t0);
Texture2D    normals : register(t1);
Texture2D    positions : register(t2);

SamplerState pointWrap  : register(s0);
SamplerState pointClamp  : register(s1);
SamplerState linearWrap  : register(s2);
SamplerState linearClamp  : register(s3);
SamplerState anisotropicWrap  : register(s4);
SamplerState anisotropicClamp  : register(s5);
 

 // material parameters
// const float3  albedo = float3(0.2, );
static float metallic = 0.8;
static float roughness = 0.2;
static float ao = 1;

// lights
// uniform float3 lightPositions[4];
// uniform float3 lightColors[4];

// uniform float3 camPos;

static float PI = 3.14159265359;
  
float DistributionGGX(float3 N, float3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(float3 N, float3 V, float3 L, float roughness);
float3 fresnelSchlick(float cosTheta, float3 F0);

float4 main(PixelShaderInput IN ) : SV_Target
{
    // hard coded values
    Light light;
    light.Color = float3(0.4, 0.4, 0.4);
    light.Color = light.Color * 2;
    light.Position = float3(0, 10, 0.4);
    light.Direction = normalize(float3(-1, -1, 1));

    float3 albedo = albedo_tx.Sample(linearClamp, IN.TexC).xyz;
    float3 WorldPos = positions.Sample(linearClamp, IN.TexC).xyz;
    float3 normal = normals.Sample(linearClamp, IN.TexC).xyz;
    float3 N = normalize(normal);
    float3 V = normalize(CamPos.vec.xyz - WorldPos);

    float3 F0 = float3(0.04, 0.04, 0.04); 
    F0 = lerp(F0, albedo, metallic);
	           
    // reflectance equation
    float3 Lo = float3(0.0, 0.0, 0.0);
    for(int i = 0; i < 1; ++i) 
    {
        // calculate per-light radiance
        float3 L = normalize(light.Position - WorldPos);
        float3 H = normalize(V + L);
        float distance    = length(light.Position - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        float3 radiance     = light.Color * attenuation;        
        
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
  
    float3 ambient = float3(0.03, 0.03, 0.03) * albedo * ao;
    float3 color = ambient + Lo;
	
    color = color / (color + float3(1.0, 1.0, 1.0));
    color = pow(color, float3(1.0/2.2, 1.0/2.2, 1.0/2.2));  
   
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

 /*
float4 main( PixelShaderInput IN ) : SV_Target
{
    // normal mapping
    float3 normal = normals.Sample(linearClamp, IN.TexC).xyz;
    normal = normalize(normal); 

    // hard coded values
    Light light;
    light.Color = float3(0.4, 0.4, 0.4);
    light.Color = light.Color * 2;
    light.Direction = normalize(float3(-1, -1, 1));
    float shininess = 12;
    
    // ambient
    float ambientStrength = 0.1;
    float3 ambient = ambientStrength * light.Color;

    // diffuse
    float3 light_dir = normalize(-light.Direction);
    float diff = max(dot(normal, light_dir), 0.0);
    float3 diffuse = diff * light.Color;

    // specular
    float3 viewDir = normalize(CamPos.vec.xyz - positions.Sample(linearClamp, IN.TexC).xyz);
    float3 halfwayDir = normalize(light_dir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    float3 specular = light.Color * spec;


    float4 diffuseAlbedo = albedo.Sample(linearClamp, IN.TexC);
    float3 result = (ambient + diffuse+ specular) * diffuseAlbedo.rgb;
    float4 pix_color = float4(result, 1);

    // brightness and contrast
    float c = 1.013;
    float b = -0.001;
    //pix_color.rgb = c * (pix_color.rgb - float3(0.5, 0.5, 0.5)) + 0.5 + b;

    // saturation
    float3 greyscale = pix_color.rgb * float3(0.299, 0.587, 0.114);
    //pix_color.rgb = lerp(greyscale, pix_color.rgb, 1);


    return pix_color;
}
*/