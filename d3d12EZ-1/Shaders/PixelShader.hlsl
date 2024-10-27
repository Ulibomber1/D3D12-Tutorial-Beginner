struct VS_Output1
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

// takes an RGBA or other 4-float value and writes it to a render target
float4 main(VS_Output1 input) : SV_Target
{
    return input.color;
}