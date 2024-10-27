struct VS_Output
{
    float2 uv : Texcoord;
    float4 pos : SV_Position;
};

VS_Output main(float2 pos : POSITION, float2 uv : TEXCOORD)
{
    VS_Output vertexOut;
	
    vertexOut.pos = float4(pos, 0.0f, 1.0f);
    vertexOut.uv = uv;
	
    return vertexOut;
}