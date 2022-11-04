struct PixelShaderInput
{
    float2 TexC : TEXCOORD;
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
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
    float4 vec;
};

ConstantBuffer<CameraPos> CamPos : register(b3);

Texture2D    gDiffuseMap : register(t0);
Texture2D    gNormalMap : register(t1);

SamplerState pointWrap  : register(s0);
SamplerState pointClamp  : register(s1);
SamplerState linearWrap  : register(s2);
SamplerState linearClamp  : register(s3);
SamplerState anisotropicWrap  : register(s4);
SamplerState anisotropicClamp  : register(s5);
 
float4 main( PixelShaderInput IN ) : SV_Target
{
    // hard coded values
    Light light;
    light.Color = float3(0.9, 0.9, 0.9);
    light.Direction = normalize(float3(-1, -1, 1));
    float shininess = 12;
    
    // ambient
    float ambientStrength = 0.2;
    float3 ambient = ambientStrength * light.Color;

    // diffuse
    float3 normal = normalize(IN.Normal);
    float3 light_dir = normalize(-light.Direction);
    float diff = max(dot(normal, light_dir), 0.0);
    float3 diffuse = diff * light.Color;

    // specular
    float3 viewDir = normalize(CamPos.vec.xyz - IN.Pos);
    float3 halfwayDir = normalize(light_dir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    float3 specular = light.Color * spec;


    float4 diffuseAlbedo = gDiffuseMap.Sample(linearWrap, IN.TexC);
    float3 result = (ambient + diffuse+ specular) * diffuseAlbedo.rgb;
    float4 pix_color = float4(result, 1);
    
    return pix_color;
}