#pragma once

#include <WinSupport/WinInclude.h>
#include <WinSupport/ComPointer.h>
#include <DXSupport/ImageLoader.h>
#include <DXSupport/DXContext.h>


/* TODO:
* - change void pointers to templates or union types
*/
class DXDataHandler
{
public:
	bool Init();
	void Shutdown();
	
	void CreateGPUVertexBuffer(ID3D12Resource2* pVertexBuffer, void* data, size_t dataSize, size_t dataStride);
	void CreateGPUIndexBuffer(ID3D12Resource2* pIndexBuffer, void* data, size_t dataStride);
	void CreateGPUTexture(ID3D12Resource2* texture, ImageLoader::ImageData* pTxtrData);
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
	std::vector<ImageLoader::ImageData*> m_txtrData;
	std::vector<ID3D12Resource2*> m_textures;

	std::vector<D3D12_VERTEX_BUFFER_VIEW> m_vbViews;
	std::vector<void*> m_vbvData;
	std::vector<ID3D12Resource2*> m_vertexBuffers;

	std::vector<D3D12_INDEX_BUFFER_VIEW> m_ibViews;
	std::vector<void*> m_ibvData;
	std::vector<ID3D12Resource2*> m_indexBuffers;

	ComPointer<ID3D12Resource2> uploadBuffer;
	size_t uploadBufferSize = 0;
	
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
