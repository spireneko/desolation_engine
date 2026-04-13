Texture2D diffuseTexture : register(t0);
SamplerState samplerState : register(s0);

struct DirectionalLight {
    float3 direction;
    float intensity;

	float3 color;
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
    float padding;
    DirectionalLight light;
    Material material;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float4 color : COLOR;
};

float4 main(PS_INPUT input) : SV_Target {
    float3 normal = normalize(input.normal);
    float3 lightDir = normalize(-light.direction);
    float3 viewDir = normalize(cameraPosition - input.worldPos);
    float3 reflectDir = reflect(-lightDir, normal);

    float3 ambient = light.color * material.ambient;

    float3 diffuse = max(dot(normal, lightDir), 0.0) * light.color * material.diffuse;

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    float3 specular = light.color * material.specular * spec;

    float3 lighting = (ambient + diffuse + specular) * light.intensity;
    float4 texColor = diffuseTexture.Sample(samplerState, input.uv);

    return float4(lighting, 1.0) * texColor * input.color;
}
