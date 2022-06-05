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

PSOutput pixel_main(VSOutput input) {
    PSOutput output;

    output.color = input.color;
    
    return output;
}
