cbuffer LightBuffer : register(b0) {
    float3 lightDirection;
    float padding;
    float4 lightColor;
    float4 ambientColor;
};

struct PS_INPUT {
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET {
    float3 normal = normalize(input.normal);
    float3 lightDir = normalize(-lightDirection);

    float diff = max(dot(normal, lightDir), 0.0f);
    float4 diffuse = diff * lightColor;

    float4 finalColor = input.color * (ambientColor + diffuse);
    finalColor.a = 1.0f;

    return finalColor;
}
