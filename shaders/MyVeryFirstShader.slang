// Входные данные вершинного шейдера
struct VSIn
{
    float4 pos : POSITION0;
    float4 col : COLOR0;
};

// Выходные данные вершинного шейдера / входные для пиксельного
struct PSIn
{
    float4 pos : SV_Position;
    float4 col : COLOR;
};

// Параметры, передаваемые извне (uniforms)
cbuffer Params
{
    float4 gTColor;  // TEST макрос заменён на runtime-переменную
};

// Вершинный шейдер
[shader("vertex")]
PSIn vsMain(VSIn input)
{
    PSIn output;
    output.pos = input.pos;
    output.col = input.col;
    return output;
}

[SpecializationConstant]
const bool TEST = false;

// Пиксельный шейдер
[shader("fragment")]
float4 psMain(PSIn input) : SV_Target
{
    float4 col = input.col;
    if (TEST && input.pos.x > 400.0) {
        col = gTColor;
	}
    return col;
}
