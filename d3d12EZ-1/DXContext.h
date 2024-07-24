#pragma once

#include <WinInclude.h>
#include <ComPointer.h>

class DXContext
{
public:
	bool Init();
	void Shutdown();

	void SignalAndWait();
	ID3D12GraphicsCommandList6* InitCommandList();
	void ExecuteCommandList();

	inline void Flush(size_t count) // Flushes command queue a certain number of times specified by 'count'
	{
		for (size_t i = 0; i < count; i++)
		{
			SignalAndWait();
		}
	}
	inline ComPointer<IDXGIFactory7>& GetFactory()
	{
		return m_dxgiFactory;
	}
	inline ComPointer<ID3D12Device8>& GetDevice()
	{
		return m_device;
	}
	inline ComPointer<ID3D12CommandQueue>& GetCommandQueue()
	{
		return m_cmdQueue;
	}

private:
	ComPointer<IDXGIFactory7> m_dxgiFactory;

	ComPointer<ID3D12Device8> m_device; // ID3D12Device8 is the latest supported by Windows 10 Home Version 22H2, Build 19045.4651
	ComPointer<ID3D12CommandQueue> m_cmdQueue;

	ComPointer<ID3D12CommandAllocator> m_cmdAllocator;
	ComPointer<ID3D12GraphicsCommandList6> m_cmdList; // ID3D12GraphicsCommandAllocator is the latest supported by Windows 10 Home Version 22H2, Build 19045.4651

	ComPointer<ID3D12Fence1> m_fence;
	UINT64 m_fenceValue = 0;
	HANDLE m_fenceEvent = nullptr;

	// Singleton
public:
	DXContext(const DXContext&) = delete;
	DXContext& operator=(const DXContext&) = delete;

	inline static DXContext& Get()
	{
		static DXContext instance;
		return instance;
	}

private:
	DXContext() = default;
};
