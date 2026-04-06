Texture2D diffuseTexture : register(t0);
SamplerState samplerState : register(s0);

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

float4 main(PS_INPUT input) : SV_Target {
    float4 textureColor = diffuseTexture.Sample(samplerState, input.uv);
    return textureColor * input.color;
}
