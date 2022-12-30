ByteAddressBuffer vertex_data : register(t5);

struct VertexData
{
    float3 position;
    float3 normal;
    float3 tangents;
    float3 bitangents;
    float3 color;
    float3 tex_coords;
};

float3 unpack_vertex_float3(ByteAddressBuffer vertex_buffer, uint offset)
{
    float3 output;
    output.x = asfloat(vertex_buffer.Load(offset));
    output.y = asfloat(vertex_buffer.Load(offset + 4));
    output.z = asfloat(vertex_buffer.Load(offset + 8));
    
    return output;
}

float2 unpack_vertex_float2(ByteAddressBuffer vertex_buffer, uint offset)
{
    float2 output;
    output.x = asfloat(vertex_buffer.Load(offset));
    output.y = asfloat(vertex_buffer.Load(offset + 4));
    
    return output;
}

float unpack_vertex_float(ByteAddressBuffer vertex_buffer, uint offset)
{
    float output;
    output.x = asfloat(vertex_buffer.Load(offset));
    
    return output;
}

uint get_vertex_size(uint vertex_type)
{
    uint size = 0;
    
    if (vertex_type == 0)
    {
        size = 36;
    }
    else if (vertex_type == 1)
    {
        size = 56;
    }
    
    return size;
}

VertexData unpack_vertex_buffer_data(ByteAddressBuffer vertex_buffer, uint offset, uint vertex_type)
{
    VertexData output = (VertexData)0;
    output.position.xyz = unpack_vertex_float3(vertex_buffer, offset);
    
    switch (vertex_type)
    {
        case 0:
            output.normal.xyz = unpack_vertex_float3(vertex_buffer, offset + 12);
            output.color.xyz = unpack_vertex_float3(vertex_buffer, offset + 24);
            break;
        case 1:
            output.normal.xyz = unpack_vertex_float3(vertex_buffer, offset + 12);
            output.tangents.xyz = unpack_vertex_float3(vertex_buffer, offset + 24);
            output.bitangents.xyz = unpack_vertex_float3(vertex_buffer, offset + 36);
            output.tex_coords.xy = unpack_vertex_float2(vertex_buffer, offset + 48);
            break;
        case 3:
            output.tex_coords.xyz = output.position.xyz;
            break;
    }
    
    
    return output;
}