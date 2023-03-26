#include "shader_defs.hlsl"
#include "constant_buffers.hlsl"

Texture2D normals : register(t0);
Texture2D colors : register(t1);
Texture2D materials_tex : register(t2);
Texture2D world_poses : register(t3);

RWTexture2D<float4> output_tex : register(u0);

static const float max_ray_length = 50.f;
static const float ray_step = 0.25f;

#define N 32

[numthreads(N, N, 1)]
void main(int3 group_thread_id : SV_GroupThreadID,
				int3 dispatch_thread_id : SV_DispatchThreadID)
{
	const int2 pixel_pos = int2(dispatch_thread_id.x, dispatch_thread_id.y);
	const float2 tex_coord = (float2(dispatch_thread_id.xy) + float2(0.5, 0.5)) / RTdim.xy;
	
	float reflectivity = materials_tex.SampleLevel(pointClamp, tex_coord, 0).b;
	float4 color = float4(0, 0, 0, 0);
	
	if (reflectivity > 0)
	{
		float3 current_pos = world_poses.SampleLevel(pointClamp, tex_coord, 0).rgb;
		float3 current_nm = normals.SampleLevel(pointClamp, tex_coord, 0).rgb;
		float3 cam_dir = normalize(current_pos - CamPos.rgb);
		float3 ray_dir = normalize(reflect(cam_dir, current_nm));
		float current_ray_len = ray_step;
		
		matrix VP = mul(V, P);
		
		while (current_ray_len < max_ray_length )
		{
			float3 next_point = current_pos + ray_dir * ray_step;
			
			float4 next_uv = mul(float4(next_point, 1.0), VP);
			next_uv.xyz /= next_uv.w;
			next_uv.x = (next_uv.x + 1) * 0.5;
			next_uv.y = (1 - next_uv.y) * 0.5;
			
			float3 next_point_real = world_poses.SampleLevel(pointClamp, next_uv.xy, 0).rgb;
			
            float len_expected = length(next_point - current_pos);
            float len_actual = length(next_point_real - current_pos);
			
            if (len_expected > len_actual + 0.001)
			{
				color.rgb = colors.SampleLevel(pointClamp, next_uv.xy, 0).rgb;
				color.a = reflectivity;
				
				break;
			}
			
			current_pos = next_point;
			current_ray_len += ray_step;
		}
	}

	output_tex[pixel_pos] = color;
}