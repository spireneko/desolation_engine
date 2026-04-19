Texture2D diffuseTexture : register(t0);
SamplerState samplerState : register(s0);

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

cbuffer ConstantBuffer : register(b0) {
    matrix world, view, projection;

    float3 cameraPosition;
    int pointLightCount;

    DirectionalLight light;

    PointLight pointLights[16];

    Material material;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float4 color : COLOR;
};

float3 CalcDirLight(DirectionalLight light, float3 normal, float3 viewDir, Material mat) {
    float3 lightDir = normalize(-light.direction);
    float3 reflectDir = reflect(-lightDir, normal);

    float3 ambient = light.color * mat.ambient;

    float3 diffuse = max(dot(normal, lightDir), 0.0) * light.color * mat.diffuse;

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
    float3 specular = light.color * mat.specular * spec;

    return (ambient + diffuse + specular) * light.intensity;
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

    float3 lighting = CalcDirLight(light, normal, viewDir, material);

    for (int i = 0; i < pointLightCount; i++) {
        lighting += CalcPointLight(pointLights[i], input.worldPos, normal, viewDir, material);
    }

    float4 texColor = diffuseTexture.Sample(samplerState, input.uv);

    return float4(lighting, 1.0) * texColor * input.color;
}
