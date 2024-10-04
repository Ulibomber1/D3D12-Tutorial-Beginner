#include "GPSODescBuilder.h"

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

// === 3D PL Description Builder ===
void GPSODescBuilder3D::buildRootSig(ID3D12RootSignature* rootSig)
{
	descClass.desc.pRootSignature = rootSig;
}

// === PL Description Builder ===
void GPSODescDirector::construct(GPSODescBuilder& builder)
{
	builder.buildRootSig(builder.);
	builder.buildInputLayout();
	builder.buildIndexBuffer();
	builder.buildShaders();
	builder.buildRasterizer();
	builder.buildStreamOutput();
	builder.buildRTV();
	builder.buildDSV();
	builder.buildBlendState();
	builder.buildDepthStencil();
	builder.buildSample();
	builder.buildPSODetails();
}	

GPSODescBuilder::GPSODescBuilder(ID3D12RootSignature* rs, D3D12_INPUT_ELEMENT_DESC* vl, UINT vls, Shader* vs, Shader* ps, Shader* ds, Shader* hs, Shader* gs)
{
	pRootSig = rs;
}
