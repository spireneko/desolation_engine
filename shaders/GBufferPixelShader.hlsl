#include "Common.hlsli"

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float4 color : COLOR;
    float viewDepth : TEXCOORD1;
};

struct GBufferOutput {
    float4 albedoMetallic : SV_TARGET0;
    float4 normalRoughness : SV_TARGET1;
    float4 emissiveAO : SV_TARGET2;
};

Texture2D diffuseTexture : register(t0);
SamplerState samplerState : register(s0);

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
GBufferOutput main(PS_INPUT input) {
    GBufferOutput output;

    float4 texColor = diffuseTexture.Sample(samplerState, input.uv);
    float3 albedo = texColor.rgb * input.color.rgb;

    float metallic = saturate(length(material.specular));
    float roughness = saturate(1.0 - (material.shininess / 256.0));

    float3 emissive = material.ambient * 0.5;

    output.albedoMetallic = float4(albedo, metallic);
    output.normalRoughness = float4(input.normal * 0.5 + 0.5, roughness);
    output.emissiveAO = float4(emissive, 1.0);

    return output;
}
