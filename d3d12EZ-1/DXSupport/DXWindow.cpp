#include "DXWindow.h"

bool DXWindow::Init()
{
	// Define the window class
	WNDCLASSEXW wcex{};
	wcex.cbSize = sizeof(wcex); // 
	wcex.style = CS_OWNDC; // window style
	wcex.lpfnWndProc = &DXWindow::OnWindowMessage; // window message handler
	wcex.cbClsExtra = 0; // additional memory per class
	wcex.cbWndExtra = 0; // per window
	wcex.hInstance = GetModuleHandle(nullptr); // registers the window(?)
	wcex.hIcon = LoadIconW(nullptr, IDI_APPLICATION); // window icon in upper left and taskbar
	wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW); // the cursor graphic to be used
	wcex.hbrBackground = nullptr; // the background color
	wcex.lpszMenuName = nullptr; // name of the menu
	wcex.lpszClassName = L"D3D12EzWndCls"; // name of the window class
	wcex.hIconSm = LoadIconW(nullptr, IDI_APPLICATION); // small version of the window icon

	// Register the window class, then verify its registration
	m_wndClass = RegisterClassExW(&wcex);
	if (m_wndClass == 0)
	{
		return false;
	}

	// Setup for opening window on same monitor as cursor, default being primary monitor
	POINT pos{ 0,0 };
	GetCursorPos(&pos);
	HMONITOR monitor = MonitorFromPoint(pos, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monitorInfo{};
	monitorInfo.cbSize = sizeof(monitorInfo);
	GetMonitorInfoW(monitor, &monitorInfo);

	// Create the window
	m_window = CreateWindowExW(
		WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW, 
		(LPCWSTR)m_wndClass, 
		L"d3d12ez", 
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
		monitorInfo.rcWork.left + 100, 
		monitorInfo.rcWork.top + 100, 
		1920, 
		1080, 
		nullptr, 
		nullptr, 
		wcex.hInstance, 
		nullptr);
	if (m_window == nullptr) 
	{
		return false;
	}

	// Describe Swap Chain
	DXGI_SWAP_CHAIN_DESC1 swd{};
	swd.Width = 1920;
	swd.Height = 1080;
	swd.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // RGBA format and bit depth
	swd.Stereo = false; // enables stereo video output, which allows for 3D glasses or other HMDs to be used
	swd.SampleDesc.Count = 1; // How many pixels per actual pixel
	swd.SampleDesc.Quality = 0; // Antialiasing quality (0 means off)
	swd.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT; // Defines how the buffer is used; which buffer is drawn to, and whether it's drawn by CPU or GPU(?)
	swd.BufferCount = GetFrameCount(); // how many buffers are used in the swap chain (3 are typcially used by vsync in D3D12)
	swd.Scaling = DXGI_SCALING_STRETCH; // The type of scalinng to be used if the window gets resized 
	swd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Define what swap behavior is used
	swd.AlphaMode = DXGI_ALPHA_MODE_IGNORE; // Defines the type of alpha blending that is used
	swd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; // Extra option flags; here we allow mode switching and screen tearing
	m_format = swd.Format;
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC sfd{};
	sfd.Windowed = true; // Specify that the window is not fullscreen 

	// Swap Chain creation
	auto& factory = DXContext::Get().GetFactory();
	ComPointer<IDXGISwapChain1> sc1;
	factory->CreateSwapChainForHwnd(DXContext::Get().GetCommandQueue(), m_window, &swd, &sfd, nullptr, &sc1);
	if (!sc1.QueryInterface(m_swapChain))
	{
		return false;
	}

	// === RTV Descriptor Heap ===
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	{
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // this is a Render Target View heap
		descHeapDesc.NumDescriptors = FrameCount; // There are two descriptors in this heap
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // No flags, which set options for the heap
		descHeapDesc.NodeMask = 0; // no node mask, which is responsible for distinguishing adapter nodes. We only use 1 adapter in this program, so 0 is the appropriate value.
	}
	if (FAILED(DXContext::Get().GetDevice()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&m_rtvDescHeap))))
	{
		return false;
	}

	// === RTV Handles ===
	auto firstHandle = m_rtvDescHeap->GetCPUDescriptorHandleForHeapStart(); // the very first handle in the heap
	auto handleIncrement = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV); // the handle heap increment amount
	for (size_t i = 0; i < FrameCount; ++i)
	{
		m_rtvHandles[i] = firstHandle; // set the first handle...
		m_rtvHandles[i].ptr += handleIncrement * i; // ...then add the increment size of the descriptor handles times i to the pointer to set the pointer correctly.
	}

	// === DSV Descriptor Heap ===
	D3D12_DESCRIPTOR_HEAP_DESC depthDHD{};
	{
		depthDHD.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		depthDHD.NumDescriptors = 1;
	}
	if (FAILED(DXContext::Get().GetDevice()->CreateDescriptorHeap(&depthDHD, IID_PPV_ARGS(&m_dsvDescHeap))))
	{
		return false;
	}

	// === DSV Handle ===
	m_dsvHandle = m_dsvDescHeap->GetCPUDescriptorHandleForHeapStart();

	// Get Buffers
	if (!GetBuffers())
	{
		return false;
	}

	return true;
}

