#pragma once

#define NOMINMAX // 

#include <Windows.h> // Windows' window API

#include <d3d12.h> // DirectX12 Graphics API
#include <dxgi1_6.h> // DirectX Graphics Infrastructure (latest)
#include <DirectXMath.h> 
#include <d3dx12.h> // DirectX12 helper structs and inline functions
#include <d3dx12_root_signature.h>

// DirectXTK12 
#include "BufferHelpers.h"
#include "CommonStates.h"
#include "DDSTextureLoader.h"
#include "DescriptorHeap.h"
#include "DirectXHelpers.h"
#include "EffectPipelineStateDescription.h"
#include "Effects.h"
#include "GamePad.h"
#include "GeometricPrimitive.h"
#include "GraphicsMemory.h"
#include "Keyboard.h"
#include "Model.h" // Model Drawing from .cmo, .vbo, .sdkmesh files (DirectX12TK)
#include "Mouse.h"
#include "PostProcess.h"
#include "PrimitiveBatch.h"
#include "RenderTargetState.h"
#include "ResourceUploadBatch.h"
#include "ScreenGrab.h"
#include "SimpleMath.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "VertexTypes.h"
#include "WICTextureLoader.h"


#ifdef _DEBUG // Include debug layer headers, if this is a debug build
#include <d3d12sdklayers.h>
#include <dxgidebug.h>
#endif

#include <wincodec.h>