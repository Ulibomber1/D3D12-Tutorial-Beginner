struct VS_Output1
{
	float4 pos : SV_Position;
    float4 color : COLOR;
};
struct Rotation
{
	matrix transform;
};
ConstantBuffer<Rotation> rot : register(b1);

/**/

VS_Output1 main(float3 pos : POSITION, float3 color : COLOR)
{
	VS_Output1 vertexOut;
		
    vertexOut.pos = mul(float4(pos, 1.0f), rot.transform);
    vertexOut.color = float4(color, 1.0f);
	
	return vertexOut;
}
