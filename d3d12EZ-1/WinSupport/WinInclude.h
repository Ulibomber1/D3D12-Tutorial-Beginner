#pragma once

#define NOMINMAX // 

#include <Windows.h> // Windows' window API

#include <d3d12.h> // DirectX12 Graphics API
#include <dxgi1_6.h> // DirectX Graphics Infrastructure (latest)
#include <DirectXMath.h> 
#include <d3dx12.h>
#include <d3dx12_root_signature.h>

#ifdef _DEBUG // Include debug layer headers, if this is a debug build
#include <d3d12sdklayers.h>
#include <dxgidebug.h>
#endif

#include <wincodec.h>