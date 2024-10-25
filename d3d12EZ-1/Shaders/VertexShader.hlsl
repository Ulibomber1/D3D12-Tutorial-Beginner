struct VS_Output
{
	float2 uv : Texcoord;
	float4 pos : SV_Position;
};
struct VS_Output1
{
	float4 col : Color;
	float4 pos : SV_Position;
};
struct Rotation
{
	matrix transform;
};
ConstantBuffer<Rotation> rot : register(b1);
ConstantBuffer<Rotation> rot1 : register(b2);

VS_Output main(float2 pos : POSITION, float2 uv : TEXCOORD)
{
	VS_Output vertexOut;
	
	vertexOut.pos = mul(float4(pos, 0.0f, 1.0f), rot.transform);
	vertexOut.uv = uv;
	
	return vertexOut;
}

/*VS_Output1 main(float3 pos : POSITION, float3 color : COLOR)
{
	VS_Output1 vertexOut;
		
    vertexOut.pos = mul(float4(pos, 1.0f), rot1.transform);
    vertexOut.col = float4(color, 1.0f);
	
	return vertexOut;
}*/
