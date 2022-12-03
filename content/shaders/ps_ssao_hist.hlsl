#include "shader_defs.ihlsl"
#include "constant_buffers.ihlsl"

Texture2D positions : register(t0);
Texture2D normals : register(t1);
Texture2D randlom_vals : register(t2);
Texture2D depth_map : register(t3);

static const int gSampleCount = 14;

struct PS_INPUT
{
    float2 tex_coord : TEXCOORD0;
    float4 sv_pos : SV_Position;
    float3 positionV : POSITION;
};

static float gSurfaceEpsilon = 0.05f;
static float gOcclusionFadeStart = 0.2f;
static float gOcclusionFadeEnd = 2.0f;

//=============================================================================
// Ssao.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//============================================================================= 
// Nonnumeric values cannot be added to a cbuffer.

//static const int gSampleCount = 14;

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
 
float4 main(PS_INPUT pin) : SV_Target
{
	// p -- the point we are computing the ambient occlusion for.
	// n -- normal vector at p.
	// q -- a random offset from p.
	// r -- a potential occluder that might occlude p.

	// Get viewspace normal and z-coord of this pixel.  
    float3 normal = normalize(normals.SampleLevel(pointClamp, pin.tex_coord, 0.0f).xyz);
    float pz = depth_map.SampleLevel(depthMapSam, pin.tex_coord, 0.0f).r;
    //pz = 0.1 + (100.0 - 0.1) * pz;
    pz = NdcDepthToViewDepth(pz);

	//
	// Reconstruct full view space position (x,y,z).
	// Find t such that p = t*pin.PosV.
	// p.z = t*pin.PosV.z
	// t = p.z / pin.PosV.z    
    float3 p = (pz / pin.positionV.z) * pin.positionV;
	
	// Extract random vector and map from [0,1] --> [-1, +1].
    const float2 noiseScale = float2(RTdim.x / noise_dim, RTdim.y / noise_dim); // screen = 1280x720
    float3 randVec = 2.0f * randlom_vals.SampleLevel(linearWrap, noiseScale * pin.tex_coord, 0.0f).rgb - 1.0f;

    float occlusionSum = 0.0f;
	
	// Sample neighboring points about p in the hemisphere oriented by n.
    for (int i = 0; i < gSampleCount; ++i)
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
	
    occlusionSum /= gSampleCount;
	
    float access = 1.0f - occlusionSum;

	// Sharpen the contrast of the SSAO map to make the SSAO affect more dramatic.
    return saturate(pow(access, 6.0f));
}



/*

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
        float3 sampleDir = normalize(samplePos - fragPos);
        float nDotS = max(dot(normal, sampleDir), 0);
        
        float sampleDepth = mul(V, positions.Sample(linearClamp, offset.xy)).z;
        
        //occlusion += (sampleDepth <= samplePos.z + bias ? 1.0 : 0.0);
        
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += step(sampleDepth, samplePos.z) * rangeCheck * nDotS;
    }
    
    occlusion = 1.0 - (occlusion / kernelSize);
    
    return occlusion;
}

*/