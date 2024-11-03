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

void DXDataHandler::CreateGPUVertexBuffer(ID3D12Resource2* pVertexBuffer, void* data, size_t dataStride)
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
        rdv.Width = sizeof(data); // width of the buffer.
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
    D3D12_VERTEX_BUFFER_VIEW vbv{};
    {
        vbv.BufferLocation = pVertexBuffer->GetGPUVirtualAddress();
        vbv.SizeInBytes = sizeof(data);
        vbv.StrideInBytes = dataStride;
        typeid(vbv);
    }
    m_vbViews.push_back(vbv);
    m_vbvData.push_back(data);
    uploadBufferSize += sizeof(data);
}
