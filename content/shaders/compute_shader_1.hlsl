#include "constant_buffers.ihlsl"

static const int blur_radius = 5;

Texture2D input_tex : register(t0);
RWTexture2D<float> output_tex : register(u0);


#define N 32
#define CacheSize (N + 2*blur_radius)
groupshared float4 g_cache[CacheSize][CacheSize];

void horizontal_pass(int3 group_thread_id, int3 dispatch_thread_id);
void vertical_pass(int3 group_thread_id, int3 dispatch_thread_id);


[numthreads(N, N, 1)]
void main(int3 group_thread_id : SV_GroupThreadID,
				int3 dispatch_thread_id : SV_DispatchThreadID)
{
    if (pass_type == 0)
    {
        horizontal_pass(group_thread_id, dispatch_thread_id);
    }
    else
    {
        vertical_pass(group_thread_id, dispatch_thread_id);
    }
}

void horizontal_pass(int3 group_thread_id, int3 dispatch_thread_id)
{
    uint4 dims;
    input_tex.GetDimensions(dims.x, dims.y, dims.z, dims.w);
    
    float weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

    if (group_thread_id.x < blur_radius)
    {
        int x = max(dispatch_thread_id.x - blur_radius, 0);
        g_cache[group_thread_id.x][group_thread_id.y] = input_tex[int2(x, dispatch_thread_id.y)];
    }
    if (group_thread_id.x >= N - blur_radius)
    {
        int x = min(dispatch_thread_id.x + blur_radius, dims.y - 1);
        g_cache[group_thread_id.x + 2 * blur_radius][group_thread_id.y] = input_tex[int2(x, dispatch_thread_id.y)];
    }

    g_cache[group_thread_id.x + blur_radius][group_thread_id.y] = input_tex[min(dispatch_thread_id.xy, dims.yz - 1)];

    GroupMemoryBarrierWithGroupSync();
	
    float4 blur_color = float4(0, 0, 0, 0);
	
    for (int i = -blur_radius; i <= blur_radius; ++i)
    {
        int k = group_thread_id.x + blur_radius + i;
		
        float width_blur = 0;
        float height_blur = 0;
        float width_screen = 0;
        float height_screen = 0;
        blur_color += weights[i + blur_radius] * g_cache[k][group_thread_id.y];
    }
	
    output_tex[dispatch_thread_id.xy] = blur_color.r;
}

void vertical_pass(int3 group_thread_id, int3 dispatch_thread_id)
{
    uint4 dims;
    input_tex.GetDimensions(dims.x, dims.y, dims.z, dims.w);
    
    float weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

    if (group_thread_id.y < blur_radius)
    {
        int y = max(dispatch_thread_id.y - blur_radius, 0);
        g_cache[group_thread_id.x][group_thread_id.y] = input_tex[int2(dispatch_thread_id.x, y)];
    }
    if (group_thread_id.y >= N - blur_radius)
    {
        int y = min(dispatch_thread_id.y + blur_radius, dims.z - 1);
        g_cache[group_thread_id.x][group_thread_id.y + 2 * blur_radius] = input_tex[int2(dispatch_thread_id.x, y)];
    }
	
    g_cache[group_thread_id.x][group_thread_id.y + blur_radius] = input_tex[min(dispatch_thread_id.xy, dims.yz - 1)];


    GroupMemoryBarrierWithGroupSync();
	
    float4 blur_color = float4(0, 0, 0, 0);
	
    for (int i = -blur_radius; i <= blur_radius; ++i)
    {
        int k = group_thread_id.y + blur_radius + i;

        float width_blur = 0;
        float height_blur = 0;
        float width_screen = 0;
        float height_screen = 0;

        blur_color += weights[i + blur_radius] * g_cache[group_thread_id.x][k];
    }
	
    output_tex[dispatch_thread_id.xy] = blur_color.r;
}