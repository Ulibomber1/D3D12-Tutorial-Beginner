#include <iostream>

#include <WinSupport/WinInclude.h>
#include <WinSupport/ComPointer.h>
#include <DXSupport/DXWindow.h>
#include <DXSupport/Shader.h>
#include <DXDebug/DXDebugLayer.h>

#include <DXSupport/DXContext.h>

void initHeapPropsUpload(D3D12_HEAP_PROPERTIES*);
void initHeapPropsDefault(D3D12_HEAP_PROPERTIES*);
void initResourceDesc(D3D12_RESOURCE_DESC*);
void initPipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC* gfxpsd, ComPointer<ID3D12RootSignature> rootSig, D3D12_INPUT_ELEMENT_DESC* verLayout, unsigned long long verLayoutSize, Shader* verShader, Shader* pixShader);

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

		// == Upload & Vertex Buffer Description ==
		D3D12_RESOURCE_DESC rd{};
		initResourceDesc(&rd);

		ComPointer<ID3D12Resource2> uploadBuffer, vertexBuffer;
		DXContext::Get().GetDevice()->CreateCommittedResource(&hpUpload, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));
		DXContext::Get().GetDevice()->CreateCommittedResource(&hpDefault, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&vertexBuffer));

		// copy void* --> CPU resource
		void* uploadBufferAddress;
		D3D12_RANGE uploadRange;
		uploadRange.Begin = 0;
		uploadRange.End = 1023;
		uploadBuffer->Map(0, &uploadRange, &uploadBufferAddress);
		memcpy(uploadBufferAddress, vertices, sizeof(vertices));
		uploadBuffer->Unmap(0, &uploadRange);

		// copy CPU resource --> GPU resource
		auto* cmdList = DXContext::Get().InitCommandList();
		cmdList->CopyBufferRegion(vertexBuffer, 0, uploadBuffer, 0, 1024);
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

		//DXContext::Get().GetDevice()->createpipeline

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
			
			// == Input Assembler ==
			cmdList->IASetVertexBuffers(0, 1, &vbv);
			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			
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

void initResourceDesc(D3D12_RESOURCE_DESC* rscDesc)
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

void initPipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC* gfxpsd, ComPointer<ID3D12RootSignature> rootSig, D3D12_INPUT_ELEMENT_DESC* verLayout, unsigned long long verLayoutSize, Shader* verShader, Shader* pixShader)
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
	// TODO: BlendState, DepthStencilState, SampleMask, NumRenderTargets, RTVFormats, DSVFormat, SampleDesc
	gfxpsd->NodeMask = 0;
	gfxpsd->CachedPSO.CachedBlobSizeInBytes = 0;
	gfxpsd->CachedPSO.pCachedBlob = nullptr;
	gfxpsd->Flags = D3D12_PIPELINE_STATE_FLAG_NONE; // start with the none flag
	// TODO: Output Merger
}
