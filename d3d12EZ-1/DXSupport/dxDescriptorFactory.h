#pragma once

#include <iostream>

#include <WinSupport/WinInclude.h>



// Abstract Product Class
class Descriptor 
{
public:
	virtual ~Descriptor() {}
};

// Concrete Product Classes
class HeapProps : public Descriptor 
{
public:
	D3D12_HEAP_PROPERTIES props;
};

class RescDesc : public Descriptor 
{
public:
	D3D12_RESOURCE_DESC desc;
};


class InptElmtDesc : public Descriptor 
{
public:
	D3D12_INPUT_ELEMENT_DESC desc;

};

// Absract Factory Class
class DescriptorFactory 
{
public:
	virtual Descriptor* createDesc() = 0;
	virtual Descriptor* createDesc(int type) = 0;
	virtual ~DescriptorFactory() {}
};

// Concrete Factory Classes
class HeapPropsFactory : public DescriptorFactory 
{
public:
	Descriptor* createDesc(int type) override
	{
		HeapProps* hp = new HeapProps;

		if (type < 1 || type > 5)
		{
			return nullptr;
		}

		hp->props.Type = (D3D12_HEAP_TYPE) type; // What type of heap this is is. We specify one that uploads to the GPU
		hp->props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; // Which memory pool (RAM or vRAM) is preferred 
		hp->props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; // Specifies how the GPU can access the memory
		hp->props.CreationNodeMask = 0; // Which GPU the heap shall be stored. We choose the first one.
		hp->props.VisibleNodeMask = 0; // Where the memory can be seen from
		return hp;
	}
};

class RescDescFactory : public DescriptorFactory
{
public:
	Descriptor* createDesc(int type) override
	{
		return new RescDesc();
	}
};
