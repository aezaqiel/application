
[[vk::binding(0, 0)]] RWTexture2D<float4> storage_image;

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 dispatch_id : SV_DispatchThreadID, uint3 group_id : SV_GroupThreadID)
{
    int2 texel_coord = int2(dispatch_id.xy);

    uint width;
    uint height;
    storage_image.GetDimensions(width, height);

    if (texel_coord.x >= width || texel_coord.y >= height) return;

    float4 color = float4(0.0, 0.0, 0.0, 1.0);

    if (group_id.x != 0 && group_id.y != 0) {
        color.x = float(texel_coord.x) / float(width);
        color.y = float(texel_coord.y) / float(height);
    }

    storage_image[texel_coord] = color;
}
