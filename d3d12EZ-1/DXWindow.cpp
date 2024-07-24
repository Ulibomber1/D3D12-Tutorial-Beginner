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
    swd.Stereo = false; // enables stereo video output, which allows for 3D glasses to be used
    swd.SampleDesc.Count = 1; // How many pixels per actual pixel
    swd.SampleDesc.Quality = 0; // Antialiasing quality (0 means off)
    swd.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT; // Defines how the buffer is used; which buffer is drawn to, and whether it's drawn by CPU or GPU(?)
    swd.BufferCount = GetFrameCount(); // how many buffers are used in the swap chain (3 are typcially used by vsync in D3D12)
    swd.Scaling = DXGI_SCALING_STRETCH; // The type of scalinng to be used if the window gets resized 
    swd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Define what swap behavior is used
    swd.AlphaMode = DXGI_ALPHA_MODE_IGNORE; // Defines the type of alpha blending that is used
    swd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; // Extra option flags; here we allow mode switching and screen tearing

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

    // Get Buffers
    if (!GetBuffers())
    {
        return false;
    }

    return true;
}

void DXWindow::Update()
{
    MSG msg;
    while (PeekMessageW(&msg, m_window, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg); // Translate the message (weird MS data structure stuff makes this necessary)
        DispatchMessageW(&msg); 
    }
}

void DXWindow::Present()
{
    m_swapChain->Present(1, 0);
}

void DXWindow::Shutdown()
{
    ReleaseBuffers();

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

bool DXWindow::GetBuffers()
{
    for (UINT i = 0; i < FrameCount; ++i)
    {
        if (FALSE(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_buffers[i]))))
        {
            return false;
        }
    }
    return true;
}

void DXWindow::ReleaseBuffers()
{
    for (UINT i = 0; i < FrameCount; ++i)
    {
        m_buffers[i].Release();
    }
}

LRESULT CALLBACK DXWindow::OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
        if (wParam == VK_F11)
        {
            Get().SetFullscreen(!Get().IsFullscreen());
        }
        break;
    case WM_SIZE:
        // if an lparam containing width and height are given in the message, and neither width or height are the same as before:
        if (lParam && (HIWORD(lParam) != Get().m_height || LOWORD(lParam) != Get().m_width))
        {
            Get().m_shouldResize = true; // Toggle flag for buffer/swapchain resizing
        }
        break;
    case WM_CLOSE:
        Get().m_shouldClose = true;
        return 0;
    }
    return DefWindowProcW(wnd, msg, wParam, lParam); // If the window message can't be handled here, use the default behavior
}
