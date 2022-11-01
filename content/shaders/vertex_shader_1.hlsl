struct VertexPosColor
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
	float2 TexC : TEXCOORD;
};

struct VertexShaderOutput
{
    float2 TexC : TEXCOORD;
    float4 Position : SV_Position;
};

struct ModelViewProjection
{
    matrix mx;
};
 
ConstantBuffer<ModelViewProjection> ModelCB : register(b0);
ConstantBuffer<ModelViewProjection> ViewCB : register(b1);
ConstantBuffer<ModelViewProjection> ProjectionCB : register(b2);

VertexShaderOutput main(VertexPosColor IN)
{
    VertexShaderOutput OUT;
    matrix MVP = mul(ProjectionCB.mx, ViewCB.mx);
    MVP = mul(MVP, ModelCB.mx);
    OUT.Position = mul(MVP, float4(IN.Position, 1.0f));
    OUT.TexC = IN.TexC;
 
    return OUT;
}