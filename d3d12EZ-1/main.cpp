#include <iostream>
#include <math.h> 

#include <WinSupport/WinInclude.h>
#include <WinSupport/ComPointer.h>
#include <DXSupport/ImageLoader.h>
#include <DXSupport/DXWindow.h>
#include <DXSupport/Shader.h>
#include <DXSupport/GPSODescBuilder.h>

#include <DXDebug/DXDebugLayer.h>

#include <DXSupport/DXContext.h>

using namespace DirectX;

void initHeapPropsUpload(D3D12_HEAP_PROPERTIES*);
void initHeapPropsDefault(D3D12_HEAP_PROPERTIES*);
void initRsrcDescUpload(D3D12_RESOURCE_DESC*, uint32_t);
void initRsrcDescVertex(D3D12_RESOURCE_DESC*);
void initPipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC* gfxpsd, ID3D12RootSignature* rootSig, D3D12_INPUT_ELEMENT_DESC* verLayout, unsigned long long verLayoutSize, Shader* verShader, Shader* pixShader);
void uncrnVomit(float* color, float delta);
void initResourceDescTexture(D3D12_RESOURCE_DESC* rscDesc, ImageLoader::ImageData* txtrData);

struct Vertex2D
{
	float x, y;
	float u, v;
};
struct VertexCube
{
	float x, y, z;
	float r, g, b;
};

