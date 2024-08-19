#include "RootSignature.hlsl"

[RootSignature(ROOTSIG)]
// takes a 2D position as input, and returns the position in homogeneous 3D space
void main(
    // == IN ==
    in float2 pos : Position,
    in float2 uv : Texcoord,

    // == OUT ==
    out float2 o_uv : Texcoord,
    out float4 o_pos : SV_Position
)
{
    o_pos = float4(pos.xy, 0.0f, 1.0f);
    o_uv = uv;
}

