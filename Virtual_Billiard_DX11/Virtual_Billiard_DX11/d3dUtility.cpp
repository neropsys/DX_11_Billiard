/*!
* \file d3dUtility.cpp
* \date 2016/01
*
* \author Hyuk-jae Chang, ¿Â«ı¿Á
* Contact: neropsys@gmail.com
*/

#include "d3dUtility.h"

using namespace DirectX;

ID3D11DeviceContext* d3d::deviceContext = nullptr;
ID3D11RenderTargetView* d3d::renderTargetView = nullptr;
IDXGISwapChain* d3d::swapChain = nullptr;
ID3D11Texture2D* d3d::depthStencilBuffer = nullptr;
ID3D11DepthStencilState* d3d::depthStencilState = nullptr;
ID3D11DepthStencilView* d3d::depthStencilView = nullptr;
ID3D11RasterizerState* d3d::normalState = nullptr;

bool d3d::InitD3D(
	HINSTANCE hInstance,
	int width, int height,
	bool windowed,
	ID3D11Device** device
	)
{
	WNDCLASS wc;

	//setup window class with default setting
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)d3d::WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = "Direct3D11App";

	//register window class
	if (!RegisterClass(&wc))
	{
		::MessageBox(0, "RegisterClass() - FAILED", 0, 0);
		return false;
	}

	HWND hwnd = 0;

	//creates window and get the handle to it
	hwnd = ::CreateWindow("Direct3D11App",
		"Virtual Billiard",
		WS_EX_TOPMOST,//WS_POPUP, <- use WS_POPUP if you want borderless window
		0, 0, width, height,
		0 /*parent hwnd*/, 0 /* menu */, hInstance, 0 /*extra*/);

	if (!hwnd)
	{
		::MessageBox(0, "CreateWindow() - FAILED", 0, 0);
		return false;
	}

	//set window position
	int xPos = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	int yPos = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
	SetWindowPos(hwnd, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	::ShowWindow(hwnd, SW_SHOW);
	::UpdateWindow(hwnd);

	RECT rc;
	GetClientRect(hwnd, &rc);

	UINT w = rc.right - rc.left;
	UINT h = rc.bottom - rc.top;
	HRESULT result = 0;

//
// Init D3D: 
//

	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	ID3D11DeviceContext1* deviceContext1 = nullptr;
	ID3D11Device1* device1 = nullptr;
	IDXGISwapChain1*        swapChain1 = nullptr;
	HRESULT hr = S_OK;

	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;



	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, device, &featureLevel, &deviceContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, device, &featureLevel, &deviceContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = (*device)->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr))
		return hr;

	// Create swap chain
	IDXGIFactory2* dxgiFactory2 = nullptr;
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		hr = (*device)->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&device1));
		if (SUCCEEDED(hr))
		{
			(void)deviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&deviceContext1));
		}

		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;

		hr = dxgiFactory2->CreateSwapChainForHwnd(*device, hwnd, &sd, nullptr, nullptr, &swapChain1);
		if (SUCCEEDED(hr))
		{
			hr = swapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&swapChain));
		}

		dxgiFactory2->Release();
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hwnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		hr = dxgiFactory->CreateSwapChain(*device, &sd, &swapChain);
	}

	// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

	dxgiFactory->Release();

	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
		return hr;

	hr = (*device)->CreateRenderTargetView(pBackBuffer, nullptr, &renderTargetView);
	pBackBuffer->Release();


	//point for depth buffer
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	//setup description of the depth buffer
	depthBufferDesc.Width = width;
	depthBufferDesc.Height = height;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	//create texture for depth buffer using filled out description
	result = (*device)->CreateTexture2D(&depthBufferDesc, NULL, &depthStencilBuffer);
	if (FAILED(result)) {
		OutputDebugStringW(L"failed to create texture for the depth buffer using description.\r\n ");
		return false;
	}

	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	//setup description of the stencil state
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	//create depth stencil state
	result = (*device)->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
	if (FAILED(result)) {
		OutputDebugStringW(L"Failed to create depth stencil state.\r\n ");
		return false;
	}

	//set depth stencil state
	deviceContext->OMSetDepthStencilState(depthStencilState, 1);

	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	//setup deth stencil view description
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	//create depth stencil view
	result = (*device)->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &depthStencilView);
	if (FAILED(result)) {
		OutputDebugStringW(L"Failed to create depth stencil view. \r\n ");
		return false;
	}


	if (FAILED(hr))
		return hr;

	deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	deviceContext->RSSetViewports(1, &vp);



	return true;
}

int d3d::EnterMsgLoop(bool(*ptr_display)(float timeDelta))
{
	MSG msg;
	::ZeroMemory(&msg, sizeof(MSG));
	
	static double lastTime = (double)timeGetTime();
	

	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			double currTime = (double)timeGetTime();
			double timeDelta = (currTime - lastTime) * 0.0007;
			ptr_display((float)timeDelta);
			lastTime = currTime;
			/*
			__int64 currTime;
			QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

			__int64 countsPerSec;
			QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);

			mSecondsPerCount = 1.0 / (double)countsPerSec;

			mCurrTime = currTime;

			__int64 deltaTime = (mCurrTime - mPrevTime) * mSecondsPerCount;
			ptr_display((float)deltaTime);
			mPrevTime = currTime;
			*/
		}
	}

	return msg.wParam;
}

void d3d::CleanUp()
{
	if (swapChain)
		swapChain->SetFullscreenState(FALSE, nullptr);
	Release(swapChain);
	Release(normalState);
	Release(deviceContext);
	Release(renderTargetView);
	Release(depthStencilState);
	Release(depthStencilBuffer);
	Release(depthStencilView);

}

void d3d::BeginScene()
{
	float color[4];
	color[0] = 0.7f;//red
	color[1] = 0.7f;//green
	color[2] = 0.7f;//blue
	color[3] = 1;//alpha
	d3d::deviceContext->ClearRenderTargetView(d3d::renderTargetView, color);
	d3d::deviceContext->ClearDepthStencilView(d3d::depthStencilView, D3D11_CLEAR_DEPTH, 1.f, 0);
}

void d3d::EndScene()
{
	//end scene
	d3d::swapChain->Present(0, 0);
}



