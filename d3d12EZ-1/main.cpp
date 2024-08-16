#include <iostream>
#include <math.h> 

#include <WinSupport/WinInclude.h>
#include <WinSupport/ComPointer.h>
#include <DXSupport/ImageLoader.h>
#include <DXSupport/DXWindow.h>
#include <DXSupport/Shader.h>
#include <DXDebug/DXDebugLayer.h>

#include <DXSupport/DXContext.h>

void initHeapPropsUpload(D3D12_HEAP_PROPERTIES*);
void initHeapPropsDefault(D3D12_HEAP_PROPERTIES*);
void initRsrcDescUpload(D3D12_RESOURCE_DESC*, uint32_t);
void initRsrcDescVertex(D3D12_RESOURCE_DESC*);
void initPipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC* gfxpsd, ID3D12RootSignature* rootSig, D3D12_INPUT_ELEMENT_DESC* verLayout, unsigned long long verLayoutSize, Shader* verShader, Shader* pixShader);
void uncrnVomit(float* color, float delta);
void initResourceDescTexture(D3D12_RESOURCE_DESC* rscDesc, ImageLoader::ImageData* txtrData);

int main()
{
	DXDebugLayer::Get().Init();
	if (DXContext::Get().Init() && DXWindow::Get().Init())
	{
		D3D12_HEAP_PROPERTIES hpUpload{};
		initHeapPropsUpload(&hpUpload);

		D3D12_HEAP_PROPERTIES hpDefault{};
		initHeapPropsDefault(&hpDefault);

		// == Vertex Data == 
		struct Vertex
		{
			float x, y;
		};
		Vertex vertices[] =
		{
			{ -1.f, -1.f},
			{  0.f,  1.f},
			{  1.f, -1.f},
		};
		D3D12_INPUT_ELEMENT_DESC vertexLayout[] =
		{
			{"Position", 0,  DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		};

		// == Texture Data ==
		ImageLoader::ImageData textureData;
		ImageLoader::LoadImageFromDisk("./auge_512_512_BGRA_32BPP.png", textureData);
		uint32_t textureStride = textureData.width * ((textureData.bpp + 7) / 8);
		uint32_t textureSize = textureData.height * textureStride;

		// == Upload & Vertex Buffer Description ==
		D3D12_RESOURCE_DESC rdu{};
		initRsrcDescUpload(&rdu, textureSize);

		D3D12_RESOURCE_DESC rdv{};
		initRsrcDescVertex(&rdv);

		ComPointer<ID3D12Resource2> uploadBuffer, vertexBuffer;
		DXContext::Get().GetDevice()->CreateCommittedResource(&hpUpload, D3D12_HEAP_FLAG_NONE, &rdu, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));
		DXContext::Get().GetDevice()->CreateCommittedResource(&hpDefault, D3D12_HEAP_FLAG_NONE, &rdv, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&vertexBuffer));

		// == Texture Description ==
		D3D12_RESOURCE_DESC rdt{};
		initResourceDescTexture(&rdt, &textureData);

		ComPointer<ID3D12Resource2> texture;
		DXContext::Get().GetDevice()->CreateCommittedResource(&hpDefault, D3D12_HEAP_FLAG_NONE, &rdt, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&texture));

		// copy void* --> CPU resource
		char* uploadBufferAddress;
		D3D12_RANGE uploadRange;
		uploadRange.Begin = 0;
		uploadRange.End = 1024 + textureSize;
		uploadBuffer->Map(0, &uploadRange, (void**)& uploadBufferAddress);
		memcpy(&uploadBufferAddress[0], textureData.data.data(), textureSize);
		memcpy(&uploadBufferAddress[textureSize], vertices, sizeof(vertices));
		uploadBuffer->Unmap(0, &uploadRange);

		// copy CPU resource --> GPU resource
		auto* cmdList = DXContext::Get().InitCommandList();
		cmdList->CopyBufferRegion(vertexBuffer, 0, uploadBuffer, textureSize, 1024);

		D3D12_BOX textureSizeAsBox;
		textureSizeAsBox.left = textureSizeAsBox.top = textureSizeAsBox.front = 0;
		textureSizeAsBox.right = textureData.width;
		textureSizeAsBox.bottom = textureData.height;
		textureSizeAsBox.back = 1;

		D3D12_TEXTURE_COPY_LOCATION txtcSrc, txtcDst;
		txtcSrc.pResource = uploadBuffer;
		txtcSrc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		txtcSrc.PlacedFootprint.Offset = 0;
		txtcSrc.PlacedFootprint.Footprint.Width = textureData.width;
		txtcSrc.PlacedFootprint.Footprint.Height = textureData.height;
		txtcSrc.PlacedFootprint.Footprint.Depth = 1;
		txtcSrc.PlacedFootprint.Footprint.RowPitch = textureStride;
		txtcSrc.PlacedFootprint.Footprint.Format = textureData.giPixelFormat;
		txtcDst.pResource = texture;
		txtcDst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		txtcDst.PlacedFootprint.Offset = 0;

		cmdList->CopyTextureRegion(&txtcDst, 0, 0, 0, &txtcSrc, &textureSizeAsBox);
		DXContext::Get().ExecuteCommandList();

		// == Shaders ==
		Shader rootSignatureShader("RootSignature.cso");
		Shader vertexShader("VertexShader.cso");
		Shader pixelShader("PixelShader.cso");

		// == Create Root Sig ==
		ComPointer<ID3D12RootSignature> rootSignature;
		DXContext::Get().GetDevice()->CreateRootSignature(0, rootSignatureShader.GetBuffer(), rootSignatureShader.GetSize(), IID_PPV_ARGS(&rootSignature));

		// == Pipeline State Description ==
		D3D12_GRAPHICS_PIPELINE_STATE_DESC gfxPsod{};
		initPipelineState(&gfxPsod, rootSignature, vertexLayout, _countof(vertexLayout), &vertexShader, &pixelShader); // we pass vertexLayout array since it will decay to a pointer anyways

		ComPointer<ID3D12PipelineState> pso;
		DXContext::Get().GetDevice()->CreateGraphicsPipelineState(&gfxPsod, IID_PPV_ARGS(&pso));

		// == Vertex Buffer View ==
		D3D12_VERTEX_BUFFER_VIEW vbv{};
		vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vbv.SizeInBytes = sizeof(vertices) * _countof(vertices);
		vbv.StrideInBytes = sizeof(Vertex);

		DXWindow::Get().SetFullscreen(true);
		while (!DXWindow::Get().ShouldClose())
		{
			DXWindow::Get().Update(); // Poll the window, so that it's considered 'responding'

			// handle resizing
			if (DXWindow::Get().ShouldResize())
			{
				DXContext::Get().Flush(DXWindow::Get().GetFrameCount()); // Flush Command queue 
				DXWindow::Get().Resize();
			}

			// begin drawing
			cmdList = DXContext::Get().InitCommandList();

			DXWindow::Get().BeginFrame(cmdList);
			
			// == Pipeline State Object ==
			cmdList->SetPipelineState(pso);
			cmdList->SetGraphicsRootSignature(rootSignature);

			// == Input Assembler ==
			cmdList->IASetVertexBuffers(0, 1, &vbv);
			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// == Rasterizer ==
			D3D12_VIEWPORT vp;
			vp.TopLeftX = vp.TopLeftY = 0;
			vp.Width = DXWindow::Get().GetWidth();
			vp.Height = DXWindow::Get().GetHeight();
			vp.MinDepth = 1.f;
			vp.MaxDepth = 0.f;
			cmdList->RSSetViewports(1, &vp);
			RECT scRect;
			scRect.left = scRect.top = 0;
			scRect.right = DXWindow::Get().GetWidth();
			scRect.bottom = DXWindow::Get().GetHeight();
			cmdList->RSSetScissorRects(1, &scRect);

			// == Root Sig ==
			static float color[] = { 0.0f, 0.0f, 1.0f };
			uncrnVomit(color, 0.0025f);
			cmdList->SetGraphicsRoot32BitConstants(0, 3, color, 0);
			
			// Draw
			cmdList->DrawInstanced(_countof(vertices), 1, 0, 0);

			DXWindow::Get().EndFrame(cmdList);

			// Finish and Show the render
			DXContext::Get().ExecuteCommandList();
			DXWindow::Get().Present();
		}

		// Flush command queue
		DXContext::Get().Flush(DXWindow::Get().GetFrameCount());

		// Release Buffers
		vertexBuffer.Release();
		uploadBuffer.Release();

		DXWindow::Get().Shutdown();
		DXContext::Get().Shutdown();
	}
	DXDebugLayer::Get().Shutdown();
}

