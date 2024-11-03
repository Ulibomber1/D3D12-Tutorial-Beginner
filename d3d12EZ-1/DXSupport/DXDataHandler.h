#pragma once

#include <WinSupport/WinInclude.h>
#include <WinSupport/ComPointer.h>
#include <DXSupport/ImageLoader.h>
#include <DXSupport/DXContext.h>


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
	
	void CreateGPUVertexBuffer(ID3D12Resource2* pVertexBuffer, void* data, size_t dataStride);
	void CreateGPUIndexBuffer(ID3D12Resource2* pIndexBuffer, void* data, size_t dataStride);
	void CreateGPUTexture(ImageLoader::ImageData* pTxtrData);
	void ExecuteUploadToGPU();
	
	/*void SetRSDescRange();
	void SetRSParams();
	void SetRSFlags();
	void SetRSStaticSamplers();
	void CreateSetRootSig();
	void CreateRootSig();*/

	inline std::vector<ComPointer<ID3D12DescriptorHeap>> GetSRVHeaps()
	{
		return m_srvHeaps;
	}
	inline std::vector<D3D12_VERTEX_BUFFER_VIEW> GetVBVs() 
	{
		return m_vbViews;
	}
	inline std::vector<D3D12_INDEX_BUFFER_VIEW> GetIBVs()
	{
		return m_ibViews;
	}

private:
	ID3D12GraphicsCommandList6* cmdList = nullptr;

	std::vector<ComPointer<ID3D12DescriptorHeap>> m_srvHeaps;
	std::vector<D3D12_VERTEX_BUFFER_VIEW> m_vbViews;
	std::vector<D3D12_INDEX_BUFFER_VIEW> m_ibViews;
	std::vector<void*> m_srvData;
	std::vector<void*> m_vbvData;
	std::vector<void*> m_ibvData;

	ComPointer<ID3D12Resource2> uploadBuffer;
	size_t uploadBufferSize = 0;

	void CreateUploadBuffer(ID3D12Resource2* pUploadBuffer, void* data);
	void CreateGPUDefaultBuffer(ID3D12Resource2* pDefaultBuffer, void* data);
	
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
