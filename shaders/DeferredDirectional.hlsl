#include "Common.hlsli"

Texture2D gAlbedoMetallic : register(t0);
Texture2D gNormalRoughness : register(t1);
Texture2D gEmissiveAO : register(t2);
Texture2D gDepth : register(t3);
Texture2DArray<float> shadowMap : register(t4);

SamplerState pointSampler : register(s0);
SamplerComparisonState shadowSampler : register(s1);

cbuffer LightingConstants : register(b0) {
    float3 cameraPosition;
    int cascadeCount;

    DirectionalLight dirLight;

    matrix invProj;

    matrix invViewProj;

    ShadowConstants shadowData;

    float2 screenSize;
    int debugCascades;
    float pad;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

static const float3 cascadeDebugColors[4] = {
    float3(1.0, 0.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 0.0, 1.0),
    float3(1.0, 0.0, 1.0)
};

int GetCascadeIndex(float viewDepth, ShadowConstants shadow) {
    for (int i = 0; i < shadow.cascadeCount; i++) {
        if (viewDepth < shadow.cascades[i].splitDistance)
            return i;
    }
    return shadow.cascadeCount - 1;
}

float SampleShadowPCF(float3 shadowCoord, int cascadeIndex, ShadowConstants shadow, float bias) {
    float2 uv = shadowCoord.xy;
    float compareDepth = shadowCoord.z + bias;
    float result = 0.0;
    float kernel = shadow.pcfKernelSize / shadow.shadowMapSize;

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float2 offset = float2(x, y) * kernel;
            result += shadowMap.SampleCmpLevelZero(
                shadowSampler,
                float3(uv + offset, cascadeIndex),
                compareDepth
            );
        }
    }
    return result / 9.0;
}

float CalculateShadowWithIndex(float3 worldPos, float3 normal, float viewDepth, ShadowConstants shadow, float3 lightDir, out int outCascadeIndex) {
    outCascadeIndex = GetCascadeIndex(viewDepth, shadow);

    float4 shadowCoord = mul(float4(worldPos, 1.0), shadow.cascades[outCascadeIndex].viewProj);
    shadowCoord.xyz /= shadowCoord.w;

    float2 shadowUV = shadowCoord.xy * 0.5 + 0.5;
    shadowUV.y = 1.0 - shadowUV.y;

    float bias = max(shadow.bias * (1.0 - saturate(dot(normal, -lightDir))), 0.0005);

    if (any(shadowUV < 0.0) || any(shadowUV > 1.0) || shadowCoord.z < 0.0 || shadowCoord.z > 1.0) {
        return 1.0;
    }

    return SampleShadowPCF(float3(shadowUV, shadowCoord.z), outCascadeIndex, shadow, bias);
}

float3 ReconstructWorldPosition(float2 uv, float depth, matrix invViewProj) {
    float4 ndc = float4(uv * 2.0 - 1.0, depth, 1.0);
    ndc.y = -ndc.y;
    float4 worldPos = mul(ndc, invViewProj);

    return worldPos.xyz / worldPos.w;
}

float ReconstructViewDepth(float2 uv, float depth, matrix invProj) {
    float4 clipPos = float4(uv * 2.0 - 1.0, depth, 1.0);
    clipPos.y = -clipPos.y;
    float4 viewPos = mul(clipPos, invProj);

    return -viewPos.z / viewPos.w;
}

float4 main(PS_INPUT input) : SV_TARGET {
    float4 albedoMetallic = gAlbedoMetallic.Sample(pointSampler, input.uv);
    float4 normalRoughness = gNormalRoughness.Sample(pointSampler, input.uv);
    float4 emissiveAO = gEmissiveAO.Sample(pointSampler, input.uv);
    float depth = gDepth.Sample(pointSampler, input.uv).r;

    if (depth >= 0.999f) discard;

    float3 albedo = albedoMetallic.rgb;
    float metallic = albedoMetallic.a;
    float3 normal = normalize(normalRoughness.rgb * 2.0 - 1.0);
    float roughness = normalRoughness.a;
    float3 emissive = emissiveAO.rgb;

    float3 worldPos = ReconstructWorldPosition(input.uv, depth, invViewProj);

    // Directional light
    float3 lightDir = normalize(-dirLight.direction);
    float NdotL = max(dot(normal, lightDir), 0.0);

    float3 viewDir = normalize(cameraPosition - worldPos);
    float3 halfDir = normalize(lightDir + viewDir);
    float NdotH = max(dot(normal, halfDir), 0.0);

    // Blinn-Phong with roughness
    float specPower = exp2(10 * (1.0 - roughness) + 1);
    float spec = pow(NdotH, specPower) * metallic;

    float viewDepth = ReconstructViewDepth(input.uv, depth, invProj);

    int cascadeIndex = 0;
    float shadow = CalculateShadowWithIndex(worldPos, normal, viewDepth, shadowData, dirLight.direction, cascadeIndex);

    float3 diffuse = albedo * NdotL * dirLight.color;
    float3 specular = dirLight.color * spec * metallic;

    float3 lighting = (diffuse + specular) * dirLight.intensity * shadow;

    if (debugCascades != 0) {
        float3 debugColor = cascadeDebugColors[cascadeIndex];
        lighting += debugColor * 0.3;
    }

    // Add emissive
    lighting += emissive;

    return float4(lighting, 1.0);
}
