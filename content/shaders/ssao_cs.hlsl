#include "shader_defs.ihlsl"
#include "constant_buffers.ihlsl"

Texture2D depth_map : register(t0);
Texture2D normals : register(t1);
Texture2D randlom_vals : register(t2);
Texture2D world_positions : register(t3);

RWTexture2D<float> output_tex : register(u0);

#define N 32
 
[numthreads(N, N, 1)]
void main(int3 group_thread_id : SV_GroupThreadID,
				int3 dispatch_thread_id : SV_DispatchThreadID)
{
    const int2 pixel_pos = int2(dispatch_thread_id.x, dispatch_thread_id.y);
    const float2 tex_coord = (float2(dispatch_thread_id.xy) + float2(0.5, 0.5)) / RTdim.xy;
    float2 noiseScale = float2(RTdim.x / noise_dim, RTdim.y / noise_dim);
    float3 randVec = normalize(float3(randlom_vals.SampleLevel(linearWrap, noiseScale * tex_coord, 0.0f).gb, 0)) * 2.0f - 1.0f;
    
    float4 nm_sampled = normals.SampleLevel(pointClamp, tex_coord, 0.0f);
    float3 world_pos = world_positions.SampleLevel(pointClamp, tex_coord, 0.0f).rgb;

    float3 normal = normalize(mul(float4(normalize(nm_sampled.xyz), 0), V).rgb);
    float3 view_pos = mul(float4(world_pos, 1), V).rgb;

    float3 tangent = normalize(randVec - normal * dot(randVec, normal));
    float3 bitangent = cross(normal, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, normal);
	
    uint sample_size = kernelSize;
    float occlusionSum = 0.0f;
    for (int i = 0; i < kernelSize; ++i)
    {
        float3 offset = float3(gOffsetVectors[i].xyz);
		
        float3 samplePos = mul(offset, TBN);
        if (dot(samplePos, normal) < 0.2)
        {
            sample_size--;
            continue;
        }
        
        samplePos = view_pos + samplePos * radius;

        float4 offset_coord = mul(float4(samplePos, 1.0), P);
        offset_coord.xyz /= offset_coord.w; // perspective divide
        offset_coord.x = (offset_coord.x + 1) / 2;
        offset_coord.y = (1 - offset_coord.y) / 2;
		
        float3 world_pos_new = world_positions.SampleLevel(pointClamp, offset_coord.xy, 0.0f).rgb;
        float3 view_pos_new = mul(float4(world_pos_new, 1), V).rgb;

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(view_pos.z - view_pos_new.z));
        occlusionSum += (view_pos_new.z <= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
	
    occlusionSum /= sample_size;
	
    float access = 1.0f - occlusionSum; 
    output_tex[pixel_pos] = saturate(pow(access, 2.0f));
}