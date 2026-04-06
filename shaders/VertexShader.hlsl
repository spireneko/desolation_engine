cbuffer ConstantBuffer : register(b0) {
    matrix world;
    matrix view;
    matrix projection;
};

struct VS_INPUT {
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

PS_INPUT main(VS_INPUT input) {
    PS_INPUT output;
    float4 worldPos = mul(float4(input.pos, 1.0f), world);
    output.pos = mul(worldPos, view);
    output.pos = mul(output.pos, projection);
    output.normal = mul(float4(input.normal, 0.0f), world).xyz;
    output.uv = input.uv;
    output.color = input.color;
    return output;
}
