#include "DXContext.h"

bool DXContext::Init()
{
	// A factory for various common graphics infrastructure in DirectX
	if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&m_dxgiFactory)))) return false;

	// A virtual adapter that allows for creation of various D3D12 objects & resources
	if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)))) return false;

	// Create a command queue description
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	// create the command queue with the above description
	if (FAILED(m_device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&m_cmdQueue)))) return false;

	// Create the fence...
	if (FAILED(m_device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)))) return false;
	m_fenceEvent = CreateEvent(nullptr, false, false, nullptr); // ...and its event
	if (!m_fenceEvent) return false;

	// Create the command allocator, which is a heap where commands in command lists are stored
	if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmdAllocator)))) return false;

	// Create the command list, an interface instance that allows the user to write commands into the command allocator
	if (FAILED(m_device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_cmdList)))) return false;

	return true;
}

/* TODO:
-once command list count is more than one, release all command lists
-
*/
/* Clean shutdown of the DX components */
void DXContext::Shutdown()
{
	m_cmdList.Release();
	m_cmdAllocator.Release();
	if (m_fenceEvent)
	{
		CloseHandle(m_fenceEvent);
	}
	m_fence.Release();
	m_cmdQueue.Release();
	m_device.Release();
}

/* TODO:
-actually use the async computation that DX12 provides
--need to research DX12 async conventions and architecture
--setup a robust architecture for async based on research
--use the new async functionality
-refactor use of exit() function?
*/
// Sets a synchronization fence and waits for all workloads behind the fence to finish (kind of like a moving goal post)
void DXContext::SignalAndWait()
{
	m_cmdQueue->Signal(m_fence, ++m_fenceValue);
	if (SUCCEEDED(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent)))
	{
		if (WaitForSingleObject(m_fenceEvent, 20000) != WAIT_OBJECT_0)
		{
			exit(-1); // exit the program due to wait timeout
		}
	}
	else
	{
		exit(-1); // fence event could not be set, exit the program
	}
}

// Resets the command allocator and command lists
ID3D12GraphicsCommandList6* DXContext::InitCommandList()
{
	m_cmdAllocator->Reset();
	m_cmdList->Reset(m_cmdAllocator, nullptr);
	return m_cmdList;
}

/* TODO:
-add more command lists when tasks get computationally expensive and can be done in parallel
-
*/
// Closes the command list, puts it into list of command lists, puts the lists into a command queue, and finally call SignalAndWait()
void DXContext::ExecuteCommandList()
{
	if (SUCCEEDED(m_cmdList->Close()))
	{
		ID3D12CommandList* lists[] = { m_cmdList };
		m_cmdQueue->ExecuteCommandLists(1, lists);
		SignalAndWait();
	}
}
