#include <iostream>

#include <WinInclude.h>
#include <ComPointer.h>
#include <DXWindow.h>

#include <DXDebugLayer.h>

#include <DXContext.h>

int main()
{
	DXDebugLayer::Get().Init();
	if (DXContext::Get().Init() && DXWindow::Get().Init())
	{
		const char* hello = "Hello World!";

		D3D12_HEAP_PROPERTIES hpUpload{};
		hpUpload.Type = D3D12_HEAP_TYPE_UPLOAD; // What type of heap this is is. We specify one that uploads to the GPU
		hpUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; // Which memory pool (RAM or vRAM) is preferred 
		hpUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; // Specifies how the GPU can access the memory
		hpUpload.CreationNodeMask = 0; // Which GPU the heap shall ber stored. We choose the first one.
		hpUpload.VisibleNodeMask = 0; // Where the memory can be seen from

		D3D12_HEAP_PROPERTIES hpDefault{};
		hpDefault.Type = D3D12_HEAP_TYPE_DEFAULT; // What type of heap this is is. We specify one is default
		hpDefault.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; // Which memory pool (RAM or vRAM) is preferred.
		hpDefault.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; // Specifies how the GPU can access the memory.
		hpDefault.CreationNodeMask = 0; // Which GPU the heap shall ber stored. We choose the first one.
		hpDefault.VisibleNodeMask = 0; // Where the memory can be seen from.

		D3D12_RESOURCE_DESC rd{};
		rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // how many dimensions this resource has
		rd.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT; // 
		rd.Width = 1024; // width of the buffer. 1024 is an arbitrary bit width
		rd.Height = 1; // if this were a higher dimension resource, this would likely be larger than 1
		rd.DepthOrArraySize = 1; // same as previous comment
		rd.MipLevels = 1; // specify which level of mipmapping to use (1 means no mipmapping)
		rd.Format = DXGI_FORMAT_UNKNOWN; // Specifies what type of byte format to use
		rd.SampleDesc.Count = 1; // the amount of samples
		rd.SampleDesc.Quality = 0; // the quality of the anti aliasing (0 means off)
		rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // define the texture layout. Other values will mess with the order of our buffer, so we choose row major (in order)
		rd.Flags = D3D12_RESOURCE_FLAG_NONE; // specify various other options related to access and usage

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