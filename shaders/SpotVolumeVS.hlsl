#include "Common.hlsli"

cbuffer SpotLightConstants : register(b0) {
    SpotLight spotLight;
    matrix invViewProj;
    float3 cameraPosition;
    float pad2;
    matrix worldViewProj;
    float2 screenSize;
    float2 pad3;
};

struct VS_INPUT {
    float3 pos : POSITION;
};

struct PS_OUTPUT {
    float4 pos : SV_POSITION;
};

PS_OUTPUT main(VS_INPUT input) {
    PS_OUTPUT output;
    output.pos = mul(float4(input.pos, 1.0), worldViewProj);
    return output;
}
