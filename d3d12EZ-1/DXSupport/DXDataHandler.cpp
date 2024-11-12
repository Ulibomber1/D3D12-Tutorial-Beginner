#include "DXDataHandler.h"

bool DXDataHandler::Init()
{
	cmdList = DXContext::Get().InitCommandList();
	if (cmdList == nullptr)
		return false;
	return true;
}

void DXDataHandler::Shutdown()
{
	uploadBuffer.Release();
}

void DXDataHandler::CreateGPUVertexBuffer(ComPointer<ID3D12Resource2>& pVertexBuffer, D3D12_VERTEX_BUFFER_VIEW* view, void* data, size_t dataSize, size_t dataStride)
{
	D3D12_HEAP_PROPERTIES hpDefault{};
	{
		hpDefault.Type = D3D12_HEAP_TYPE_DEFAULT; // What type of heap this is is. We specify one is default
		hpDefault.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; // Which memory pool (RAM or vRAM) is preferred.
		hpDefault.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; // Specifies how the GPU can access the memory.
		hpDefault.CreationNodeMask = 0; // Which GPU the heap shall be stored. We choose the first one.
		hpDefault.VisibleNodeMask = 0; // Where the memory can be seen from.
	}
	D3D12_RESOURCE_DESC rdv{};
	{
		rdv.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // how many dimensions this resource has
		rdv.Alignment = 0;//D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT; // 
		rdv.Width = dataSize; // width of the buffer.
		rdv.Height = 1; // if this were a higher dimension resource, this would likely be larger than 1
		rdv.DepthOrArraySize = 1; // same as previous comment
		rdv.MipLevels = 1; // specify which level of mipmapping to use (1 means no mipmapping)
		rdv.Format = DXGI_FORMAT_UNKNOWN; // Specifies what type of byte format to use
		rdv.SampleDesc.Count = 1; // the amount of samples
		rdv.SampleDesc.Quality = 0; // the quality of the anti aliasing (0 means off)
		rdv.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // define the texture layout. Other values will mess with the order of our buffer, so we choose row major (in order)
		rdv.Flags = D3D12_RESOURCE_FLAG_NONE; // specify various other options related to access and usage
	}

	DXContext::Get().GetDevice()->CreateCommittedResource(&hpDefault, D3D12_HEAP_FLAG_NONE, &rdv, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pVertexBuffer));
	
	view->BufferLocation = 0;
	view->SizeInBytes = dataSize;
	view->StrideInBytes = dataStride;

	m_vbViews.push_back(view);
	m_vbvData.push_back(data);
	m_vertexBuffers.push_back(pVertexBuffer);
	uploadBufferSize += dataSize;
}

void DXDataHandler::CreateGPUIndexBuffer(ComPointer<ID3D12Resource2>& pIndexBuffer, D3D12_INDEX_BUFFER_VIEW* view, void* data, size_t dataSize)
{
	D3D12_HEAP_PROPERTIES hpDefault{};
	{
		hpDefault.Type = D3D12_HEAP_TYPE_DEFAULT; // What type of heap this is is. We specify one is default
		hpDefault.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; // Which memory pool (RAM or vRAM) is preferred.
		hpDefault.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; // Specifies how the GPU can access the memory.
		hpDefault.CreationNodeMask = 0; // Which GPU the heap shall be stored. We choose the first one.
		hpDefault.VisibleNodeMask = 0; // Where the memory can be seen from.
	}
	D3D12_RESOURCE_DESC rdi{};
	{
		rdi.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // how many dimensions this resource has
		rdi.Alignment = 0;//D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT; // 
		rdi.Width = dataSize; // width of the buffer.
		rdi.Height = 1; // if this were a higher dimension resource, this would likely be larger than 1
		rdi.DepthOrArraySize = 1; // same as previous comment
		rdi.MipLevels = 1; // specify which level of mipmapping to use (1 means no mipmapping)
		rdi.Format = DXGI_FORMAT_UNKNOWN; // Specifies what type of byte format to use
		rdi.SampleDesc.Count = 1; // the amount of samples
		rdi.SampleDesc.Quality = 0; // the quality of the anti aliasing (0 means off)
		rdi.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // define the texture layout. Other values will mess with the order of our buffer, so we choose row major (in order)
		rdi.Flags = D3D12_RESOURCE_FLAG_NONE; // specify various other options related to access and usage
	}

	DXContext::Get().GetDevice()->CreateCommittedResource(&hpDefault, D3D12_HEAP_FLAG_NONE, &rdi, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pIndexBuffer));
	
	view->BufferLocation = 0;
	view->SizeInBytes = dataSize;
	view->Format = DXGI_FORMAT_R16_UINT; // Find out how to generalize format

	m_ibViews.push_back(view);
	m_ibvData.push_back(data);
	m_indexBuffers.push_back(pIndexBuffer);
	uploadBufferSize += dataSize;
}