void initHeapPropsUpload(D3D12_HEAP_PROPERTIES* heapProps)
{
	heapProps->Type = D3D12_HEAP_TYPE_UPLOAD; // What type of heap this is is. We specify one that uploads to the GPU
	heapProps->MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; // Which memory pool (RAM or vRAM) is preferred 
	heapProps->CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; // Specifies how the GPU can access the memory
	heapProps->CreationNodeMask = 0; // Which GPU the heap shall be stored. We choose the first one.
	heapProps->VisibleNodeMask = 0; // Where the memory can be seen from
}

void initHeapPropsDefault(D3D12_HEAP_PROPERTIES* heapProps)
{
	heapProps->Type = D3D12_HEAP_TYPE_DEFAULT; // What type of heap this is is. We specify one is default
	heapProps->MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; // Which memory pool (RAM or vRAM) is preferred.
	heapProps->CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; // Specifies how the GPU can access the memory.
	heapProps->CreationNodeMask = 0; // Which GPU the heap shall be stored. We choose the first one.
	heapProps->VisibleNodeMask = 0; // Where the memory can be seen from.
}

void initRsrcDescUpload(D3D12_RESOURCE_DESC* rscDesc, uint32_t imgSize)
{
	rscDesc->Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // how many dimensions this resource has
	rscDesc->Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT; // 
	rscDesc->Width = 1024 + imgSize; // width of the buffer. 1024 is an arbitrary bit width
	rscDesc->Height = 1; // if this were a higher dimension resource, this would likely be larger than 1
	rscDesc->DepthOrArraySize = 1; // same as previous comment
	rscDesc->MipLevels = 1; // specify which level of mipmapping to use (1 means no mipmapping)
	rscDesc->Format = DXGI_FORMAT_UNKNOWN; // Specifies what type of byte format to use
	rscDesc->SampleDesc.Count = 1; // the amount of samples
	rscDesc->SampleDesc.Quality = 0; // the quality of the anti aliasing (0 means off)
	rscDesc->Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // define the texture layout. Other values will mess with the order of our buffer, so we choose row major (in order)
	rscDesc->Flags = D3D12_RESOURCE_FLAG_NONE; // specify various other options related to access and usage
}

