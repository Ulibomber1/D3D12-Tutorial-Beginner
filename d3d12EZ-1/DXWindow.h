#pragma once

#include <WinInclude.h>

class DXWindow
{
public:
	bool Init();
	void Update();
	void Shutdown();

	inline bool ShouldClose() const
	{
		return m_shouldClose;
	}

private:
	static LRESULT CALLBACK OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
	ATOM m_wndClass = 0; // stores the window class instance
	HWND m_window = nullptr; // stores a reference to the window instance
	bool m_shouldClose = false; 

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