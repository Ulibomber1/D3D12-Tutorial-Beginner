struct VS_Output
{
    float2 uv : Texcoord;
    float4 pos : SV_Position;
};

float3 color : register(b0);
Texture2D<float4> textures[1] : register(t0);
sampler textureSampler : register(s0);

// takes an RGBA or other 4-float value and writes it to a render target
float4 main(VS_Output input) : SV_Target
{
    float4 texel = textures[0].Sample(textureSampler, input.uv);
    float4 pixel = float4(texel.rgb * color, 1.0f);
    
	return pixel;
}