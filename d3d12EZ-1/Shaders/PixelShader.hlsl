#include "RootSignature.hlsl"

float3 color : register(b0);

[RootSignature(ROOTSIG)]
// takes an RGBA or other 4-float value and writes it to a render target
float4 main() : SV_Target
{
    return float4(color, 1.0f);
}