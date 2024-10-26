struct CubeColors
{
    float4 colors[6];
};

float3 color : register(b0);
ConstantBuffer<CubeColors> cubeColors : register(b2);
Texture2D<float4> textures[] : register(t0);
sampler textureSampler : register(s0);

// takes an RGBA or other 4-float value and writes it to a render target
/*void main(
    // == IN ==
    in float2 uv : Texcoord,

    // == OUT ==
    out float4 pixel : SV_Target
) 
{
    float4 texel = textures[0].Sample(textureSampler, uv);
    pixel = float4(texel.rgb * color, 1.0f);
}*/

float4 main(uint primitiveID : SV_PrimitiveID) : SV_Target
{
    float4 pixel;
    
    pixel = cubeColors.colors[primitiveID >> 1];
    
    return pixel;
}