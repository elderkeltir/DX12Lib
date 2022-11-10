struct PixelShaderInput
{
    float4 Color    : COLOR;
    float4 Normal : NORMAL;
    float4 WorldPos : POSITION;
};

struct CameraPos
{
    float3 vec;
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

struct ps_output
{
	float4 albedo : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 pos : SV_TARGET2;
};

ConstantBuffer<CameraPos> CamPos : register(b0);
 
ps_output main( PixelShaderInput IN )
{
    ps_output output;
    output.albedo = IN.Color;

    // normal mapping
    output.normal = IN.Normal;
    output.pos = IN.WorldPos;

    return output;
}