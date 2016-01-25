//Made by hjchang ¿Â«ı¿Á 2016-01
/*!
* \file d3dUtility.h
* \date 2016/01
*
* \author Hyuk-jae Chang, ¿Â«ı¿Á
* Contact: neropsys@gmail.com
*/
//Desc : Initializes D3D and lets the user define message loop


#pragma once


#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "winmm.lib")

#include <dxgi.h>
#include <d3d11.h>
#include <d3dcommon.h>
#include <DirectXMath.h>
namespace d3d
{
	bool InitD3D(
		HINSTANCE hInstance,
		int width, int height,
		bool windowed,
		ID3D11Device** device
		);

	int EnterMsgLoop(bool(*ptr_display)(float timeDelta));

	void CleanUp();

	void BeginScene();

	void EndScene();


	template<class T> void Release(T t)
	{
		if (t)
		{
			t->Release();
			t = nullptr;
		}
	}
	template<class T> void Delete(T t)
	{
		if (t)
		{
			delete t;
			t = nullptr;
		}
	}
	LRESULT CALLBACK WndProc(
		HWND hwnd,
		UINT msg,
		WPARAM wParam,
		LPARAM lParam);


	extern IDXGISwapChain* swapChain;
	extern ID3D11DeviceContext* deviceContext;
	extern ID3D11RenderTargetView* renderTargetView;
	extern ID3D11Texture2D* depthStencilBuffer;
	extern ID3D11DepthStencilState* depthStencilState;
	extern ID3D11DepthStencilView* depthStencilView;
	extern ID3D11RasterizerState* normalState;

	const float SCREEN_DEPTH = 100.0f;
	const float SCREEN_NEAR = 1.f;


}