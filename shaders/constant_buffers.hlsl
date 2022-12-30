#define LIGHTS_NUM 16
#define MATERIALS_NUM 64

struct Material {
    float metal;
    float rough;
    float2 padding;
};

struct Light
{
    float3 Position;
    uint type;
    float3 Direction;
    uint id;
    float3 Color;
    uint padding;
};


// 1 x 256
cbuffer ModelCB : register(b0) {
    float4x4 M;
    uint material_id;
	uint vertex_offset;
    uint vertex_type;
    float padding;
};

// 1 x 256
cbuffer SceneCB : register(b1){
    float4x4 V;
    float4x4 P;
    float4 CamPos;
    float4x4 Pinv;
    float4 RTdim;
    float4 NearFarZ;
    float4 Time;
};

// 3 x 256
cbuffer LightsCB : register(b2)
{
    Light lights[LIGHTS_NUM];
}

// 4 x 256
cbuffer MaterialsCB : register(b3)
{
    Material materials[MATERIALS_NUM];
}

cbuffer SsaoCb : register(b4)
{
	float4   gOffsetVectors[14];

    uint     kernelSize;
    float    radius;
    float    bias;
    uint    noise_dim;

    //blur
    int pass_type;

    float w0;
    float w1;
    float w2;
    float w3;
    float w4;
    float w5;
    float w6;
    float w7;
    float w8;
    float w9;
    float w10;
};