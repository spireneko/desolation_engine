Texture2D particleTexture : register(t0);
Texture2D sceneDepth : register(t1);

SamplerState samplerState : register(s0);
SamplerState pointSampler : register(s1);

cbuffer ParticleConstants : register(b0) {
    matrix viewProj;
    float3 cameraRight;
    float pad0;
    float3 cameraUp;
    float pad1;
    float2 screenSize;
    float pad2[2];
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR;
    float2 screenUV : TEXCOORD1;
};

float4 main(PS_INPUT input) : SV_TARGET {
    float4 texColor = particleTexture.Sample(samplerState, input.uv);

    // Soft particles
    float sceneZ = sceneDepth.Sample(pointSampler, input.screenUV).r;
    float particleZ = input.pos.z / input.pos.w;

    // Reconstruct linear depth difference (simplified)
    float depthDiff = sceneZ - particleZ;
    float softFade = saturate(depthDiff * 50.0); // soft scale

    float alpha = texColor.a * input.color.a * softFade;
    float3 color = texColor.rgb * input.color.rgb;

    return float4(color * alpha, alpha);
}
