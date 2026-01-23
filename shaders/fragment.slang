
struct SceneData
{
    float4 ambient_color;
    float4 sunlight_direction; // [direction: vec3, power: f32]
    float4 sunlight_color;
};

struct CameraData
{
    float4x4 view;
    float4x4 proj;
    float4x4 view_proj;
    float3 position;
    float4 parameters; // [vfov, ar, near, far]
}

[[vk::binding(0, 0)]] ConstantBuffer<CameraData> camera;
[[vk::binding(1, 0)]] ConstantBuffer<SceneData> scene;

struct VertexOutput
{
    float4 position : SV_Position;
    [vk::location(0)] float3 color : COLOR;
    [vk::location(1)] float2 uv : TEXCOORD;
    [vk::location(2)] float3 normal : NORMAL;
    [vk::location(3)] float3 world_pos: POSITION0;
};

float3 aces_tonemap(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

[shader("fragment")]
float4 main(VertexOutput input) : SV_Target
{
    float3 N = normalize(input.normal);
    float3 L = normalize(scene.sunlight_direction.xyz);

    float3 V = normalize(camera.position - input.world_pos);

    float3 H = normalize(L + V);
    
    float3 ambient = scene.ambient_color.rgb;

    float NdotL = max(dot(N, L), 0.0);
    float3 diffuse = scene.sunlight_color.rgb * NdotL;

    float specular_strength = 0.5;
    float shininess = 32.0;
    
    float NdotH = max(dot(N, H), 0.0);
    float specular_intensity = pow(NdotH, shininess);
    float3 specular = scene.sunlight_color.rgb * specular_intensity * specular_strength;

    float light_power = scene.sunlight_direction.w;
    float3 light_contribution = (diffuse + specular) * light_power;

    float3 final_color = (ambient + light_contribution) * input.color;

    final_color = aces_tonemap(final_color);
    final_color = pow(final_color, 1.0 / 2.2);

    return float4(final_color, 1.0);
}
