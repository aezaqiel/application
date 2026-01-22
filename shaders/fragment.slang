
struct VertexOutput
{
    float4 position : SV_Position;
    float3 color : COLOR;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

[shader("fragment")]
float4 main(VertexOutput input) : SV_Target
{
    float3 normal = input.normal * 0.5 + 0.5;
    return float4(normal, 1.0);
}