void initRsrcDescVertex(D3D12_RESOURCE_DESC* rscDesc)
{
	rscDesc->Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // how many dimensions this resource has
	rscDesc->Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT; // 
	rscDesc->Width = 1024; // width of the buffer. 1024 is an arbitrary bit width
	rscDesc->Height = 1; // if this were a higher dimension resource, this would likely be larger than 1
	rscDesc->DepthOrArraySize = 1; // same as previous comment
	rscDesc->MipLevels = 1; // specify which level of mipmapping to use (1 means no mipmapping)
	rscDesc->Format = DXGI_FORMAT_UNKNOWN; // Specifies what type of byte format to use
	rscDesc->SampleDesc.Count = 1; // the amount of samples
	rscDesc->SampleDesc.Quality = 0; // the quality of the anti aliasing (0 means off)
	rscDesc->Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // define the texture layout. Other values will mess with the order of our buffer, so we choose row major (in order)
	rscDesc->Flags = D3D12_RESOURCE_FLAG_NONE; // specify various other options related to access and usage
}

void initPipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC* gfxpsd, ID3D12RootSignature* rootSig, D3D12_INPUT_ELEMENT_DESC* verLayout, unsigned long long verLayoutSize, Shader* verShader, Shader* pixShader)
{
	gfxpsd->pRootSignature = rootSig; // give pointer to a root signature (created by DXContext)

	gfxpsd->InputLayout.NumElements = verLayoutSize; // the number of elements in the vertex layout
	gfxpsd->InputLayout.pInputElementDescs = verLayout; // pointer to the vertex layout array

	gfxpsd->IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED; // 

	gfxpsd->VS.BytecodeLength = verShader->GetSize();     // give the bytecode lengths and the pointer to the bytecode for each pipeline shader
	gfxpsd->VS.pShaderBytecode = verShader->GetBuffer();
	gfxpsd->PS.BytecodeLength = pixShader->GetSize();
	gfxpsd->PS.pShaderBytecode = pixShader->GetBuffer();
	gfxpsd->DS.BytecodeLength = 0;
	gfxpsd->DS.pShaderBytecode = nullptr;
	gfxpsd->HS.BytecodeLength = 0;
	gfxpsd->HS.pShaderBytecode = nullptr;
	gfxpsd->GS.BytecodeLength = 0;
	gfxpsd->GS.pShaderBytecode = nullptr;

	gfxpsd->PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gfxpsd->RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // define the fill mode, which defines how the drawn objects are filled in for enclosed faces
	gfxpsd->RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // define what faces are culled, if at all
	gfxpsd->RasterizerState.FrontCounterClockwise = FALSE; // define which face is the front
	gfxpsd->RasterizerState.DepthBias = 0; // depth bias will attempt to push drawn objects forward
	gfxpsd->RasterizerState.DepthBiasClamp = .0f;
	gfxpsd->RasterizerState.SlopeScaledDepthBias = .0f;
	gfxpsd->RasterizerState.DepthClipEnable = FALSE; // set whether z-clipping can occur
	gfxpsd->RasterizerState.MultisampleEnable = FALSE; // set whether multisample will be on
	gfxpsd->RasterizerState.AntialiasedLineEnable = FALSE; // set whether anti-aliasing is on
	gfxpsd->RasterizerState.ForcedSampleCount = 0; // the sample count for anti-aliasing

	gfxpsd->StreamOutput.NumEntries = 0; // StreamOutput is a layer that streams transformed vertex data back to memory resources. We do not use it here. 
	gfxpsd->StreamOutput.NumStrides = 0;
	gfxpsd->StreamOutput.pBufferStrides = nullptr;
	gfxpsd->StreamOutput.pSODeclaration = nullptr;
	gfxpsd->StreamOutput.RasterizedStream = 0;

	gfxpsd->NumRenderTargets = 1;
	gfxpsd->RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	gfxpsd->DSVFormat = DXGI_FORMAT_UNKNOWN;

	gfxpsd->BlendState.AlphaToCoverageEnable = FALSE;
	gfxpsd->BlendState.IndependentBlendEnable = FALSE;
	gfxpsd->BlendState.RenderTarget[0].BlendEnable = TRUE;
	gfxpsd->BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	gfxpsd->BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	gfxpsd->BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	gfxpsd->BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
	gfxpsd->BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	gfxpsd->BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	gfxpsd->BlendState.RenderTarget[0].LogicOpEnable = FALSE;
	gfxpsd->BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	gfxpsd->BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	gfxpsd->DepthStencilState.DepthEnable = FALSE;
	gfxpsd->DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	gfxpsd->DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	gfxpsd->DepthStencilState.StencilEnable = FALSE;
	gfxpsd->DepthStencilState.StencilReadMask = 0;
	gfxpsd->DepthStencilState.StencilWriteMask = 0;
	gfxpsd->DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	gfxpsd->DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	gfxpsd->DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	gfxpsd->DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	gfxpsd->DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	gfxpsd->DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	gfxpsd->DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	gfxpsd->DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;

	gfxpsd->SampleMask = 0xFFFFFFFF;
	gfxpsd->SampleDesc.Count = 1;
	gfxpsd->SampleDesc.Quality = 0;

	gfxpsd->NodeMask = 0;

	gfxpsd->CachedPSO.CachedBlobSizeInBytes = 0;
	gfxpsd->CachedPSO.pCachedBlob = nullptr;

	gfxpsd->Flags = D3D12_PIPELINE_STATE_FLAG_NONE; // start with the none flag
}

