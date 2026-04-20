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
    float4x4 viewProj;
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

struct VS_INPUT {
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
    float viewDepth : TEXCOORD1;
};

PS_INPUT main(VS_INPUT input) {
    PS_INPUT output;

    float4 worldPos = mul(float4(input.pos, 1.0f), world);
    output.worldPos = worldPos.xyz;

    float4 viewPos = mul(worldPos, view);
    output.viewDepth = viewPos.z;

    output.pos = mul(viewPos, projection);

    output.normal = mul(float4(input.normal, 0.0f), world).xyz;

    output.uv = input.uv;

    output.color = input.color;

    return output;
}
