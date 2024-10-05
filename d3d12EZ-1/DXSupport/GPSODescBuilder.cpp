#include "GPSODescBuilder.h"

// === Builder Constructor ===
GPSODescBuilder::GPSODescBuilder(ID3D12RootSignature* rs, D3D12_INPUT_ELEMENT_DESC* vl, UINT vls, Shader* vs, Shader* ps, Shader* ds, Shader* hs, Shader* gs)
{
	descClass = GPSODesc();
	pRootSig = rs;
	pVertLayout = vl;
	_vertLayoutSize = vls;
	pVertShader = vs;
	pPixShader = ps;
	pDomShader = ds;
	pHullShader = hs;
	pGeomShader = gs;
}

// === 2D PL Description Builder ===
void GPSODescBuilder2D::buildRootSig(ID3D12RootSignature* rootSig)
{
	descClass.desc.pRootSignature = rootSig;
}
void GPSODescBuilder2D::buildInputLayout(D3D12_INPUT_ELEMENT_DESC* vertLayout, UINT vertLayoutSize)
{
	descClass.desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	descClass.desc.InputLayout.NumElements = vertLayoutSize;
	descClass.desc.InputLayout.pInputElementDescs = vertLayout;
}
void GPSODescBuilder2D::buildIndexBuffer()
{
	descClass.desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
}
void GPSODescBuilder2D::buildShaders(Shader* vertShader, Shader* pixShader, Shader* domShader, Shader* hullShader, Shader* geomShader)
{
	descClass.desc.VS.BytecodeLength = vertShader != nullptr ? vertShader->GetSize() : 0;
	descClass.desc.VS.pShaderBytecode = vertShader != nullptr ? vertShader->GetBuffer() : nullptr;

	descClass.desc.PS.BytecodeLength = pixShader != nullptr ? pixShader->GetSize() : 0;
	descClass.desc.PS.pShaderBytecode = pixShader != nullptr ? pixShader->GetBuffer() : nullptr;
	
	descClass.desc.DS.BytecodeLength = domShader != nullptr ? domShader->GetSize() : 0;
	descClass.desc.DS.pShaderBytecode = domShader != nullptr ? domShader->GetBuffer() : nullptr;

	descClass.desc.HS.BytecodeLength = hullShader != nullptr ? hullShader->GetSize() : 0;
	descClass.desc.HS.pShaderBytecode = hullShader != nullptr ? hullShader->GetBuffer() : nullptr;

	descClass.desc.GS.BytecodeLength = geomShader != nullptr ? geomShader->GetSize() : 0;
	descClass.desc.GS.pShaderBytecode = geomShader != nullptr ? geomShader->GetBuffer() : nullptr;
}
void GPSODescBuilder2D::buildRasterizer()
{
	descClass.desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // define the fill mode, which defines how the drawn objects are filled in for enclosed faces
	descClass.desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // define what faces are culled, if at all
	descClass.desc.RasterizerState.FrontCounterClockwise = FALSE; // define which face is the front
	descClass.desc.RasterizerState.DepthBias = 0; // depth bias will attempt to push drawn objects forward
	descClass.desc.RasterizerState.DepthBiasClamp = .0f;
	descClass.desc.RasterizerState.SlopeScaledDepthBias = .0f;
	descClass.desc.RasterizerState.DepthClipEnable = FALSE; // set whether z-clipping can occur
	descClass.desc.RasterizerState.MultisampleEnable = FALSE; // set whether multisample will be on
	descClass.desc.RasterizerState.AntialiasedLineEnable = FALSE; // set whether anti-aliasing is on
	descClass.desc.RasterizerState.ForcedSampleCount = 0; // the sample count for anti-aliasing
}
void GPSODescBuilder2D::buildStreamOutput()
{
	descClass.desc.StreamOutput.NumEntries = 0; // StreamOutput is a layer that streams transformed vertex data back to memory resources. We do not use it here.
	descClass.desc.StreamOutput.NumStrides = 0;
	descClass.desc.StreamOutput.pBufferStrides = nullptr;
	descClass.desc.StreamOutput.pSODeclaration = nullptr;
	descClass.desc.StreamOutput.RasterizedStream = 0;
}
void GPSODescBuilder2D::buildRTV()
{
	descClass.desc.NumRenderTargets = 1;
	descClass.desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
}
void GPSODescBuilder2D::buildDSV()
{
	descClass.desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
}
void GPSODescBuilder2D::buildBlendState()
{
	descClass.desc.BlendState.AlphaToCoverageEnable = FALSE;
	descClass.desc.BlendState.IndependentBlendEnable = FALSE;
	descClass.desc.BlendState.RenderTarget[0].BlendEnable = TRUE;
	descClass.desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	descClass.desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	descClass.desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	descClass.desc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
	descClass.desc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	descClass.desc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	descClass.desc.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
	descClass.desc.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	descClass.desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
}
void GPSODescBuilder2D::buildDepthStencil()
{
	descClass.desc.DepthStencilState.DepthEnable = FALSE;
	descClass.desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	descClass.desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	descClass.desc.DepthStencilState.StencilEnable = FALSE;
	descClass.desc.DepthStencilState.StencilReadMask = 0;
	descClass.desc.DepthStencilState.StencilWriteMask = 0;
	descClass.desc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	descClass.desc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	descClass.desc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	descClass.desc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	descClass.desc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	descClass.desc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	descClass.desc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	descClass.desc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
}
void GPSODescBuilder2D::buildSample()
{
	descClass.desc.SampleMask = 0xFFFFFFFF;
	descClass.desc.SampleDesc.Count = 1;
	descClass.desc.SampleDesc.Quality = 0;
}
void GPSODescBuilder2D::buildPSODetails()
{
	descClass.desc.NodeMask = 0;
	descClass.desc.CachedPSO.CachedBlobSizeInBytes = 0;
	descClass.desc.CachedPSO.pCachedBlob = nullptr;
	descClass.desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE; // start with the none flag
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC GPSODescBuilder2D::getDescriptor()
{
	return descClass.desc;
}

// === 3D PL Description Builder ===
void GPSODescBuilder3D::buildRootSig(ID3D12RootSignature* rootSig)
{
	descClass.desc.pRootSignature = rootSig;
}

// === PL Description Builder ===
void GPSODescDirector::construct(GPSODescBuilder& builder)
{
	builder.buildRootSig(builder.pRootSig);
	builder.buildInputLayout(builder.pVertLayout, builder._vertLayoutSize);
	builder.buildIndexBuffer();
	builder.buildShaders(builder.pVertShader, builder.pPixShader, builder.pDomShader, builder.pHullShader, builder.pGeomShader);
	builder.buildRasterizer();
	builder.buildStreamOutput();
	builder.buildRTV();
	builder.buildDSV();
	builder.buildBlendState();
	builder.buildDepthStencil();
	builder.buildSample();
	builder.buildPSODetails();
}	


