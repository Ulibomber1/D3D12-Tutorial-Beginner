#pragma once

#include <WinSupport/WinInclude.h>
#include <WinSupport/ComPointer.h>

class DXDebugLayer
{
public:
	bool Init();
	void Shutdown();

private:
	#ifdef _DEBUG
	ComPointer<ID3D12Debug3> m_d3d12Debug;
	ComPointer<IDXGIDebug1> m_dxgiDebug;
	#endif

// Singleton
public:
	DXDebugLayer(const DXDebugLayer&) = delete;
	DXDebugLayer& operator=(const DXDebugLayer&) = delete;

	inline static DXDebugLayer& Get()
	{
		static DXDebugLayer instance;
		return instance;
	}

private:
	DXDebugLayer() = default;
};
