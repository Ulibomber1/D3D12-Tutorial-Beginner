struct VS_Output
{
    float4 pos : SV_Position;
    float2 uv : Texcoord;
};
struct Rotation
{
    matrix transform;
};
ConstantBuffer<Rotation> rot : register(b1);

VS_Output main(float2 pos : POSITION, float2 uv : TEXCOORD)
{
    VS_Output vertexOut;
	
    vertexOut.pos = mul(float4(pos, 0.0f, 1.0f), rot.transform);
    vertexOut.uv = uv;
	
    return vertexOut;
}