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

Texture2D diffuse_texture : register(t0);
SamplerState diffuse_sampler_state : register(s0);

PSOutput pixel_main(VSOutput input) {
    PSOutput output;

    float3 light_pos = float3(0.0, 0.0, 0.0);

    float3 light_dir = normalize(light_pos - input.world_position.xyz);
    float dot_result = dot(input.world_normal, light_dir);
    float brightness = max(0.0, dot_result);
    float3 diffuse = brightness;
    
    float4 sampled_color = diffuse_texture.Sample(diffuse_sampler_state, input.uv);
    output.color = sampled_color * float4(diffuse, 1.0);
    
    return output;
}
