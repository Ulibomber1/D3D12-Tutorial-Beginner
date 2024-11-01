#pragma once

#include <WinSupport/WinInclude.h>
#include <WinSupport/ComPointer.h>


/* TODO:
* - Figure out return types
* - Figure out attributes
* - Figure out parameters
*
*/
class DXDataHandler
{
public:
	bool Init();
	void Shutdown();

	void CreateUploadBuffer();
	void CreateGPUDefaultBuffer();
	void CreateGPUTextureBuffer();
	
	void SetRSDescRange();
	void SetRSParams();
	void SetRSFlags();
	void SetRSStaticSamplers();
	void CreateSetRootSig();
	void CreateRootSig();

	void GetSRVHeaps();
	void GetVBVs();
	void GetIBVs();

private:
	ID3D12GraphicsCommandList7* cmdList;
	//std::vector<D3D12_DESCRIPTOR_HEAP_DESC> m_
	std::vector<D3D12_VERTEX_BUFFER_VIEW> m_vbViews;
	std::vector<D3D12_INDEX_BUFFER_VIEW> m_ibViews;

	
// Singleton
public:
	DXDataHandler(const DXDataHandler&) = delete;
	DXDataHandler& operator=(const DXDataHandler&) = delete;

	inline static DXDataHandler& Get()
	{
		static DXDataHandler instance;
		return instance;
	}

private:
	DXDataHandler() = default;
};
