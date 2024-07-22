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

void DXWindow::Shutdown()
{
    if (m_window)
    {
        DestroyWindow(m_window);
    }

    if (m_wndClass)
    {
        UnregisterClassW((LPCWSTR)m_wndClass, GetModuleHandleW(nullptr));
    }
}

LRESULT CALLBACK DXWindow::OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        Get().m_shouldClose = true;
        return 0;
    }
    return DefWindowProcW(wnd, msg, wParam, lParam); // If the window message can't be handled here, use the default behavior
}
