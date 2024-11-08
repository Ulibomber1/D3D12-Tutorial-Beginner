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
	
	void CreateGPUVertexBuffer(ComPointer<ID3D12Resource2>& pVertexBuffer, D3D12_VERTEX_BUFFER_VIEW* view, void* data, size_t dataSize, size_t dataStride);
	void CreateGPUIndexBuffer(ComPointer<ID3D12Resource2>& pIndexBuffer, D3D12_INDEX_BUFFER_VIEW* view, void* data, size_t dataStride);
	void CreateGPUTexture(ComPointer<ID3D12Resource2>& texture, ComPointer<ID3D12DescriptorHeap>& srvHeap, ImageLoader::ImageData* pTxtrData);
	void ExecuteUploadToGPU();

private:
	ID3D12GraphicsCommandList6* cmdList = nullptr;

	std::vector<ID3D12DescriptorHeap*> m_srvHeaps;
	std::vector<ImageLoader::ImageData*> m_txtrData;
	std::vector<ID3D12Resource2*> m_textures;

	std::vector<D3D12_VERTEX_BUFFER_VIEW*> m_vbViews;
	std::vector<void*> m_vbvData;
	std::vector<ID3D12Resource2*> m_vertexBuffers;

	std::vector<D3D12_INDEX_BUFFER_VIEW*> m_ibViews;
	std::vector<void*> m_ibvData;
	std::vector<ID3D12Resource2*> m_indexBuffers;

	ComPointer<ID3D12Resource2> uploadBuffer;
	size_t uploadBufferSize = 0;
	std::vector<D3D12_RESOURCE_BARRIER> barrierVec;
	
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
