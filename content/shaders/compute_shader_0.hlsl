cbuffer cbSettings : register(b0)
{	
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

static const int gBlurRadius = 5;

Texture2D gInput : register(t0);
RWTexture2D<float> gOutput : register(u0);


#define N 32
#define CacheSize (N + 2*gBlurRadius)
groupshared float4 gCache[CacheSize][CacheSize];

void horizontal_pass(int3 groupThreadID, int3 dispatchThreadID);
void vertical_pass(int3 groupThreadID, int3 dispatchThreadID);


[numthreads(N, N, 1)]
void main(int3 groupThreadID : SV_GroupThreadID,
				int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (pass_type == 0)
    {
        horizontal_pass(groupThreadID, dispatchThreadID);
    }
    else
    {
        vertical_pass(groupThreadID, dispatchThreadID);
    }
}

void horizontal_pass(int3 groupThreadID, int3 dispatchThreadID)
{
    uint4 dims;
    gInput.GetDimensions(dims.x, dims.y, dims.z, dims.w);
    
    float weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

    if (groupThreadID.x < gBlurRadius)
    {
        int x = max(dispatchThreadID.x - gBlurRadius, 0);
        gCache[groupThreadID.x][groupThreadID.y] = gInput[int2(x, dispatchThreadID.y)];
    }
    if (groupThreadID.x >= N - gBlurRadius)
    {
        int x = min(dispatchThreadID.x + gBlurRadius, dims.y - 1);
        gCache[groupThreadID.x + 2 * gBlurRadius][groupThreadID.y] = gInput[int2(x, dispatchThreadID.y)];
    }

    gCache[groupThreadID.x + gBlurRadius][groupThreadID.y] = gInput[min(dispatchThreadID.xy, dims.yz - 1)];

    GroupMemoryBarrierWithGroupSync();
	
    float4 blurColor = float4(0, 0, 0, 0);
	
    for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
    {
        int k = groupThreadID.x + gBlurRadius + i;
		
        float WidthBlur = 0;
        float HeightBlur = 0;
        float WidthScreen = 0;
        float HeightScreen = 0;
        blurColor += weights[i + gBlurRadius] * gCache[k][groupThreadID.y];
    }
	
    gOutput[dispatchThreadID.xy] = blurColor.r;
}

void vertical_pass(int3 groupThreadID, int3 dispatchThreadID)
{
    uint4 dims;
    gInput.GetDimensions(dims.x, dims.y, dims.z, dims.w);
    
    float weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

    if (groupThreadID.y < gBlurRadius)
    {
        int y = max(dispatchThreadID.y - gBlurRadius, 0);
        gCache[groupThreadID.x][groupThreadID.y] = gInput[int2(dispatchThreadID.x, y)];
    }
    if (groupThreadID.y >= N - gBlurRadius)
    {
        int y = min(dispatchThreadID.y + gBlurRadius, dims.z - 1);
        gCache[groupThreadID.x][groupThreadID.y + 2 * gBlurRadius] = gInput[int2(dispatchThreadID.x, y)];
    }
	
    gCache[groupThreadID.x][groupThreadID.y + gBlurRadius] = gInput[min(dispatchThreadID.xy, dims.yz - 1)];


    GroupMemoryBarrierWithGroupSync();
	
    float4 blurColor = float4(0, 0, 0, 0);
	
    for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
    {
        int k = groupThreadID.y + gBlurRadius + i;

        float WidthBlur = 0;
        float HeightBlur = 0;
        float WidthScreen = 0;
        float HeightScreen = 0;

        blurColor += weights[i + gBlurRadius] * gCache[groupThreadID.x][k];
    }
	
    gOutput[dispatchThreadID.xy] = blurColor.r;
}