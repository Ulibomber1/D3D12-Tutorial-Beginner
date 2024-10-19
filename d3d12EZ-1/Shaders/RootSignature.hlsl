/*
 *  Root Signature Layout (CPU)
    - 0: float3 "Color" CPU r0 ---> GPU b0
    - 1: ARCorrection "aspect ratio correction"
    - 2: descriptor table "textures" CPU r2 ---> GPU t0
 *  

    Root Signature Layout (GPU)
    - b0: float3 "Color"
    - t0: Texture2D<float4> "textures"
    - s0: Sampler for textures
*/

#define ROOTSIG \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
"RootConstants(num32BitConstants=4, b0)," \
"RootConstants(num32BitConstants=4, b1)," \
"DescriptorTable(" \
    "SRV(t0, numDescriptors = 1)" \
")," \
"StaticSampler(s0)" 

