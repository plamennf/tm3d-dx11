// @NoAlphaBlend

struct VSOutput {
    float4 position : SV_POSITION;
    float4 world_position : POSITION;
    float2 uv : UV;
    float3 world_normal : NORMAL;
};

cbuffer Transform : register(b0) {
    float4x4 projection;
    float4x4 view;
    float4x4 world;
    float4x4 transform;
};

VSOutput vertex_main(float3 position : POSITION, float2 uv : UV, float3 normal : NORMAL) {
    VSOutput output;

    output.world_position = mul(world, float4(position, 1.0));
    output.position = mul(view, output.world_position);
    output.position = mul(projection, output.position);
    output.uv = uv;
    output.world_normal = mul(world, float4(normal, 0.0)).xyz;
    
    return output;
}

struct PSOutput {
    float4 color : SV_TARGET;
};

Texture2D background_texture : register(t0);
Texture2D r_texture : register(t1);
Texture2D g_texture : register(t2);
Texture2D b_texture : register(t3);
Texture2D blend_map : register(t4);
SamplerState texture_sampler : register(s0);

PSOutput pixel_main(VSOutput input) {
    PSOutput output;

    float4 blend_map_color = blend_map.Sample(texture_sampler, input.uv);

    float background_texture_amount = 1 - (blend_map_color.r + blend_map_color.g + blend_map_color.b);
    float2 tiled_uv = input.uv * 100.0;
    float4 background_texture_color = background_texture.Sample(texture_sampler, tiled_uv) * background_texture_amount;
    float4 r_texture_color = r_texture.Sample(texture_sampler, tiled_uv) * blend_map_color.r;
    float4 g_texture_color = r_texture.Sample(texture_sampler, tiled_uv) * blend_map_color.g;
    float4 b_texture_color = r_texture.Sample(texture_sampler, tiled_uv) * blend_map_color.b;

    float4 total_color = background_texture_color + r_texture_color + g_texture_color + b_texture_color;

    output.color = total_color;
    
    return output;
}
