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
public:
	GPSODescBuilder();
protected:
	GPSODesc descClass;
	ID3D12RootSignature* pRootSig = nullptr;
	D3D12_INPUT_ELEMENT_DESC* pVertLayout = nullptr;
	UINT _vertLayoutSize = 0;
	Shader *pVertShader=nullptr, * pPixShader=nullptr, * pDomShader=nullptr, * pHullShader=nullptr, * pGeomShader=nullptr;

public:
	virtual void BuildRootSig(ID3D12RootSignature* rootSig) = 0;
	virtual void BuildInputLayout(D3D12_INPUT_ELEMENT_DESC* vertLayout, UINT vertLayoutSize) = 0;
	virtual void BuildIndexBuffer() = 0;
	virtual void BuildShaders(Shader* vertShader, Shader* pixShader, Shader* domShader, Shader* hullShader, Shader* geomShader) = 0;
	virtual void BuildRasterizer() = 0;
	virtual void BuildStreamOutput() = 0;
	virtual void BuildRTV() = 0;
	virtual void BuildDSV() = 0;
	virtual void BuildBlendState() = 0;
	virtual void BuildDepthStencil() = 0;
	virtual void BuildSample() = 0;
	virtual void BuildPSODetails() = 0;

	virtual D3D12_GRAPHICS_PIPELINE_STATE_DESC GetDescriptor() = 0;

	friend class GPSODescDirector;
};

class GPSODescBuilder2D : public GPSODescBuilder 
{
public:
	GPSODescBuilder2D(ID3D12RootSignature* rs, D3D12_INPUT_ELEMENT_DESC* vl, UINT vls, Shader* vs, Shader* ps, Shader* ds, Shader* hs, Shader* gs);
public:
	void BuildRootSig(ID3D12RootSignature* rootSig) override;
	void BuildInputLayout(D3D12_INPUT_ELEMENT_DESC* vertLayout, UINT vertLayoutSize) override;
	void BuildIndexBuffer() override;
	void BuildShaders(Shader* vertShader, Shader* pixShader, Shader* domShader, Shader* hullShader, Shader* geomShader) override;
	void BuildRasterizer() override;
	void BuildStreamOutput() override;
	void BuildRTV() override;
	void BuildDSV() override;
	void BuildBlendState() override;
	void BuildDepthStencil() override;
	void BuildSample() override;
	void BuildPSODetails() override;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetDescriptor() override;
};

class GPSODescBuilder3D : public GPSODescBuilder
{
	GPSODescBuilder3D(ID3D12RootSignature* rs, D3D12_INPUT_ELEMENT_DESC* vl, UINT vls, Shader* vs, Shader* ps, Shader* ds, Shader* hs, Shader* gs);
public:
	void BuildRootSig(ID3D12RootSignature* rootSig) override;
	void BuildInputLayout(D3D12_INPUT_ELEMENT_DESC* vertLayout, UINT vertLayoutSize) override;
	void BuildIndexBuffer() override;
	void BuildShaders(Shader* vertShader, Shader* pixShader, Shader* domShader, Shader* hullShader, Shader* geomShader) override;
	void BuildRasterizer() override;
	void BuildStreamOutput() override;
	void BuildRTV() override;
	void BuildDSV() override;
	void BuildBlendState() override;
	void BuildDepthStencil() override;
	void BuildSample() override;
	void BuildPSODetails() override;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetDescriptor() override;
};

class GPSODescDirector
{
public:
	void Construct(GPSODescBuilder& builder);
};