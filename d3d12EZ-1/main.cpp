#include <iostream>

#include <WinInclude.h>
#include <ComPointer.h>
#include <DXWindow.h>

#include <DXDebugLayer.h>

#include <DXContext.h>

int main()
{
	DXDebugLayer::Get().Init();
	if (DXContext::Get().Init() && DXWindow::Get().Init())
	{
		while (!DXWindow::Get().ShouldClose())
		{
			DXWindow::Get().Update();
			auto* cmdList = DXContext::Get().InitCommandList();

			// setup
			
			// draw/render

			DXContext::Get().ExecuteCommandList();
			
			// Show the render

		}

		DXWindow::Get().Shutdown();
		DXContext::Get().Shutdown();
	}
	DXDebugLayer::Get().Shutdown();
}