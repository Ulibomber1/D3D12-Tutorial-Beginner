#pragma once

#include <WinInclude.h>
#include <ComPointer.h>
#include <DXContext.h>

class DXWindow
{
public:
	bool Init();
	void Update();
	void Present();
	void Shutdown();

	inline bool ShouldClose() const
	{
		return m_shouldClose;
	}

	static constexpr size_t GetFrameCount()
	{
		return 2;
	}

private:
	static LRESULT CALLBACK OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
	ATOM m_wndClass = 0; // stores the window class instance
	HWND m_window = nullptr; // stores a reference to the window instance
	bool m_shouldClose = false; 

	ComPointer<IDXGISwapChain3> m_swapChain;

// Singleton
public:
	DXWindow(const DXWindow&) = delete;
	DXWindow& operator=(const DXWindow&) = delete;

	inline static DXWindow& Get()
	{
		static DXWindow instance;
		return instance;
	}

private:
	DXWindow() = default;
};