
struct VertexData
{
    float3 position;
    float3 normal;
    float3 tangents;
    float3 bitangents;
    float3 color;
    float2 tex_coords;
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

VertexData unpack_vertex_buffer_data()
{
    VertexData output = (VertexData)0;
    
    switch (vertex_type)
    {
        case 0:
            // pos, normal, color
            break;
        case 1:
            // pos, normal, tangents, bitangents, tex_coords
            break;
        
    }
    
    
    return output;
}