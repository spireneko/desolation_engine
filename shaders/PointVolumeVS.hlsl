cbuffer VolumeConstants : register(b0) {
    matrix worldViewProj;
    float3 lightPosition;
    float lightRange;
};

struct VS_OUTPUT {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 viewRay : TEXCOORD1;
};

VS_OUTPUT main(float3 pos : POSITION) {
    VS_OUTPUT output;
    output.pos = mul(float4(pos * lightRange + lightPosition, 1.0), worldViewProj);
    output.uv = output.pos.xy * 0.5 + 0.5;
    output.viewRay = pos;
    return output;
}
