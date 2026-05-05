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