void DXDataHandler::CreateGPUTexture(ComPointer<ID3D12Resource2>& pTexture, ComPointer<ID3D12DescriptorHeap>& srvHeap, ImageLoader::ImageData* pTxtrData)
{
	D3D12_HEAP_PROPERTIES hpDefault{};
	{
		hpDefault.Type = D3D12_HEAP_TYPE_DEFAULT; // What type of heap this is is. We specify one is default
		hpDefault.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; // Which memory pool (RAM or vRAM) is preferred.
		hpDefault.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; // Specifies how the GPU can access the memory.
		hpDefault.CreationNodeMask = 0; // Which GPU the heap shall be stored. We choose the first one.
		hpDefault.VisibleNodeMask = 0; // Where the memory can be seen from.
	}
	D3D12_RESOURCE_DESC rdt{};
	{
		rdt.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // how many dimensions this resource has
		rdt.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT; // 
		rdt.Width = pTxtrData->width; // width of the buffer. 1024 is an arbitrary bit width
		rdt.Height = pTxtrData->height; // if this were a higher dimension resource, this would likely be larger than 1
		rdt.DepthOrArraySize = 1; // same as previous comment
		rdt.MipLevels = 1; // specify which level of mipmapping to use (1 means no mipmapping)
		rdt.Format = pTxtrData->giPixelFormat; // Specifies what type of byte format to use
		rdt.SampleDesc.Count = 1; // the amount of samples
		rdt.SampleDesc.Quality = 0; // the quality of the anti aliasing (0 means off)
		rdt.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // define the layout.
		rdt.Flags = D3D12_RESOURCE_FLAG_NONE; // specify various other options related to access and usage
	}
	DXContext::Get().GetDevice()->CreateCommittedResource(&hpDefault, D3D12_HEAP_FLAG_NONE, &rdt, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pTexture));
	D3D12_DESCRIPTOR_HEAP_DESC dhd{};
	{
		dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		dhd.NumDescriptors = 1;
		dhd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		dhd.NodeMask = 0;
	}
	DXContext::Get().GetDevice()->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&srvHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
	{
		srv.Format = pTxtrData->giPixelFormat;
		srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv.Texture2D.MipLevels = 1;
		srv.Texture2D.MostDetailedMip = 0;
		srv.Texture2D.PlaneSlice = 0;
		srv.Texture2D.ResourceMinLODClamp = 0.0f;
	}
	DXContext::Get().GetDevice()->CreateShaderResourceView(pTexture, &srv, srvHeap->GetCPUDescriptorHandleForHeapStart());
	
	m_srvHeaps.push_back(srvHeap);
	m_textures.push_back(pTexture);
	m_txtrData.push_back(pTxtrData);

	uint32_t textureStride = pTxtrData->width * ((pTxtrData->bpp + 7) / 8);
	uint32_t textureSize = pTxtrData->height * textureStride;
	uploadBufferSize += textureSize;
}

