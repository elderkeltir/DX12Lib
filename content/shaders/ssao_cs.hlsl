#include "shader_defs.ihlsl"
#include "constant_buffers.ihlsl"

Texture2D depth_map : register(t0);
Texture2D normals : register(t1);
Texture2D randlom_vals : register(t2);

RWTexture2D<float> output_tex : register(u0);

#define N 32

static float gSurfaceEpsilon = 0.05f;
static float gOcclusionFadeStart = 0.2f;
static float gOcclusionFadeEnd = 2.0f;

// Determines how much the sample point q occludes the point p as a function
// of distZ.
float OcclusionFunction(float distZ)
{
	//
	// If depth(q) is "behind" depth(p), then q cannot occlude p.  Moreover, if 
	// depth(q) and depth(p) are sufficiently close, then we also assume q cannot
	// occlude p because q needs to be in front of p by Epsilon to occlude p.
	//
	// We use the following function to determine the occlusion.  
	// 
	//
	//       1.0     -------------\
	//               |           |  \
	//               |           |    \
	//               |           |      \
	//               |           |        \
	//               |           |          \
	//               |           |            \
	//  ------|------|-----------|-------------|---------|--> zv
	//        0     Eps          z0            z1        
	//
	
    float occlusion = 0.0f;
    if (distZ > gSurfaceEpsilon)
    {
        float fadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;
		
		// Linearly decrease occlusion from 1 to 0 as distZ goes 
		// from gOcclusionFadeStart to gOcclusionFadeEnd.	
        occlusion = saturate((gOcclusionFadeEnd - distZ) / fadeLength);
    }
	
    return occlusion;
}

float NdcDepthToViewDepth(float z_ndc)
{
    // z_ndc = A + B/viewZ, where gProj[2,2]=A and gProj[3,2]=B.
    float viewZ = P[3][2] / (z_ndc - P[2][2]);
    return viewZ;
}
 
[numthreads(N, N, 1)]
void main(int3 group_thread_id : SV_GroupThreadID,
				int3 dispatch_thread_id : SV_DispatchThreadID)
{
    const int2 pixel_pos = int2(dispatch_thread_id.x, dispatch_thread_id.y);
    const float2 tex_coord = (float2(dispatch_thread_id.xy) + float2(0.5, 0.5)) / RTdim.xy;
    
    // Quad covering screen in NDC space.
    float4 posH = float4(2.0f * tex_coord.x - 1.0f, 1.0f - 2.0f * tex_coord.y, 0.0f, 1.0f);
 
    // Transform quad corners to view space near plane.
    float4 ph = mul(posH, Pinv);
    float3 PosV = ph.xyz / ph.w;

    
	// p -- the point we are computing the ambient occlusion for.
	// n -- normal vector at p.
	// q -- a random offset from p.
	// r -- a potential occluder that might occlude p.

	// Get viewspace normal and z-coord of this pixel. 
    float4 nm_sampled = normals.SampleLevel(pointClamp, tex_coord, 0.0f);
    //if (nm_sampled.a > 0.99)
    //{
    //    output_tex[pixel_pos] = 1;
    //    return;
    //}
	
    float3 normal = normalize(nm_sampled.xyz);
    float pz = depth_map.SampleLevel(depthMapSam, tex_coord, 0.0f).r;
    //pz = 0.1 + (100.0 - 0.1) * pz;
    pz = NdcDepthToViewDepth(pz);

	//
	// Reconstruct full view space position (x,y,z).
	// Find t such that p = t*pin.PosV.
	// p.z = t*pin.PosV.z
	// t = p.z / pin.PosV.z    
    float3 p = (pz / PosV.z) * PosV;
	
	// Extract random vector and map from [0,1] --> [-1, +1].
    const float2 noiseScale = float2(RTdim.x / noise_dim, RTdim.y / noise_dim); // screen = 1280x720
    float3 randVec = 2.0f * randlom_vals.SampleLevel(linearWrap, noiseScale * tex_coord, 0.0f).rgb - 1.0f;

    float occlusionSum = 0.0f;
	
	// Sample neighboring points about p in the hemisphere oriented by n.
    for (int i = 0; i < kernelSize; ++i)
    {
		// Are offset vectors are fixed and uniformly distributed (so that our offset vectors
		// do not clump in the same direction).  If we reflect them about a random vector
		// then we get a random uniform distribution of offset vectors.
        float3 offset = reflect(gOffsetVectors[i].xyz, randVec);
	
		// Flip offset vector if it is behind the plane defined by (p, n).
        float flip = sign(dot(offset, normal));
		
		// Sample a point near p within the occlusion radius.
        float3 q = p + flip * radius * offset;
		
		// Project q and generate projective tex-coords.

        //float4 projQ = mul(float4(q, 1.0f), gProjTex);
        //projQ /= projQ.w;
        float4 offset_coord = mul(float4(q, 1.0), P);
        offset_coord.xyz /= offset_coord.w; // perspective divide
        offset_coord.x = (offset_coord.x + 1) / 2;
        offset_coord.y = (1 - offset_coord.y) / 2;

		// Find the nearest depth value along the ray from the eye to q (this is not
		// the depth of q, as q is just an arbitrary point near p and might
		// occupy empty space).  To find the nearest depth we look it up in the depthmap.

        float rz = depth_map.SampleLevel(depthMapSam, offset_coord.xy, 0.0f).r;
        rz = NdcDepthToViewDepth(rz);

		// Reconstruct full view space position r = (rx,ry,rz).  We know r
		// lies on the ray of q, so there exists a t such that r = t*q.
		// r.z = t*q.z ==> t = r.z / q.z

        float3 r = (rz / q.z) * q;
		
		//
		// Test whether r occludes p.
		//   * The product dot(n, normalize(r - p)) measures how much in front
		//     of the plane(p,n) the occluder point r is.  The more in front it is, the
		//     more occlusion weight we give it.  This also prevents self shadowing where 
		//     a point r on an angled plane (p,n) could give a false occlusion since they
		//     have different depth values with respect to the eye.
		//   * The weight of the occlusion is scaled based on how far the occluder is from
		//     the point we are computing the occlusion of.  If the occluder r is far away
		//     from p, then it does not occlude it.
		// 
		
        float distZ = p.z - r.z;
        float dp = max(dot(normal, normalize(r - p)), 0.0f);

        float occlusion = dp * OcclusionFunction(distZ);

        occlusionSum += occlusion;
    }
	
    occlusionSum /= kernelSize;
	
    float access = 1.0f - occlusionSum;

	// Sharpen the contrast of the SSAO map to make the SSAO affect more dramatic.    
    output_tex[pixel_pos] = saturate(pow(access, 6.0f));
}