// === 2D Vertex Data ===
Vertex2D vertices2D[] =
{
	{ -1.0f, -1.0f, 0.0f, 1.0f},
	{  0.0f,  1.0f, 0.5f, 0.0f},
	{  1.0f, -1.0f, 1.0f, 1.0f},
};
D3D12_INPUT_ELEMENT_DESC vertexLayout2D[] =
{
	{"Position", 0,  DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"Texcoord", 0,  DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 2, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
};

// === 3D Vertex Data (Cube) ===
VertexCube verticesCube[] =
{
	{ -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f}, // 0 
	{ -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f}, // 1 
	{  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  0.0f}, // 2 
	{  1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f}, // 3 
	{ -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f}, // 4 
	{ -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  1.0f}, // 5 
	{  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f}, // 6 
	{  1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  1.0f}  // 7 
};
WORD cubeIndices[] =
{
	0, 1, 2, 0, 2, 3,
	4, 6, 5, 4, 7, 6,
	4, 5, 1, 4, 1, 0,
	3, 2, 6, 3, 6, 7,
	1, 5, 6, 1, 6, 2,
	4, 0, 3, 4, 3, 7
};
D3D12_INPUT_ELEMENT_DESC vertexLayout3D[] =
{
	{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"Color", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 3, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
};

// D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE|D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE

int main()
{
	DXDebugLayer::Get().Init();
	if (DXContext::Get().Init() && DXWindow::Get().Init() && DirectX::XMVerifyCPUSupport())
	{
		auto* cmdList = DXContext::Get().InitCommandList();

		// === Heap Descriptors ===
		D3D12_HEAP_PROPERTIES hpUpload{};
		initHeapPropsUpload(&hpUpload);
		D3D12_HEAP_PROPERTIES hpDefault{};
		initHeapPropsDefault(&hpDefault);

		// === Texture Data ===
		ImageLoader::ImageData textureData;
		ImageLoader::LoadImageFromDisk("./auge_512_512_BGRA_32BPP.png", textureData);
		uint32_t textureStride = textureData.width * ((textureData.bpp + 7) / 8);
		uint32_t textureSize = textureData.height * textureStride;

		// === Buffer Descriptors ===
		D3D12_RESOURCE_DESC rdu{};
		initRsrcDescUpload(&rdu, textureSize);
		D3D12_RESOURCE_DESC rdv{};
		initRsrcDescVertex(&rdv);
		D3D12_RESOURCE_DESC rdi{};
		initRsrcDescVertex(&rdi);

		// === Heap & Buffer Creation ===
		UINT numIndices = (UINT)std::size(cubeIndices);
		ComPointer<ID3D12Resource2> uploadBuffer, vertexBuffer, indexBuffer;
		DXContext::Get().GetDevice()->CreateCommittedResource(&hpUpload, D3D12_HEAP_FLAG_NONE, &rdu, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));
		DXContext::Get().GetDevice()->CreateCommittedResource(&hpDefault, D3D12_HEAP_FLAG_NONE, &rdv, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&vertexBuffer));
		DXContext::Get().GetDevice()->CreateCommittedResource(&hpDefault, D3D12_HEAP_FLAG_NONE, &rdi, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&indexBuffer));

		// === Texture Description ===
		D3D12_RESOURCE_DESC rdt{};
		initResourceDescTexture(&rdt, &textureData);
		ComPointer<ID3D12Resource2> texture;
		DXContext::Get().GetDevice()->CreateCommittedResource(&hpDefault, D3D12_HEAP_FLAG_NONE, &rdt, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&texture));

		// === SRV Descriptor Heap (for now, just Textures) ===
		D3D12_DESCRIPTOR_HEAP_DESC dhd{};
		{
			dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			dhd.NumDescriptors = 1;
			dhd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			dhd.NodeMask = 0;
		}
		ComPointer<ID3D12DescriptorHeap> srvHeap;
		DXContext::Get().GetDevice()->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&srvHeap));

		// === SRV ===
		D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
		{
			srv.Format = textureData.giPixelFormat;
			srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srv.Texture2D.MipLevels = 1;
			srv.Texture2D.MostDetailedMip = 0;
			srv.Texture2D.PlaneSlice = 0;
			srv.Texture2D.ResourceMinLODClamp = 0.0f;
		}
		DXContext::Get().GetDevice()->CreateShaderResourceView(texture, &srv, srvHeap->GetCPUDescriptorHandleForHeapStart());

		// === Resource Uploads ===
		// copy void* (Memory) --> CPU resource (Upload Buffer)
		{
			char* uploadBufferAddress = nullptr;
			D3D12_RANGE uploadRange;
			uploadRange.Begin = 0;
			uploadRange.End = rdu.Width;
			uploadBuffer->Map(0, &uploadRange, (void**)&uploadBufferAddress);
			memcpy(&uploadBufferAddress[0], textureData.data.data(), textureSize);
			memcpy(&uploadBufferAddress[textureSize], vertices2D, sizeof(vertices2D));
			memcpy(&uploadBufferAddress[textureSize + sizeof(vertices2D)], cubeIndices, sizeof(cubeIndices));
			uploadBuffer->Unmap(0, &uploadRange);
		}

		// copy vertex data (Upload Buffer) --> GPU resource (Vertex Buffer)
		cmdList->CopyBufferRegion(vertexBuffer, 0, uploadBuffer, textureSize, 1024);

		// copy texture (Upload Buffer) --> GPU resource (Texture)
		D3D12_BOX textureSizeAsBox;
		D3D12_TEXTURE_COPY_LOCATION txtcSrc, txtcDst;
		{
			textureSizeAsBox.left = textureSizeAsBox.top = textureSizeAsBox.front = 0;
			textureSizeAsBox.right = textureData.width;
			textureSizeAsBox.bottom = textureData.height;
			textureSizeAsBox.back = 1;
		
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
		}
		cmdList->CopyTextureRegion(&txtcDst, 0, 0, 0, &txtcSrc, &textureSizeAsBox);

		// copy index data (upload buffer) --> GPU Resource (Index Buffer)
		cmdList->CopyBufferRegion(indexBuffer, 0, uploadBuffer, textureSize + 1024, 1024);

		// === Resource Barriers ===
		D3D12_RESOURCE_BARRIER vertBuffBarr; // Vertex Buffer Resource Barrier
		{
			vertBuffBarr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; // Specify the type of resource barrier
			vertBuffBarr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE; // Specifies whether the barrier should be floating; if so, what type.
			vertBuffBarr.Transition.pResource = vertexBuffer; // The resource...
			vertBuffBarr.Transition.Subresource = 0; // ...and its subresources
			vertBuffBarr.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST; // Specify the state before...
			vertBuffBarr.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER; // ...and the state after
		}
		D3D12_RESOURCE_BARRIER textureBarr; // Texture Resource Barrier
		{
			textureBarr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; // Specify the type of resource barrier
			textureBarr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE; // Specifies whether the barrier should be floating; if so, what type.
			textureBarr.Transition.pResource = texture; // The resource...
			textureBarr.Transition.Subresource = 0; // ...and its subresources
			textureBarr.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST; // Specify the state before...
			textureBarr.Transition.StateAfter = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE; // ...and the state after
		}
		D3D12_RESOURCE_BARRIER indexBuffBarr; // Texture Resource Barrier
		{
			indexBuffBarr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; // Specify the type of resource barrier
			indexBuffBarr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE; // Specifies whether the barrier should be floating; if so, what type.
			indexBuffBarr.Transition.pResource = indexBuffer; // The resource...
			indexBuffBarr.Transition.Subresource = 0; // ...and its subresources
			indexBuffBarr.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST; // Specify the state before...
			indexBuffBarr.Transition.StateAfter = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE; // ...and the state after
		}
		D3D12_RESOURCE_BARRIER barriers[] = { vertBuffBarr, textureBarr, indexBuffBarr };
		cmdList->ResourceBarrier(_countof(barriers), barriers);

		// === Execute Resource Uploads ===
		DXContext::Get().ExecuteCommandList();

		// === Shaders ===
		Shader vertexShader("VertexShader.cso");
		Shader pixelShader("PixelShader.cso");

		// === Create Root Sig ===
		ComPointer<ID3D12RootSignature> rootSignature;
		CD3DX12_DESCRIPTOR_RANGE descRange[1] = {};
		descRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		CD3DX12_ROOT_PARAMETER rootParams[4] = {};
		rootParams[0].InitAsConstants(4, 0);
		rootParams[1].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		rootParams[2].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 2, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		rootParams[3].InitAsDescriptorTable(1, &descRange[0]);
		D3D12_ROOT_SIGNATURE_FLAGS rootSigFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
		CD3DX12_STATIC_SAMPLER_DESC staticSampler[1] = {};
		staticSampler[0].Init(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc = {};
		rootSigDesc.Init((UINT)std::size(rootParams), rootParams, 1, staticSampler, rootSigFlags);
		ComPointer<ID3DBlob> rootSigBlob;
		ComPointer<ID3DBlob> errorBlob;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSigBlob, &errorBlob);
		if (FAILED(hr))
		{
			return -2;
		}
		DXContext::Get().GetDevice()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

		// === Pipeline State Object Description ===
		GPSODescBuilder2D PSObuilder(rootSignature, vertexLayout2D, (UINT)_countof(vertexLayout2D), &vertexShader, &pixelShader, (Shader*)nullptr, (Shader*)nullptr, (Shader*)nullptr); // we pass vertexLayout array since it will decay to a pointer anyways
		GPSODescDirector PSOdirector;
		PSOdirector.Construct(PSObuilder);
		D3D12_GRAPHICS_PIPELINE_STATE_DESC gfxPsod = PSObuilder.GetDescriptor();
		ComPointer<ID3D12PipelineState> pso;
		DXContext::Get().GetDevice()->CreateGraphicsPipelineState(&gfxPsod, IID_PPV_ARGS(&pso));

		// === Vertex Buffer View ===
		D3D12_VERTEX_BUFFER_VIEW vbv{};
		{
			vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
			vbv.SizeInBytes = sizeof(vertices2D) * _countof(vertices2D);
			vbv.StrideInBytes = sizeof(Vertex2D);
		}

		// === Index Buffer View ===
		D3D12_INDEX_BUFFER_VIEW ibv{};
		{
			ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress();
			ibv.SizeInBytes = numIndices * sizeof(WORD);
			ibv.Format = DXGI_FORMAT_R16_UINT;
		}
		XMMATRIX viewProjection;
		{
			// setup view mat
			auto eyePos = XMVectorSet(0, 0, -3, 1);
			auto focusPos = XMVectorSet(0, 0, 0, 1);
			auto upDir = XMVectorSet(0, 1, 0, 0);
			auto viewMtx = XMMatrixLookAtLH(eyePos, focusPos, upDir);
			// setup perspective
			auto aspectRatio = float(DXWindow::Get().GetWidth()) / float(DXWindow::Get().GetHeight());
			auto projectionMtx = XMMatrixPerspectiveFovLH(XMConvertToRadians(65.f), aspectRatio, 0.1f, 100.0f);
			// combine view and perspective matrices
			viewProjection = viewMtx * projectionMtx;
		}

		// === RENDER LOOP ===
		float time = 0.f;
		const float timeStep = 0.005f;
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
			
			// == Pipeline State Object Updates ==
			cmdList->SetPipelineState(pso);
			cmdList->SetGraphicsRootSignature(rootSignature);
			cmdList->SetDescriptorHeaps(1, &srvHeap);

			// == Input Assembler Updates ==
			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			cmdList->IASetVertexBuffers(0, 1, &vbv);
			cmdList->IASetIndexBuffer(&ibv);

			// == Rasterizer Updates ==
			D3D12_VIEWPORT vp;
			{
				vp.TopLeftX = vp.TopLeftY = 0;
				vp.Width = DXWindow::Get().GetWidth();
				vp.Height = DXWindow::Get().GetHeight();
				vp.MinDepth = 1.f;
				vp.MaxDepth = 0.f;
			}
			cmdList->RSSetViewports(1, &vp);
			RECT scRect;
			{
				scRect.left = scRect.top = 0;
				scRect.right = DXWindow::Get().GetWidth();
				scRect.bottom = DXWindow::Get().GetHeight();
			}
			cmdList->RSSetScissorRects(1, &scRect);

			// == Misc. Updates ==
			static float color[] = { 0.0f, 0.0f, 1.0f };
			uncrnVomit(color, 0.0025f);
			XMMATRIX illuminatiTM = XMMatrixTranspose(
				XMMatrixRotationX(2.0f * time + 0.f) *
				XMMatrixRotationY(4.0f * time + 0.f) *
				XMMatrixRotationZ(1.0f * time + 0.f) * 
				viewProjection
			);
			XMMATRIX cubeTM = XMMatrixTranspose(
				XMMatrixRotationX(2.0f * time + 0.f) *
				XMMatrixRotationY(4.0f * time + 0.f) *
				XMMatrixRotationZ(1.0f * time + 0.f) *
				viewProjection
			);

			// == Root Sig Updates ==
			cmdList->SetGraphicsRoot32BitConstants(0, 3, color, 0);
			cmdList->SetGraphicsRoot32BitConstants(1, sizeof(illuminatiTM) / 4, &illuminatiTM, 0);
			cmdList->SetGraphicsRoot32BitConstants(2, sizeof(cubeTM) / 4, &cubeTM, 0);
			cmdList->SetGraphicsRootDescriptorTable(3, srvHeap->GetGPUDescriptorHandleForHeapStart());
			
			// Draw
			cmdList->DrawInstanced(_countof(vertices2D), 1, 0, 0);
			// cmdList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);

			DXWindow::Get().EndFrame(cmdList);

			// Finish and Show the render
			DXContext::Get().ExecuteCommandList();
			DXWindow::Get().Present();

			// == Timed Updates ==
			time += timeStep;
		}

		// Flush command queue
		DXContext::Get().Flush(DXWindow::Get().GetFrameCount());

		// Release Buffers
		vertexBuffer.Release();
		uploadBuffer.Release();
		indexBuffer.Release();

		DXWindow::Get().Shutdown();
		DXContext::Get().Shutdown();
	}
	DXDebugLayer::Get().Shutdown();
}

// Make these into static inline funcs in header??
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
	rscDesc->Width = 2048 + imgSize; // width of the buffer. 1024 is an arbitrary bit width
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
