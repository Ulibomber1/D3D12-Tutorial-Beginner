#include <iostream>
#include <math.h> 
#include <chrono>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_dx12.h>

#include <WinSupport/WinInclude.h>
#include <WinSupport/ComPointer.h>
#include <DXSupport/ImageLoader.h>
#include <DXSupport/DXWindow.h>
#include <DXSupport/Shader.h>
#include <DXSupport/GPSODescBuilder.h>
#include <DXSupport/DXDataHandler.h>

#include <DXDebug/DXDebugLayer.h>

#include <DXSupport/DXContext.h>

using namespace DirectX;

void uncrnVomit(float* color, float delta);

// === 2D Vertex Data ===
struct Vertex2D
{
	XMFLOAT2 position;
	XMFLOAT2 texCoord;
};
Vertex2D vertices2D[] =
{
	{{ -1.0f, -1.0f}, {0.0f, 1.0f}},
	{{  0.0f,  1.0f}, {0.5f, 0.0f}},
	{{  1.0f, -1.0f}, {1.0f, 1.0f}}
};
D3D12_INPUT_ELEMENT_DESC vertexLayout2D[] =
{
	{"POSITION", 0,  DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0,  DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
};

// === 3D Vertex Data (Cube) ===
struct VertexCube
{
	XMFLOAT3 position;
	XMFLOAT3 color;
};
VertexCube verticesCube[] =
{
	{{ -1.0f, -1.0f, -1.0f}, {  0.0f,  0.0f,  0.0f}}, // 0 
	{{ -1.0f,  1.0f, -1.0f}, {  1.0f,  0.0f,  0.0f}}, // 1 
	{{  1.0f,  1.0f, -1.0f}, {  1.0f,  1.0f,  0.0f}}, // 2 
	{{  1.0f, -1.0f, -1.0f}, {  0.0f,  1.0f,  0.0f}}, // 3 
	{{ -1.0f, -1.0f,  1.0f}, {  0.0f,  1.0f,  1.0f}}, // 4 
	{{ -1.0f,  1.0f,  1.0f}, {  0.0f,  0.0f,  1.0f}}, // 5 
	{{  1.0f,  1.0f,  1.0f}, {  1.0f,  1.0f,  1.0f}}, // 6 
	{{  1.0f, -1.0f,  1.0f}, {  1.0f,  0.0f,  1.0f}}  // 7 
};												  
WORD cubeIndices[] =
{
	0, 1, 2, 0, 2, 3,
	4, 6, 5, 4, 7, 6,
	4, 5, 1, 4, 1, 0,
	3, 2, 6, 3, 6, 7,
	1, 5, 6, 1, 6, 2,
	4, 0, 3, 4, 3, 7
};
UINT numIndices = (UINT)std::size(cubeIndices);
D3D12_INPUT_ELEMENT_DESC vertexLayout3D[] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
};

const wchar_t* sdkmeshPath = L"app/SDKMESH/";