void uncrnVomit(float* color, float delta)
{
	static int pukeState = 0;
	static float temp;

	color[pukeState] += delta;
	if (pukeState != 0)
	{
		color[pukeState - 1] -= delta;
		if (color[pukeState] > 1.0f) color[pukeState] = 1.0f;
		if (color[pukeState - 1] < 0.0f) color[pukeState - 1] = 0.0f;
		temp = float(sqrt(1.0f - pow(color[pukeState - 1], 2)));
		color[pukeState - 1] = float(sqrt(1.0f - pow(color[pukeState], 2)));
		color[pukeState] = temp;
	}
	else
	{
		color[2] -= delta;
		if (color[pukeState] > 1.0f) color[pukeState] = 1.0f;
		if (color[2] < 0.0f) color[2] = 0.0f;
		temp = float(sqrt(1.0f - pow(color[2], 2)));
		color[2] = float(sqrt(1.0f - pow(color[pukeState], 2)));
		color[pukeState] = temp;
	}

	if (color[pukeState] >= 1.0f)
	{
		color[pukeState] = 1.0f;
		pukeState++;
		if (pukeState == 3)
		{
			pukeState = 0;
		}
	}
}

void initResourceDescTexture(D3D12_RESOURCE_DESC* rscDesc, ImageLoader::ImageData* txtrData)
{
	rscDesc->Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // how many dimensions this resource has
	rscDesc->Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT; // 
	rscDesc->Width = txtrData->width; // width of the buffer. 1024 is an arbitrary bit width
	rscDesc->Height = txtrData->height; // if this were a higher dimension resource, this would likely be larger than 1
	rscDesc->DepthOrArraySize = 1; // same as previous comment
	rscDesc->MipLevels = 1; // specify which level of mipmapping to use (1 means no mipmapping)
	rscDesc->Format = txtrData->giPixelFormat; // Specifies what type of byte format to use
	rscDesc->SampleDesc.Count = 1; // the amount of samples
	rscDesc->SampleDesc.Quality = 0; // the quality of the anti aliasing (0 means off)
	rscDesc->Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // define the texture layout. Other values will mess with the order of our buffer, so we choose row major (in order)
	rscDesc->Flags = D3D12_RESOURCE_FLAG_NONE; // specify various other options related to access and usage
}