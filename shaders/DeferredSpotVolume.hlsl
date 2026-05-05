#include "Common.hlsli"

Texture2D gAlbedoMetallic : register(t0);
Texture2D gNormalRoughness : register(t1);
Texture2D gEmissiveAO : register(t2);
Texture2D gDepth : register(t3);

SamplerState pointSampler : register(s0);

struct SpotLight {
    float3 position;
    float intensity;
    float3 color;
    float range;
    float3 direction;
    float innerAngle;
    float outerAngle;
    float constant;
    float linearAttenuation;
    float quadratic;
    float pad[3];
};

cbuffer SpotLightConstants : register(b0) {
    SpotLight spotLight;
    matrix invViewProj;
    float3 cameraPosition;
    float pad;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float3 ReconstructWorldPosition(float2 uv, float depth, matrix invViewProj) {
    float4 ndc = float4(uv * 2.0 - 1.0, depth, 1.0);
    ndc.y = -ndc.y;
    float4 worldPos = mul(ndc, invViewProj);
    return worldPos.xyz / worldPos.w;
}

float4 main(PS_INPUT input) : SV_TARGET {
    float4 albedoMetallic = gAlbedoMetallic.Sample(pointSampler, input.uv);
    float4 normalRoughness = gNormalRoughness.Sample(pointSampler, input.uv);
    float depth = gDepth.Sample(pointSampler, input.uv).r;

    float3 albedo = albedoMetallic.rgb;
    float metallic = albedoMetallic.a;
    float3 normal = normalize(normalRoughness.rgb * 2.0 - 1.0);
    float roughness = normalRoughness.a;

    float3 worldPos = ReconstructWorldPosition(input.uv, depth, invViewProj);

    float3 lightDir = spotLight.position - worldPos;
    float distance = length(lightDir);
    lightDir /= distance;

    if (distance > spotLight.range) discard;

    // Spot angle attenuation
    float cosAngle = dot(-lightDir, normalize(spotLight.direction));
    float innerCos = cos(spotLight.innerAngle);
    float outerCos = cos(spotLight.outerAngle);
    float spotAtt = saturate((cosAngle - outerCos) / (innerCos - outerCos));
    spotAtt = spotAtt * spotAtt;

    // Distance attenuation
    float att = 1.0 / (spotLight.constant + spotLight.linearAttenuation * distance + spotLight.quadratic * distance * distance);
    att *= spotAtt;

    float NdotL = max(dot(normal, lightDir), 0.0);

    float3 viewDir = normalize(cameraPosition - worldPos);
    float3 halfDir = normalize(lightDir + viewDir);
    float NdotH = max(dot(normal, halfDir), 0.0);

    float specPower = exp2(10 * (1.0 - roughness) + 1);
    float spec = pow(NdotH, specPower) * (metallic > 0.5 ? 1.0 : 0.0);

    float3 diffuse = albedo * NdotL * spotLight.color;
    float3 specular = spotLight.color * spec * metallic;

    float3 lighting = (diffuse + specular) * spotLight.intensity * att;

    return float4(lighting, 1.0);
}
