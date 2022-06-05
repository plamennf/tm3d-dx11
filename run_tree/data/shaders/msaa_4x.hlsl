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

Texture2DMS<float4> diffuse_texture : register(t0);

PSOutput pixel_main(VSOutput input) {
    PSOutput output;

    uint width, height, num_samples;
    diffuse_texture.GetDimensions(width, height, num_samples);
    
    float4 sampled_color = float4(0.0, 0.0, 0.0, 0.0);
    for (uint i = 0; i < 4; i++) {
        int2 uv = int2(input.uv.x * width, input.uv.y * height);
        sampled_color += diffuse_texture.Load(uv, i);
    }
    sampled_color /= 4.0;
    
    output.color = input.color * sampled_color;
    
    return output;
}
