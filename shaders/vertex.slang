
struct Vertex
{
    float3 position;
    float3 normal;
    float2 uv;
    float4 color;
};

struct Camera
{
    float4x4 view;
    float4x4 proj;
    float3 position;
    float4 parameters; // [vfov, aspect_ratio, near, far]
};

struct PushConstants
{
    Camera camera;
    float4x4 transform;
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

    Camera camera = push_constants.camera;
    Vertex vertex = push_constants.vertex_buffer[vertex_id];

    float4 position = mul(camera.proj, mul(camera.view, mul(push_constants.transform, float4(vertex.position, 1.0))));
    float3 world_normal = mul((float3x3)push_constants.transform, vertex.normal);

    output.position = position;
    output.color = vertex.color.rgb;
    output.uv = vertex.uv;
    output.normal = normalize(world_normal);

    return output;
}
