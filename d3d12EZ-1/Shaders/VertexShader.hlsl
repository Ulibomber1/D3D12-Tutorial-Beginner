struct VS_Output
{
	float2 uv : Texcoord;
	float4 pos : SV_Position;
};
struct VS_Output1
{
	float4 pos : SV_Position;
};
struct Rotation
{
	matrix transform;
};
ConstantBuffer<Rotation> rot : register(b1);

/*VS_Output main(float2 pos : POSITION, float2 uv : TEXCOORD)
{
	VS_Output vertexOut;
	
	vertexOut.pos = mul(float4(pos, 0.0f, 1.0f), rot.transform);
	vertexOut.uv = uv;
	
	return vertexOut;
}*/

VS_Output1 main(float3 pos : POSITION)
{
	VS_Output1 vertexOut;
		
    vertexOut.pos = mul(float4(pos, 1.0f), rot.transform);
	
	return vertexOut;
}