// polls the window so that it can respond to window messages, passes the message along to the window's message callback function
void DXWindow::Update()
{
	MSG msg;
	while (PeekMessageW(&msg, m_window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg); // Translate the message (weird MS data structure stuff makes this necessary)
		DispatchMessageW(&msg); // dispatches window message to the window's callback function
	}
}

// presents the current buffer, only whilst the buffer is in present state
void DXWindow::Present()
{
	m_swapChain->Present(1, 0);
}

// clean shutdown of the DX components of the window and the window itself
void DXWindow::Shutdown()
{
	ReleaseBuffers();

	m_rtvDescHeap.Release();
	m_dsvDescHeap.Release();
	m_swapChain.Release();

	if (m_window)
	{
		DestroyWindow(m_window);
	}

	if (m_wndClass)
	{
		UnregisterClassW((LPCWSTR)m_wndClass, GetModuleHandleW(nullptr));
	}
}

void DXWindow::Resize()
{
	ReleaseBuffers(); // Because 'resizing' buffers requires a new malloc, we need to release the old buffer...

	RECT cr;
	if (GetClientRect(m_window, &cr))
	{
		m_width = cr.right - cr.left;
		m_height = cr.bottom - cr.top;

		m_swapChain->ResizeBuffers(GetFrameCount(), m_width, m_height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
		
		m_shouldResize = false;
	}

	GetBuffers(); // ...and get the new one after the 'resizing' has been performed successfully
}

void DXWindow::SetFullscreen(bool enabled)
{
	//Update window Styling
	DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	DWORD exStyle = WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW;

	if (enabled)
	{
		style = WS_POPUP | WS_VISIBLE;
		exStyle = WS_EX_APPWINDOW;
	}

	SetWindowLongW(m_window, GWL_STYLE, style);
	SetWindowLongW(m_window, GWL_EXSTYLE, exStyle);

	// adjust window size
	if (enabled)
	{
		HMONITOR monitor = MonitorFromWindow(m_window, MONITOR_DEFAULTTONEAREST);
		MONITORINFO monitorInfo{};
		monitorInfo.cbSize = sizeof(monitorInfo);
		if (GetMonitorInfoW(monitor, &monitorInfo)) // set the fullscreen window to fill the nearest screen
		{
			SetWindowPos(m_window, nullptr,
				monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.top,
				monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
				SWP_NOZORDER
			);
		}
	}
	else
	{
		ShowWindow(m_window, SW_MAXIMIZE); // if diabling fullscreen, maximize the window size
	}

	m_isFullscreen = enabled;
}

// Sets the state of the resource barrier for the RTV to Render Target, clears it, then sets the CPU descriptor handle for the RTV
void DXWindow::BeginFrame(ID3D12GraphicsCommandList6* cmdlist)
{
	// Get the currently used buffer's index
	m_currentBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	/*
		Three rules of Resource Barriers:
		1. Constrain states as much as possible
		2. Avoid uneeded transitions
		3. Batch barriers (as opposed to interlacing them between various commands)
	*/

	// Describe the resource barrier, a structure designed to change memory state, flush caches, or ensure pipeline stalls. This all ensures correctness.
	D3D12_RESOURCE_BARRIER barr;
	barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; // Specify that it's a transition barrier
	barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE; // Specifies whether the barrier should be floating; if so, what type.
	barr.Transition.pResource = m_buffers[m_currentBufferIndex]; // The resource is the currently active buffer in the swapchain.
	barr.Transition.Subresource = 0; // No subresources
	barr.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT; // Specify the state before, Present (Common)...
	barr.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET; // ...and the state after, Render Target.

	cmdlist->ResourceBarrier(1, &barr);

	// Clear the RTV and DSV
	float clearColor[] = { .4f, .4f, .9f, 1.0f }; 
	cmdlist->ClearRenderTargetView(m_rtvHandles[m_currentBufferIndex], clearColor, 0, nullptr);
	float clearDepth = 1.0f;
	cmdlist->ClearDepthStencilView(m_dsvHandle, D3D12_CLEAR_FLAG_DEPTH, clearDepth, 0, 0, nullptr);
	// Set CPU descriptor handles for the RTVs and DSV
	cmdlist->OMSetRenderTargets(1, &m_rtvHandles[m_currentBufferIndex], false, &m_dsvHandle);
}

// Sets the state of the resource barrier for the RTV to Present (no changes allowed whilst in this state)
void DXWindow::EndFrame(ID3D12GraphicsCommandList6* cmdlist)
{
	D3D12_RESOURCE_BARRIER barr;
	barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barr.Transition.pResource = m_buffers[m_currentBufferIndex];
	barr.Transition.Subresource = 0;
	barr.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barr.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	cmdlist->ResourceBarrier(1, &barr);
}

// gets the current buffer and creates an RTV for it
bool DXWindow::GetBuffers()
{
	for (size_t i = 0; i < FrameCount; ++i)
	{
		if (FAILED(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_buffers[i]))))
		{
			return false;
		}

		// === RTV ===
		D3D12_RENDER_TARGET_VIEW_DESC rtv{};
		rtv.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtv.Texture2D.MipSlice = 0;
		rtv.Texture2D.PlaneSlice = 0;
		DXContext::Get().GetDevice()->CreateRenderTargetView(m_buffers[i], &rtv, m_rtvHandles[i]);
	}
	// === DSV ===
	D3D12_HEAP_PROPERTIES hpDepth{};
	{
		hpDepth.Type = D3D12_HEAP_TYPE_DEFAULT; // What type of heap this is is. We specify one is default
		hpDepth.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; // Which memory pool (RAM or vRAM) is preferred.
		hpDepth.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; // Specifies how the GPU can access the memory.
		hpDepth.CreationNodeMask = 0; // Which GPU the heap shall be stored. We choose the first one.
		hpDepth.VisibleNodeMask = 0; // Where the memory can be seen from.
	}
	D3D12_RESOURCE_DESC rdDepth{};
	{
		rdDepth.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // how many dimensions this resource has
		rdDepth.Alignment = 0; // 
		rdDepth.Width = m_width; // width of the buffer.
		rdDepth.Height = m_height; // height of the buffer
		rdDepth.DepthOrArraySize = 1; // same as previous comment
		rdDepth.MipLevels = 1; // specify which level of mipmapping to use (1 means no mipmapping)
		rdDepth.Format = DXGI_FORMAT_D32_FLOAT; // Specifies what type of byte format to use
		rdDepth.SampleDesc.Count = 1; // the amount of samples
		rdDepth.SampleDesc.Quality = 0; // the quality of the anti aliasing (0 means off)
		rdDepth.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // define the texture layout. Other values will mess with the order of our buffer, so we choose row major (in order)
		rdDepth.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // specify various other options related to access and usage
	}
	D3D12_CLEAR_VALUE clearValue{};
	{
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;
	}
	DXContext::Get().GetDevice()->CreateCommittedResource(&hpDepth, D3D12_HEAP_FLAG_NONE, &rdDepth, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&m_depthBuffer));
	DXContext::Get().GetDevice()->CreateDepthStencilView(m_depthBuffer, nullptr, m_dsvHandle);

	return true;
}

// called when the old buffers are getting discarded for ones with a new size
void DXWindow::ReleaseBuffers()
{
	for (size_t i = 0; i < FrameCount; ++i)
	{
		m_buffers[i].Release();
	}
	m_depthBuffer.Release();
}

/* TODO: 
-Flag for click handling
--we might need this for Dear ImGui
*/
// handles the processes for different window messages
LRESULT CALLBACK DXWindow::OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	if (ImGui_ImplWin32_WndProcHandler(wnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_F11)
		{
			Get().SetFullscreen(!Get().IsFullscreen());
		}
		if (wParam == VK_F1)
		{
			bool state = Get().Is3D();
			Get().m_is3D = !state;
		}
		break;
	case WM_SIZE:
		// if an lparam containing width and height are given in the message, and neither width or height are the same as before...
		if (lParam && (HIWORD(lParam) != Get().m_height || LOWORD(lParam) != Get().m_width))
		{
			Get().m_shouldResize = true; // ...Toggle flag for buffer/swapchain resizing
		}
		break;
	case WM_CLOSE:
		Get().m_shouldClose = true;
		return 0;
	}
	return DefWindowProcW(wnd, msg, wParam, lParam); // If the window message can't be handled here, use the default behavior
}
