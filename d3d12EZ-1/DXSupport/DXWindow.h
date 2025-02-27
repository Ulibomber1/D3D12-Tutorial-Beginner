#pragma once

#include <WinSupport/WinInclude.h>
#include <WinSupport/ComPointer.h>
#include <DXSupport/DXContext.h>

#include <imgui/imgui_impl_win32.h>

class DXWindow
{
public:
	bool Init();
	void Update();
	void Present();
	void Shutdown();
	void Resize();
	void SetFullscreen(bool enabled);

	void BeginFrame(ID3D12GraphicsCommandList6* cmdlist);
	void EndFrame(ID3D12GraphicsCommandList6* cmdlist);

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
	inline bool Is3D() const
	{
		return m_is3D;
	}

	inline UINT GetWidth() const
	{
		return m_width;
	}
	inline UINT GetHeight() const
	{
		return m_height;
	}

	inline HWND GetWindow() const 
	{
		return m_window;
	}

	static constexpr size_t FrameCount = 2;
	static constexpr size_t GetFrameCount()
	{
		return FrameCount;
	}

	inline DXGI_FORMAT GetBufferFormat() const
	{
		return m_format;
	}
private:
	bool GetBuffers();
	void ReleaseBuffers();

	static LRESULT CALLBACK OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
	ATOM m_wndClass = 0; // stores the window class instance
	HWND m_window = nullptr; // stores a reference to the window instance
	bool m_shouldClose = false; 

	bool m_shouldResize = false;
	UINT m_height = 1080;
	UINT m_width = 1920;

	bool m_isFullscreen = false;
	bool m_is3D = true;

	ComPointer<IDXGISwapChain3> m_swapChain;
	ComPointer<ID3D12Resource2> m_buffers[FrameCount];
	size_t m_currentBufferIndex = 0;
	DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;

	ComPointer<ID3D12DescriptorHeap> m_rtvDescHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandles[FrameCount] = {};

	ComPointer<ID3D12Resource2> m_depthBuffer;

	ComPointer<ID3D12DescriptorHeap> m_dsvDescHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_dsvHandle = {};


// Singleton (because *typically* you don't want multiple windows)
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