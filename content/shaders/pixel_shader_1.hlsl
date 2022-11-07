struct PixelShaderInput
{
    float2 TexC : TEXCOORD;
    float3 Pos : POSITION;
    float3x3 TBN : TBN0;
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

struct ModelViewProjection
{
    matrix mx;
};

struct CameraPos
{
    float4 vec;
};

ConstantBuffer<ModelViewProjection> ModelCB : register(b0);
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
    // normal mapping
    float3 normal = gNormalMap.Sample(linearClamp, IN.TexC).xyz;
    normal = normal * 2.0 - 1.0;   
    normal = normalize(mul(IN.TBN, normal)); 

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
    float3 viewDir = normalize(CamPos.vec.xyz - IN.Pos);
    float3 halfwayDir = normalize(light_dir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    float3 specular = light.Color * spec;


    float4 diffuseAlbedo = gDiffuseMap.Sample(linearWrap, IN.TexC);
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