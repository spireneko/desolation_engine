cbuffer ShadowCB : register(b0) {
    matrix world;
    matrix viewProj;
};

struct VS_INPUT {
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

float4 main(VS_INPUT input) : SV_POSITION {
    float4 worldPos = mul(float4(input.pos, 1.0f), world);
    return mul(worldPos, viewProj);
}
