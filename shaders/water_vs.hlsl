#include "constant_buffers.hlsl"
#include "shader_defs.hlsl"

Texture2D height_map : register(t0);

struct VertexShaderOutput
{
    float4 Position : SV_Position;
    float4 pos_world : COLOR0;
    float4 color : COLOR1;
    float4 Normal : NORMAL;
};

static float3 gVertices[4] =
{
    float3(0, 0, 1),
    float3(1, 0, 1),
    float3(1, 0, 0),
    float3(0, 0, 0)
};

VertexShaderOutput main(uint vid : SV_VertexID, uint iid : SV_InstanceID)
{
    VertexShaderOutput OUT = (VertexShaderOutput) 0;
    float scale = 1;
    float y_scale = 100;
    float3 v_pos = gVertices[vid];
    v_pos *= scale;
    uint terrain_dim = NearFarZ.z;
    float x = iid % terrain_dim;
    float z = iid / terrain_dim;
    
    v_pos.x += x * scale;
    v_pos.z += z * scale;
    
    float2 tex_coord = v_pos.xz;
    tex_coord /= (float(terrain_dim) * scale);
    
    v_pos.x -= (terrain_dim / 2) * scale;
    v_pos.z -= (terrain_dim / 2) * scale;
    
    float height = sin((tex_coord.x + tex_coord.y) * Time.y * 1) * 0.1;
    v_pos.y = height;


    OUT.color = float4(0, 0, 1, 1);

    
    matrix MVP = mul(M, V);
    MVP = mul(MVP, P);
    OUT.Position = mul(float4(v_pos, 1.0f), MVP);
        
    OUT.pos_world.xyz = mul(float4(v_pos, 1.0f), M).xyz;
    //OUT.pos_world.w = height;
    
    if (1)
    {
        // normals
        uint second_tri = iid % 2;
        uint id0 = 0;
        uint id1 = 1;
        uint id2 = 2;
        
        if (second_tri == 0)
        {
            id1 = 2;
            id2 = 3;
        }
        
        float3 p0 = gVertices[id0];
        float3 p1 = gVertices[id1];
        float3 p2 = gVertices[id2];

        uint terrain_dim = NearFarZ.z;
        float x = iid % terrain_dim;
        float z = iid / terrain_dim;
    
        p0.x += x;
        p0.z += z;
        p1.x += x;
        p1.z += z;
        p2.x += x;
        p2.z += z;
    
        float2 tex_coord0 = p0.xz;
        tex_coord0 /= float(terrain_dim);
        float2 tex_coord1 = p1.xz;
        tex_coord1 /= float(terrain_dim);
        float2 tex_coord2 = p2.xz;
        tex_coord2 /= float(terrain_dim);
    
        p0.x -= terrain_dim / 2;
        p0.z -= terrain_dim / 2;
        p1.x -= terrain_dim / 2;
        p1.z -= terrain_dim / 2;
        p2.x -= terrain_dim / 2;
        p2.z -= terrain_dim / 2;
        
        float height0 = sin((tex_coord0.x + tex_coord0.y) * Time.y * 1) * 0.1;
        p0.y = height0;
        float height1 = sin((tex_coord1.x + tex_coord1.y) * Time.y * 1) * 0.1;
        p1.y = height1;
        float height2 = sin((tex_coord2.x + tex_coord2.y) * Time.y * 1) * 0.1;
        p2.y = height2;

        
        float3 vec1 = p0 - p1;
        float3 vec2 = p1 - p2;
        float3 normal = cross(vec1, vec2);
        
        OUT.Normal = mul(float4(normal, 0), M);
    }
    else
    {
        OUT.Normal = mul(float4(0,1,0, 0), M);
    }
    
    return OUT;
}