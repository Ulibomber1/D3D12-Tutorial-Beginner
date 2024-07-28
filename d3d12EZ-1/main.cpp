#include <iostream>

#include <WinInclude.h>
#include <ComPointer.h>
#include <DXWindow.h>

#include <DXDebugLayer.h>

#include <DXContext.h>

void initHeapPropsUpload(D3D12_HEAP_PROPERTIES*);
void initHeapPropsDefault(D3D12_HEAP_PROPERTIES*);
void initResourceDesc(D3D12_RESOURCE_DESC*);

int main()
{
	DXDebugLayer::Get().Init();
	if (DXContext::Get().Init() && DXWindow::Get().Init())
	{
		const char* hello = "Hello World!";

		D3D12_HEAP_PROPERTIES hpUpload{};
		initHeapPropsUpload(&hpUpload);

		D3D12_HEAP_PROPERTIES hpDefault{};
		initHeapPropsDefault(&hpDefault);

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
		memcpy(uploadBufferAddress, hello, strlen(hello) + 1);
		uploadBuffer->Unmap(0, &uploadRange);

		// copy CPU resource --> GPU resource
		auto* cmdList = DXContext::Get().InitCommandList();
		cmdList->CopyBufferRegion(vertexBuffer, 0, uploadBuffer, 0, 1024);
		DXContext::Get().ExecuteCommandList();


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
			// Draw
			DXWindow::Get().EndFrame(cmdList);

			// Finish and Show the render
			DXContext::Get().ExecuteCommandList();
			DXWindow::Get().Present();
		}

		// Flush command queue
		DXContext::Get().Flush(DXWindow::Get().GetFrameCount());

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