int main()
{
	DXDebugLayer::Get().Init();
	if (DXContext::Get().Init() && DXWindow::Get().Init() && XMVerifyCPUSupport())
	{
		if (!DXDataHandler::Get().Init()) std::cout << "Failure to initialize data handler!";

		// === ImGui Context ===
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		// === ImGui Backends ===
		ImGui_ImplWin32_Init(DXWindow::Get().GetWindow());
		D3D12_DESCRIPTOR_HEAP_DESC imguiDHDesc;
		{
			imguiDHDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			imguiDHDesc.NumDescriptors = 1;
			imguiDHDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			imguiDHDesc.NodeMask = 0;
		}
		ComPointer<ID3D12DescriptorHeap> imguiDescHeap;
		DXContext::Get().GetDevice()->CreateDescriptorHeap(&imguiDHDesc, IID_PPV_ARGS(&imguiDescHeap));
		ImGui_ImplDX12_Init(DXContext::Get().GetDevice(), DXWindow::Get().GetFrameCount(), DXWindow::Get().GetBufferFormat(), 
			imguiDescHeap, 
			imguiDescHeap->GetCPUDescriptorHandleForHeapStart(), 
			imguiDescHeap->GetGPUDescriptorHandleForHeapStart());

		// === Cup Vertex Data ===
		auto cupMesh = Model::CreateFromSDKMESH(DXContext::Get().GetDevice(), L"cup.sdkmesh");
		ComPointer<CommonStates> m_states;
		m_states = &CommonStates(DXContext::Get().GetDevice());
		ComPointer<EffectFactory> m_fxFactory;
		ComPointer<EffectTextureFactory> m_modelResources;
		Model::EffectCollection m_modelNormal;

		// === Texture Data ===
		ImageLoader::ImageData textureData;
		ImageLoader::LoadImageFromDisk("./auge_512_512_BGRA_32BPP.png", textureData);
		ComPointer<ID3D12Resource2> texture;
		ComPointer<ID3D12DescriptorHeap> srvHeap;
		DXDataHandler::Get().CreateGPUTexture(texture, srvHeap, &textureData); // srvHeap,

		// === Buffer and View Creation ===
		ComPointer<ID3D12Resource2> vertexBuffer, vertexBuffer2;
		D3D12_VERTEX_BUFFER_VIEW vbv{}, vbv2{};
		ComPointer<ID3D12Resource2> indexBuffer;
		D3D12_INDEX_BUFFER_VIEW ibv{};
		DXDataHandler::Get().CreateGPUVertexBuffer(vertexBuffer, &vbv, &vertices2D, sizeof(vertices2D), sizeof(Vertex2D));
		DXDataHandler::Get().CreateGPUVertexBuffer(vertexBuffer2, &vbv2, &verticesCube, sizeof(verticesCube), sizeof(VertexCube));
		DXDataHandler::Get().CreateGPUIndexBuffer(indexBuffer, &ibv, &cubeIndices, sizeof(cubeIndices));

		// === Execute Resource Uploads === 
		DXDataHandler::Get().ExecuteUploadToGPU();

		// === Shaders ===
		Shader vertexShader("VertexShader.cso");
		Shader pixelShader("PixelShader.cso");
		Shader vertexShader2D("VertexShader2D.cso");
		Shader pixelShader2D("PixelShader2D.cso");

		// === Create Root Sig ===
		ComPointer<ID3D12RootSignature> rootSignature;
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc = {};
		{
			CD3DX12_DESCRIPTOR_RANGE descRange[1] = {};
			descRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
			CD3DX12_ROOT_PARAMETER rootParams[3] = {};
			rootParams[0].InitAsConstants(4, 0);
			rootParams[1].InitAsConstants(sizeof(XMMATRIX) / 4, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX);
			rootParams[2].InitAsDescriptorTable(1, &descRange[0]);
			D3D12_ROOT_SIGNATURE_FLAGS rootSigFlags =
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
			CD3DX12_STATIC_SAMPLER_DESC staticSampler[1] = {};
			staticSampler[0].Init(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);
			rootSigDesc.Init((UINT)std::size(rootParams), rootParams, 1, staticSampler, rootSigFlags);
		}
		ComPointer<ID3DBlob> rootSigBlob;
		ComPointer<ID3DBlob> errorBlob;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSigBlob, &errorBlob);
		if (FAILED(hr))
		{
			std::cout << "Root Signature did not compile!";
			return -2;
		}
		DXContext::Get().GetDevice()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

		// === Pipeline State Object Description ===
		GPSODescBuilder2D PSObuilder2D(rootSignature, vertexLayout2D, (UINT)_countof(vertexLayout2D), &vertexShader2D, &pixelShader2D, (Shader*)nullptr, (Shader*)nullptr, (Shader*)nullptr); // we pass vertexLayout array since it will decay to a pointer anyways
		GPSODescBuilder3D PSObuilder3D(rootSignature, vertexLayout3D, (UINT)_countof(vertexLayout3D), &vertexShader, &pixelShader, (Shader*)nullptr, (Shader*)nullptr, (Shader*)nullptr); // we pass vertexLayout array since it will decay to a pointer anyways
		GPSODescDirector PSOdirector;
		PSOdirector.Construct(PSObuilder2D);
		PSOdirector.Construct(PSObuilder3D);
		D3D12_GRAPHICS_PIPELINE_STATE_DESC gfxPsod2D = PSObuilder2D.GetDescriptor();
		D3D12_GRAPHICS_PIPELINE_STATE_DESC gfxPsod3D = PSObuilder3D.GetDescriptor();
		ComPointer<ID3D12PipelineState> pso;
		ComPointer<ID3D12PipelineState> pso2D;
		DXContext::Get().GetDevice()->CreateGraphicsPipelineState(&gfxPsod3D, IID_PPV_ARGS(&pso));
		DXContext::Get().GetDevice()->CreateGraphicsPipelineState(&gfxPsod2D, IID_PPV_ARGS(&pso2D));

		// === View and Projection Matrix ===
		XMMATRIX viewProjection;
		{
			// setup view mat
			auto eyePos = XMVectorSet(0, 0, -5, 1);
			auto focusPos = XMVectorSet(0, 0, 0, 1);
			auto upDir = XMVectorSet(0, 1, 0, 0);
			auto viewMtx = XMMatrixLookAtLH(eyePos, focusPos, upDir);
			// setup perspective
			auto aspectRatio = float(DXWindow::Get().GetWidth()) / float(DXWindow::Get().GetHeight());
			auto projectionMtx = XMMatrixPerspectiveFovLH(XMConvertToRadians(65.f), aspectRatio, 0.1f, 100.0f);
			// combine view and perspective matrices
			viewProjection = viewMtx * projectionMtx;
		}

		// === TIMING INIT ===
		std::chrono::steady_clock::time_point startRenderTime_nS;
		std::chrono::steady_clock::time_point endRenderTime_nS;
		std::chrono::duration<double> renderDurationSecs;
		double timeAccumulatedSecs = 0.0;
		float animTime = 0.f;
		float animStep = 0.005f;
		int frames = 0;
		
		// === RENDER LOOP ===
		ID3D12GraphicsCommandList6* cmdList;
		DXWindow::Get().SetFullscreen(true);
		while (!DXWindow::Get().ShouldClose())
		{
			// == Timed Updates (start) ==
			startRenderTime_nS = std::chrono::steady_clock::now();

			// == Poll Events ==
			DXWindow::Get().Update(); // Poll the window events, so that it's considered 'responding'
			bool is3D = DXWindow::Get().Is3D();
			if (DXWindow::Get().ShouldResize())// handle resizing
			{
				DXContext::Get().Flush(DXWindow::Get().GetFrameCount()); // Flush Command queue 
				DXWindow::Get().Resize();
			}

			// == Start ImGui Frame ==
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			//ImGui::ShowDemoWindow();

			// begin drawing
			cmdList = DXContext::Get().InitCommandList();
			DXWindow::Get().BeginFrame(cmdList);

			// == Pipeline State Object Updates ==
			cmdList->SetPipelineState(is3D ? pso : pso2D);
			cmdList->SetGraphicsRootSignature(rootSignature);
			cmdList->SetDescriptorHeaps(1, &srvHeap);

			// == Input Assembler Updates ==
			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			cmdList->IASetVertexBuffers(0, 1, is3D ? &vbv2 : &vbv);
			cmdList->IASetIndexBuffer(is3D ? &ibv : nullptr);

			// == Rasterizer Updates ==
			D3D12_VIEWPORT vp;
			{
				vp.TopLeftX = vp.TopLeftY = 0;
				vp.Width = (FLOAT)DXWindow::Get().GetWidth();
				vp.Height = (FLOAT)DXWindow::Get().GetHeight();
				vp.MinDepth = 0.f;
				vp.MaxDepth = 1.f;
			}
			cmdList->RSSetViewports(1, &vp);
			RECT scRect;
			{
				scRect.left = scRect.top = 0;
				scRect.right = DXWindow::Get().GetWidth();
				scRect.bottom = DXWindow::Get().GetHeight();
			}
			cmdList->RSSetScissorRects(1, &scRect);

			// == Misc. Updates ==
			static float color[] = { 0.0f, 0.0f, 1.0f };
			uncrnVomit(color, 0.0025f);
			XMMATRIX transformMatrix = XMMatrixTranspose(
				XMMatrixRotationX(2.0f * animTime + 0.f) *
				XMMatrixRotationY(4.0f * animTime + 5.f) *
				XMMatrixRotationZ(1.0f * animTime + 0.f) *
				viewProjection
			);

			// == Root Sig Updates ==
			cmdList->SetGraphicsRoot32BitConstants(0, 3, color, 0);
			cmdList->SetGraphicsRoot32BitConstants(1, sizeof(transformMatrix) / 4, &transformMatrix, 0);
			cmdList->SetGraphicsRootDescriptorTable(2, srvHeap->GetGPUDescriptorHandleForHeapStart());

			// == Draw == This might be where to refactor for DXRenderer
			is3D ?
				cmdList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0) :
				cmdList->DrawInstanced(_countof(vertices2D), 1, 0, 0);

			transformMatrix = XMMatrixTranspose(
				XMMatrixRotationX(0.2f * animTime + 0.f) *
				XMMatrixRotationY(0.4f * animTime + 1.f) *
				XMMatrixRotationZ(0.1f * animTime + 0.f) *
				viewProjection
			);
			cmdList->SetGraphicsRoot32BitConstants(1, sizeof(transformMatrix) / 4, &transformMatrix, 0);
			is3D ?
				cmdList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0) :
				void(); // Do nothing if in 2D

			// Render ImGui
			ImGui::Render();
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);

			// Finish and Show the render
			DXWindow::Get().EndFrame(cmdList); // set resource barrier for (current?) render target
			DXContext::Get().ExecuteCommandList(); // Execute cmdList, which is a cpu-thread-blocking function (the GPU command thread, via command allocator?)
			DXWindow::Get().Present(); // Present the last drawn frame that isn't discarded, possibly gpu-thread-blocking (via command queue)

			// == Timed Updates (end of frame) ==
			endRenderTime_nS = std::chrono::steady_clock::now();
			renderDurationSecs = std::chrono::duration_cast<std::chrono::duration<double>>(endRenderTime_nS - startRenderTime_nS);
			timeAccumulatedSecs += renderDurationSecs.count();
			frames++;
			animTime += renderDurationSecs.count();
			if (timeAccumulatedSecs > 1.0)
			{
				// Only accurate if WaitForSingleObject() on the fence event blocks the cpu thread
				// Previous comment may not be accurate, as chrono usage works as intended.
				std::cout << "AVG FRAME TIME: " << (timeAccumulatedSecs * 1000.0) / frames << " ms\n"; 
				std::cout << "APPROX. FPS: " << frames / timeAccumulatedSecs << "\n\n";
				frames = 0;
				timeAccumulatedSecs = 0;
			}
		}
		// Flush command queue
		DXContext::Get().Flush(DXWindow::Get().GetFrameCount());

		// Release Heap Memory (Buffers)
		DXDataHandler::Get().Shutdown();

		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		DXWindow::Get().Shutdown();
		DXContext::Get().Shutdown();
	}
	DXDebugLayer::Get().Shutdown();
}

void uncrnVomit(float* color, float delta)
{
	static int pukeState = 0;
	static float temp;

	color[pukeState] += delta;
	if (pukeState != 0)
	{
		color[pukeState - 1] -= delta;
		if (color[pukeState] > 1.0f) color[pukeState] = 1.0f;
		if (color[pukeState - 1] < 0.0f) color[pukeState - 1] = 0.0f;
		temp = float(sqrt(1.0f - pow(color[pukeState - 1], 2)));
		color[pukeState - 1] = float(sqrt(1.0f - pow(color[pukeState], 2)));
		color[pukeState] = temp;
	}
	else
	{
		color[2] -= delta;
		if (color[pukeState] > 1.0f) color[pukeState] = 1.0f;
		if (color[2] < 0.0f) color[2] = 0.0f;
		temp = float(sqrt(1.0f - pow(color[2], 2)));
		color[2] = float(sqrt(1.0f - pow(color[pukeState], 2)));
		color[pukeState] = temp;
	}

	if (color[pukeState] >= 1.0f)
	{
		color[pukeState] = 1.0f;
		pukeState++;
		if (pukeState == 3)
		{
			pukeState = 0;
		}
	}
}
