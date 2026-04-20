Texture2D diffuseTexture : register(t0);
SamplerState samplerState : register(s0);
Texture2DArray<float> shadowMap : register(t1);
SamplerComparisonState shadowSampler : register(s1);

struct DirectionalLight {
    float3 direction;
    float intensity;

	float3 color;
    float pad0;
};

struct PointLight {
    float3 position;
    float intensity;

    float3 color;
    float range;

    float constant;
	float linearAttenuation;
	float quadratic;
    float pad0;
};


struct Material {
	float3 ambient;
    float pad0;

	float3 diffuse;
    float pad1;

	float3 specular;
	float shininess;
};

struct CascadeData {
    matrix viewProj;

    float splitDistance;
    float3 pad;
};

struct ShadowConstants {
    CascadeData cascades[4];

    int cascadeCount;
    float shadowMapSize;
    float pcfKernelSize;
    float bias;
};

cbuffer ConstantBuffer : register(b0) {
    matrix world, view, projection;

    float3 cameraPosition;
    int pointLightCount;

    DirectionalLight light;

    PointLight pointLights[16];

    Material material;

    ShadowConstants shadowData;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float4 color : COLOR;
    float viewDepth : TEXCOORD1;
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

float CalculateShadow(float3 worldPos, float3 normal, float viewDepth, ShadowConstants shadow, float3 lightDir) {
     int cascadeIndex = GetCascadeIndex(viewDepth, shadow);
    float4 shadowCoord = mul(float4(worldPos, 1.0), shadow.cascades[cascadeIndex].viewProj);
    shadowCoord.xyz /= shadowCoord.w;

    float2 shadowUV = shadowCoord.xy * 0.5 + 0.5;
    shadowUV.y = 1.0 - shadowUV.y;

    float shadowZ = shadowCoord.z;

    float bias = max(0.005 * (1.0 - dot(normal, -lightDir)), 0.0005);

    if (any(shadowUV < 0.0) || any(shadowUV > 1.0) || shadowZ < 0.0 || shadowZ > 1.0) {
        return 1.0;
    }

    return SampleShadowPCF(float3(shadowUV, shadowZ), cascadeIndex, shadow, bias);
}

float3 CalcDirLight(DirectionalLight light, float3 normal, float3 viewDir, Material mat, float shadow) {
    float3 lightDir = normalize(-light.direction);
    float3 reflectDir = reflect(-lightDir, normal);

    float3 ambient = light.color * mat.ambient;

    float3 diffuse = max(dot(normal, lightDir), 0.0) * light.color * mat.diffuse;

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
    float3 specular = light.color * mat.specular * spec;

    return (ambient + (diffuse + specular) * shadow) * light.intensity;
}

float3 CalcPointLight(PointLight light, float3 worldPos, float3 normal, float3 viewDir, Material mat) {
    float3 lightDir = normalize(light.position - worldPos);
    float3 reflectDir = reflect(-lightDir, normal);
    float distance = length(light.position - worldPos);

    float att = 1.0 / (light.constant + light.linearAttenuation * distance + light.quadratic * distance * distance);
    float rangeFactor = saturate(1.0 - (distance / light.range));
    att *= rangeFactor;

    float3 diffuse = max(dot(normal, lightDir), 0.0) * light.color * mat.diffuse;

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
    float3 specular = light.color * mat.specular * spec;

    return (diffuse + specular) * light.intensity * att;
}


float4 main(PS_INPUT input) : SV_Target {
    float3 normal = normalize(input.normal);
    float3 viewDir = normalize(cameraPosition - input.worldPos);

    float shadow = CalculateShadow(input.worldPos, normal, input.viewDepth, shadowData, light.direction);

    float3 lighting = CalcDirLight(light, normal, viewDir, material, shadow);

    for (int i = 0; i < pointLightCount; i++) {
        lighting += CalcPointLight(pointLights[i], input.worldPos, normal, viewDir, material);
    }

    float4 texColor = diffuseTexture.Sample(samplerState, input.uv);

    return float4(lighting, 1.0) * texColor * input.color;
}