void DXDataHandler::ExecuteUploadToGPU() 
{
	uploadBuffer.Release(); // May not be needed
	D3D12_HEAP_PROPERTIES hpUpload{};
	{
		hpUpload.Type = D3D12_HEAP_TYPE_UPLOAD; // What type of heap this is is. We specify one that uploads to the GPU
		hpUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; // Which memory pool (RAM or vRAM) is preferred 
		hpUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; // Specifies how the GPU can access the memory
		hpUpload.CreationNodeMask = 0; // Which GPU the heap shall be stored. We choose the first one.
		hpUpload.VisibleNodeMask = 0; // Where the memory can be seen from
	}
	D3D12_RESOURCE_DESC rdu{};
	{
		rdu.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // how many dimensions this resource has
		rdu.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		rdu.Width = uploadBufferSize; // width of the buffer.
		rdu.Height = 1; // if this were a higher dimension resource, this would likely be larger than 1
		rdu.DepthOrArraySize = 1; // same as previous comment
		rdu.MipLevels = 1; // specify which level of mipmapping to use (1 means no mipmapping)
		rdu.Format = DXGI_FORMAT_UNKNOWN; // Specifies what type of byte format to use
		rdu.SampleDesc.Count = 1; // the amount of samples
		rdu.SampleDesc.Quality = 0; // the quality of the anti aliasing (0 means off)
		rdu.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // define the texture layout. Other values will mess with the order of our buffer, so we choose row major (in order)
		rdu.Flags = D3D12_RESOURCE_FLAG_NONE; // specify various other options related to access and usage
	}
	DXContext::Get().GetDevice()->CreateCommittedResource(&hpUpload, D3D12_HEAP_FLAG_NONE, &rdu, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));

	char* uploadBufferAddress = nullptr;
	D3D12_RANGE uploadRange;
	uploadRange.Begin = 0;
	uploadRange.End = rdu.Width;

	// Copy data to CPU memory (upload buffer)
	uploadBuffer->Map(0, &uploadRange, (void**)&uploadBufferAddress);
	int uplBuffOffset = 0;
	for (int i = 0; i < m_txtrData.size(); i++)
	{
		auto* pTextureData = m_txtrData.at(i);

		uint32_t textureStride = pTextureData->width * ((pTextureData->bpp + 7) / 8);
		uint32_t textureSize = pTextureData->height * textureStride;
		memcpy(&uploadBufferAddress[uplBuffOffset], pTextureData->data.data(), textureSize);
		uplBuffOffset += textureSize;
	}
	for (int i = 0; i < m_vbvData.size(); i++)
	{
		size_t vbSize = m_vbViews.at(i)->SizeInBytes;
		auto* pVertexData = m_vbvData.at(i);

		memcpy(&uploadBufferAddress[uplBuffOffset], pVertexData, vbSize);
		uplBuffOffset += vbSize;
	}
	for (int i = 0; i < m_ibvData.size(); i++)
	{
		size_t ibSize = m_ibViews.at(i)->SizeInBytes;
		auto* pIndexData = m_ibvData.at(i);

		memcpy(&uploadBufferAddress[uplBuffOffset], pIndexData, ibSize);
		uplBuffOffset += ibSize;
	}
	uploadBuffer->Unmap(0, &uploadRange);

	// Upload data to GPU memory (respective buffers/heaps)
	uplBuffOffset = 0;
	for (int i = 0; i < m_textures.size(); i++)
	{
		auto* pTextureData = m_txtrData.at(i);
		auto* pTexture = m_textures.at(i);
		uint32_t textureStride = pTextureData->width * ((pTextureData->bpp + 7) / 8);
		uint32_t textureSize = pTextureData->height * textureStride;
		D3D12_BOX textureSizeAsBox;
		D3D12_TEXTURE_COPY_LOCATION txtcSrc, txtcDst;
		{
			textureSizeAsBox.left = textureSizeAsBox.top = textureSizeAsBox.front = 0;
			textureSizeAsBox.right = pTextureData->width;
			textureSizeAsBox.bottom = pTextureData->height;
			textureSizeAsBox.back = 1;

			txtcSrc.pResource = uploadBuffer; 
			txtcSrc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			txtcSrc.PlacedFootprint.Offset = uplBuffOffset;
			txtcSrc.PlacedFootprint.Footprint.Width = pTextureData->width;
			txtcSrc.PlacedFootprint.Footprint.Height = pTextureData->height;
			txtcSrc.PlacedFootprint.Footprint.Depth = 1;
			txtcSrc.PlacedFootprint.Footprint.RowPitch = textureStride;
			txtcSrc.PlacedFootprint.Footprint.Format = pTextureData->giPixelFormat;

			txtcDst.pResource = pTexture;
			txtcDst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			txtcDst.PlacedFootprint.Offset = 0;
		}
		cmdList->CopyTextureRegion(&txtcDst, 0, 0, 0, &txtcSrc, &textureSizeAsBox);

		D3D12_RESOURCE_BARRIER textureBarr; // Texture Resource Barrier
		{
			textureBarr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; // Specify the type of resource barrier
			textureBarr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE; // Specifies whether the barrier should be floating; if so, what type.
			textureBarr.Transition.pResource = pTexture; // The resource...
			textureBarr.Transition.Subresource = 0; // ...and its subresources
			textureBarr.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST; // Specify the state before...
			textureBarr.Transition.StateAfter = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE; // ...and the state after
		}
		barrierVec.push_back(textureBarr);

		uplBuffOffset += textureSize;
	}
	for (int i = 0; i < m_vertexBuffers.size(); i++)
	{
		size_t vbSize = m_vbViews.at(i)->SizeInBytes;
		ID3D12Resource2* pVertexBuffer = m_vertexBuffers.at(i);
		cmdList->CopyBufferRegion(pVertexBuffer, 0, uploadBuffer, uplBuffOffset, vbSize);

		D3D12_RESOURCE_BARRIER vertBuffBarr; // Vertex Buffer Resource Barrier
		{
			vertBuffBarr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; // Specify the type of resource barrier
			vertBuffBarr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE; // Specifies whether the barrier should be floating; if so, what type.
			vertBuffBarr.Transition.pResource = pVertexBuffer; // The resource...
			vertBuffBarr.Transition.Subresource = 0; // ...and its subresources
			vertBuffBarr.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST; // Specify the state before...
			vertBuffBarr.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER; // ...and the state after
		}
		barrierVec.push_back(vertBuffBarr);

		uplBuffOffset += vbSize;
	}
	for (int i = 0; i < m_indexBuffers.size(); i++)
	{
		size_t ibSize = m_ibViews.at(i)->SizeInBytes;
		auto* pIndexBuffer = m_indexBuffers.at(i);
		cmdList->CopyBufferRegion(pIndexBuffer, 0, uploadBuffer, uplBuffOffset, ibSize);

		D3D12_RESOURCE_BARRIER indexBuffBarr; //Index Resource Barrier
		{
			indexBuffBarr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; // Specify the type of resource barrier
			indexBuffBarr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE; // Specifies whether the barrier should be floating; if so, what type.
			indexBuffBarr.Transition.pResource = pIndexBuffer; // The resource...
			indexBuffBarr.Transition.Subresource = 0; // ...and its subresources
			indexBuffBarr.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST; // Specify the state before...
			indexBuffBarr.Transition.StateAfter = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE; // ...and the state after
		}
		barrierVec.push_back(indexBuffBarr);

		uplBuffOffset += ibSize;
	}

	cmdList->ResourceBarrier(barrierVec.size(), &barrierVec[0]);
	DXContext::Get().ExecuteCommandList();
	uploadBufferSize = 0; // May not be needed

	// Complete Views (obtain GPU virtual addresses)
	for (int i = 0; i < m_vbViews.size(); i++)
	{
		m_vbViews.at(i)->BufferLocation = m_vertexBuffers.at(i)->GetGPUVirtualAddress();
	}
	for (int i = 0; i < m_ibViews.size(); i++)
	{
		m_ibViews.at(i)->BufferLocation = m_indexBuffers.at(i)->GetGPUVirtualAddress();
	}
}
