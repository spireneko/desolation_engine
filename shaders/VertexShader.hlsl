cbuffer ConstantBuffer : register(b0) {
    matrix world;
    matrix view;
    matrix projection;
};

struct VS_INPUT {
    float3 position : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
};

struct PS_INPUT {
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD0;
};

PS_INPUT main(VS_INPUT input) {
    PS_INPUT output;

    float4 worldPos = mul(float4(input.position, 1.0f), world);
    output.worldPos = worldPos.xyz;
    output.position = mul(worldPos, view);
    output.position = mul(output.position, projection);

    output.color = input.color;
    output.normal = mul(input.normal, (float3x3)world);

    return output;
}
