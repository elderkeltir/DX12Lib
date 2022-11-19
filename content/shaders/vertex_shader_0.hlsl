struct VertexPosColor
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Color    : COLOR;
};

struct VertexShaderOutput
{
    float4 Color    : COLOR;
    float4 Normal : NORMAL;
    float4 WorldPos : POSITION;
    float4 Position : SV_Position;
};

struct ModelViewProjection
{
    matrix mx;
};

ConstantBuffer<ModelViewProjection> ModelCB : register(b1);
ConstantBuffer<ModelViewProjection> ViewCB : register(b2);
ConstantBuffer<ModelViewProjection> ProjectionCB : register(b3);

VertexShaderOutput main(VertexPosColor IN)
{
    VertexShaderOutput OUT;
    matrix MVP = mul(ProjectionCB.mx, ViewCB.mx);
    MVP = mul(MVP, ModelCB.mx);
    OUT.Position = mul(MVP, float4(IN.Position, 1.0f));
    OUT.Normal = mul(ModelCB.mx, float4(IN.Normal, 0.0f));
    OUT.WorldPos = mul(ModelCB.mx, float4(IN.Position, 1.0f));
    OUT.Color = float4(IN.Color, 1.0f);
 
    return OUT;
}