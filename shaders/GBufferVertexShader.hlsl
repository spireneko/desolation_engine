#include "Common.hlsli"

struct VS_INPUT {
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float4 color : COLOR;
    float viewDepth : TEXCOORD1;
};

cbuffer PerFrameConstants : register(b0) {
    matrix view, projection;
    float3 cameraPosition;
    int pointLightCount;
    DirectionalLight light;
    PointLight pointLights[16];
    ShadowConstants shadowData;
};

cbuffer PerObjectConstants : register(b1) {
    matrix world;
    Material material;
};

PS_INPUT main(VS_INPUT input) {
    PS_INPUT output;

    float4 worldPos = mul(float4(input.pos, 1.0f), world);
    output.worldPos = worldPos.xyz;

    float4 viewPos = mul(worldPos, view);
    output.viewDepth = -viewPos.z;

    output.pos = mul(viewPos, projection);
    output.normal = normalize(mul(float4(input.normal, 0.0f), world).xyz);
    output.uv = input.uv;
    output.color = input.color;

    return output;
}
