#include "Common.hlsli"

Texture2D gAlbedoMetallic : register(t0);
Texture2D gNormalRoughness : register(t1);
Texture2D gEmissiveAO : register(t2);
Texture2D gDepth : register(t3);

SamplerState pointSampler : register(s0);

cbuffer PointLightConstants : register(b0) {
    PointLight pointLight;

    matrix invViewProj;

    float3 cameraPosition;
    float pad;

    matrix worldViewProj;

    float2 screenSize;
    float2 pad2;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
};

float3 ReconstructWorldPosition(float2 uv, float depth, matrix invViewProj) {
    float4 ndc = float4(uv * 2.0 - 1.0, depth, 1.0);
    ndc.y = -ndc.y;
    float4 worldPos = mul(ndc, invViewProj);
    return worldPos.xyz / worldPos.w;
}

float4 main(PS_INPUT input) : SV_TARGET {
    float2 uv = input.pos.xy / screenSize;

    float4 albedoMetallic = gAlbedoMetallic.Sample(pointSampler, uv);
    float4 normalRoughness = gNormalRoughness.Sample(pointSampler, uv);
    float depth = gDepth.Sample(pointSampler, uv).r;

    float3 albedo = albedoMetallic.rgb;
    float metallic = albedoMetallic.a;
    float3 normal = normalize(normalRoughness.rgb * 2.0 - 1.0);
    float roughness = normalRoughness.a;

    // Early out: no geometry here
    if (depth >= 1.0) discard;

    float3 worldPos = ReconstructWorldPosition(uv, depth, invViewProj);

    float3 lightDir = pointLight.position - worldPos;
    float distance = length(lightDir);
    lightDir /= distance;

    if (distance > pointLight.range) discard;

    // Attenuation
    float att = 1.0 / (pointLight.constant + pointLight.linearAttenuation * distance + pointLight.quadratic * distance * distance);
    float rangeFactor = saturate(1.0 - (distance / pointLight.range));
    att *= rangeFactor * rangeFactor;  // Smooth falloff

    if (att < 0.001) discard;

    float NdotL = max(dot(normal, lightDir), 0.0);

    float3 viewDir = normalize(cameraPosition - worldPos);
    float3 halfDir = normalize(lightDir + viewDir);
    float NdotH = max(dot(normal, halfDir), 0.0);

    float specPower = exp2(10 * (1.0 - roughness) + 1);
    float spec = pow(NdotH, specPower) * (metallic > 0.5 ? 1.0 : 0.0);

    float3 diffuse = albedo * NdotL * pointLight.color;
    float3 specular = pointLight.color * spec * metallic;

    float3 lighting = (diffuse + specular) * pointLight.intensity * att;

    return float4(lighting, 1.0);
}
