#include "shader_defs.ihlsl"
#include "constant_buffers.ihlsl"

Texture2D positions : register(t0);
Texture2D normals : register(t1);
Texture2D randlom_vals : register(t2);

static const int gSampleCount = 14;

struct PS_INPUT
{
    float2 tex_coord : TEXCOORD0;
    float4 sv_pos : SV_Position;
};

float main(PS_INPUT input) : SV_Target
{
    const float2 noiseScale = float2(RTdim.x / noise_dim, RTdim.y / noise_dim); // screen = 1280x720
    
    float3 fragPos = mul(V, positions.Sample(linearClamp, input.tex_coord)).rgb;
    float3 normal = normalize(mul(V, normals.Sample(linearClamp, input.tex_coord)).rgb) * 2.0 - 1.0;
    float3 ran_vec = randlom_vals.Sample(linearWrap, input.tex_coord * noiseScale).rgb;
    ran_vec = ran_vec * 2.0f - 1.0f;  

    //ran_vec.z = 0;
    float3 randomVec = normalize(ran_vec);
    //float3 randomVec = normalize(float3(0.5, 0.1, 0.0));
	
    float3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    float3 bitangent = cross(normal, tangent);
    float3x3 TBN = transpose(float3x3(tangent, bitangent, normal));
	
    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; ++i)
    {
    // get sample position
        float3 offseeet = gOffsetVectors[i].xyz;
        float3 samplePos_pure = mul(TBN, offseeet); // from tangent to view-space
        float3 samplePos = fragPos + samplePos_pure * radius;
    
        float4 offset = float4(samplePos, 1.0);
        offset = mul(P, offset); // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.x = (offset.x + 1) / 2;
        offset.y = (1 - offset.y) / 2;
        //offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0  
        
        float sampleDepth = mul(V, positions.Sample(linearClamp, offset.xy)).z;
        
        //occlusion += (sampleDepth <= samplePos.z + bias ? 1.0 : 0.0);
        
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth <= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    
    occlusion = 1.0 - (occlusion / kernelSize);
    
    return occlusion;
}

