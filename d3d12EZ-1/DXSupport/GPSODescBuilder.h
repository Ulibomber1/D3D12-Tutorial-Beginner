#pragma once
#include <WinSupport/WinInclude.h>
#include <DXSupport/Shader.h>

// Product
class GPSODesc
{
public:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
};

// Builder Interface
class GPSODescBuilder
{
	GPSODescBuilder(ID3D12RootSignature* rs, D3D12_INPUT_ELEMENT_DESC* vl, UINT vls, Shader* vs, Shader* ps, Shader* ds, Shader* hs, Shader* gs);
protected:
	GPSODesc descClass;
	ID3D12RootSignature* pRootSig = nullptr;
	D3D12_INPUT_ELEMENT_DESC* pVertLayout = nullptr;
	UINT _vertLayoutSize = 0;
	Shader *pVertShader=nullptr, * pPixShader=nullptr, * pDomShader=nullptr, * pHullShader=nullptr, * pGeomShader=nullptr;

public:
	virtual void buildRootSig(ID3D12RootSignature* rootSig) = 0;
	virtual void buildInputLayout(D3D12_INPUT_ELEMENT_DESC* vertLayout, UINT vertLayoutSize) = 0;
	virtual void buildIndexBuffer() = 0;
	virtual void buildShaders(Shader* vertShader, Shader* pixShader, Shader* domShader, Shader* hullShader, Shader* geomShader) = 0;
	virtual void buildRasterizer() = 0;
	virtual void buildStreamOutput() = 0;
	virtual void buildRTV() = 0;
	virtual void buildDSV() = 0;
	virtual void buildBlendState() = 0;
	virtual void buildDepthStencil() = 0;
	virtual void buildSample() = 0;
	virtual void buildPSODetails() = 0;

	virtual D3D12_GRAPHICS_PIPELINE_STATE_DESC getDescriptor() = 0;

	friend class GPSODescDirector;
};

class GPSODescBuilder2D : public GPSODescBuilder 
{
public:
	void buildRootSig(ID3D12RootSignature* rootSig) override;
	void buildInputLayout(D3D12_INPUT_ELEMENT_DESC* vertLayout, UINT vertLayoutSize) override;
	void buildIndexBuffer() override;
	void buildShaders(Shader* vertShader, Shader* pixShader, Shader* domShader, Shader* hullShader, Shader* geomShader) override;
	void buildRasterizer() override;
	void buildStreamOutput() override;
	void buildRTV() override;
	void buildDSV() override;
	void buildBlendState() override;
	void buildDepthStencil() override;
	void buildSample() override;
	void buildPSODetails() override;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC getDescriptor() override;
};

class GPSODescBuilder3D : public GPSODescBuilder
{
public:
	void buildRootSig(ID3D12RootSignature* rootSig) override;
	void buildInputLayout(D3D12_INPUT_ELEMENT_DESC* vertLayout, UINT vertLayoutSize) override;
	void buildIndexBuffer() override;
	void buildShaders(Shader* vertShader, Shader* pixShader, Shader* domShader, Shader* hullShader, Shader* geomShader) override;
	void buildRasterizer() override;
	void buildStreamOutput() override;
	void buildRTV() override;
	void buildDSV() override;
	void buildBlendState() override;
	void buildDepthStencil() override;
	void buildSample() override;
	void buildPSODetails() override;
};

class GPSODescDirector
{
public:
	void construct(GPSODescBuilder& builder);
};