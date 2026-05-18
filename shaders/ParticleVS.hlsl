cbuffer ParticleConstants : register(b0) {
    matrix viewProj;
    float3 cameraRight;
    float pad0;
    float3 cameraUp;
    float pad1;
    float2 screenSize;
    float pad2[2];
};

struct ParticleData {
    float3 position;
    float size;
    float4 color;
    float rotation;
    float pad[3];
};

StructuredBuffer<ParticleData> particleBuffer : register(t0);

struct VS_OUTPUT {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR;
    float2 screenUV : TEXCOORD1;
};

static const float2 quadUVs[4] = {
    float2(0, 1), float2(0, 0), float2(1, 1), float2(1, 0)
};

static const float2 quadOffsets[4] = {
    float2(-0.5, -0.5), float2(-0.5, 0.5), float2(0.5, -0.5), float2(0.5, 0.5)
};

VS_OUTPUT main(uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID) {
    VS_OUTPUT output;

    ParticleData p = particleBuffer[instanceId];

    float2 offset = quadOffsets[vertexId];
    float2 uv = quadUVs[vertexId];

    // Rotate
    float c = cos(p.rotation);
    float s = sin(p.rotation);
    float2 rotated = float2(
        offset.x * c - offset.y * s,
        offset.x * s + offset.y * c
    );

    // Billboard
    float3 worldPos = p.position + (cameraRight * rotated.x + cameraUp * rotated.y) * p.size;

    output.pos = mul(float4(worldPos, 1.0), viewProj);
    output.uv = uv;
    output.color = p.color;
    output.screenUV = output.pos.xy / output.pos.w * 0.5 + 0.5;
    output.screenUV.y = 1.0 - output.screenUV.y;

    return output;
}
