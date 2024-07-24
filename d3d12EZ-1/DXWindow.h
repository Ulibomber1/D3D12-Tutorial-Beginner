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
	void Resize();
	void SetFullscreen(bool enabled);

	inline bool ShouldClose() const
	{
		return m_shouldClose;
	}

	inline bool ShouldResize() const
	{
		return m_shouldResize;
	}
	inline bool IsFullscreen() const
	{
		return m_isFullscreen;
	}
	static constexpr UINT GetFrameCount()
	{
		return 2;
	}

	

private:
	static LRESULT CALLBACK OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
	ATOM m_wndClass = 0; // stores the window class instance
	HWND m_window = nullptr; // stores a reference to the window instance
	bool m_shouldClose = false; 

	bool m_shouldResize = false;
	UINT m_height = 1080;
	UINT m_width = 1920;

	bool m_isFullscreen = false;

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