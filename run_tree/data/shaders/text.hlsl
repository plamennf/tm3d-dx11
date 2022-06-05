// @DiffuseTextureClamped
// @NoDepthTest

struct VSOutput {
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : UV;
};

cbuffer Transform : register(b0) {
    float4x4 projection;
    float4x4 view;
    float4x4 world;
    float4x4 transform;
};

VSOutput vertex_main(float3 position : POSITION, float4 color : COLOR, float2 uv : UV) {
    VSOutput output;
    
    output.position = mul(transform, float4(position, 1.0));
    output.color = color;
    output.uv = uv;
    
    return output;
}

struct PSOutput {
    float4 color : SV_TARGET;
};

Texture2D diffuse_texture : register(t0);
SamplerState diffuse_sampler_state : register(s0);

PSOutput pixel_main(VSOutput input) {
    PSOutput output;

    float4 sampled_color = diffuse_texture.Sample(diffuse_sampler_state, input.uv);
    output.color = input.color * sampled_color;

    return output;
}
