
struct Vertex
{
    float3 position;
    float3 normal;
    float2 uv;
    float4 color;
};

struct PushConstants
{
    float4x4 world_transform;
    Vertex* vertex_buffer;
};

[[vk::push_constant]] ConstantBuffer<PushConstants> push_constants;

struct VertexOutput
{
    float4 position : SV_Position;
    [vk::location(0)] float3 color : COLOR0;
    [vk::location(1)] float2 uv : TEXCOORD0;
    [vk::location(2)] float3 normal: NORMAL0;
};

[shader("vertex")]
VertexOutput main(uint vertex_id : SV_VertexID)
{
    VertexOutput output;

    Vertex vertex = push_constants.vertex_buffer[vertex_id];

    output.position = mul(push_constants.world_transform, float4(vertex.position, 1.0f));
    output.color = vertex.color.rgb;
    output.uv = vertex.uv;
    output.normal = vertex.normal;

    return output;
}
