// @NoAlphaBlend

struct VSOutput {
    float4 position : SV_POSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
};

cbuffer Transform : register(b0) {
    float4x4 projection;
    float4x4 view;
    float4x4 world;
};

VSOutput vertex_main(float3 position : POSITION, float2 uv : UV, float3 normal : NORMAL) {
    VSOutput output;

    float4x4 wvp = mul(projection, mul(view, world));
    output.position = mul(wvp, float4(position, 1.0));
    output.uv = uv;
    output.normal = normal;
    
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
    output.color = sampled_color;
    
    return output;
}
