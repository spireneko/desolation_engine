#include "Common.hlsli"

cbuffer PointLightConstants : register(b0) {
    PointLight pointLight;

    matrix invViewProj;

    float3 cameraPosition;
    float pad;

    matrix worldViewProj;

    float2 screenSize;
    float2 pad2;
};

struct VS_INPUT {
    float3 pos : POSITION;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
};

PS_INPUT main(VS_INPUT input) {
    PS_INPUT output;
    output.pos = mul(float4(input.pos, 1.0), worldViewProj);
    return output;
}
