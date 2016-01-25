/*!
* \file d3dUtility.cpp
* \date 2016/01
*
* \author Hyuk-jae Chang, ÀåÇõÀç
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
		WS_POPUP,
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


	//
	// Init D3D: 
	//

	HRESULT result = 0;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, i, numerator, denominator, stringLength;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error, videoCardMemory;
	char videoCardDesc[128];
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D* backBufferPtr;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC normalRasterDesc;
	D3D11_VIEWPORT viewport;

	//creates directX graphics interface factory
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to create dxgifactory.\r\n ");
		return false;
	}

	//use factory to create an adapter for primary graphics interface
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to to create an adapter for the primary graphics interface.\r\n ");
		return false;
	}
	//enumerate primary adapter output(monitor)
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to enumerate primary adapter output.\r\n ");
		return false;
	}

	//get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output(monitor)
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output.\r\n ");
		return false;
	}

	//crate a list to hold all possible display modes for this monitor/video combination
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList){
		OutputDebugStringW(L"Failed to create a list to hold all the possible display modes for this monitor/video card combination.\r\n ");
		return false;
	}
	//fill the display mode list structure
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to fill the display mode list structures.\r\n ");
		return false;
	}

	//go through all the display modes and find the one that matches the screen width & height
	//when match is found, store the numerator and denominator of the refresh rate for that monitor
	for (i = 0; i < numModes; i++){
		if (displayModeList[i].Width == (unsigned int)width){
			if (displayModeList[i].Height == (unsigned int)height){
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	//gett the adapter(video card) description;
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to get the adapter description.\r\n ");
		return false;
	}

	//store the dedicated video card memory in megabyte
	videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	//convert name of the video card to character array and store 
	error = wcstombs_s(&stringLength, videoCardDesc, 128, adapterDesc.Description, 128);
	if (error != 0){
		OutputDebugStringW(L"Failed to get the video card name \r\n");
		return false;

	}

	//cleanup 
	delete[] displayModeList;
	displayModeList = nullptr;

	adapterOutput->Release();
	adapterOutput = nullptr;

	adapter->Release();
	adapter = nullptr;

	factory->Release();
	factory = nullptr;

	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	//set to single back buffer and set the width & height
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;

	//set regular 32bit surface for back buffer
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	//set refresh rate(vsync disabled)
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

	//set usage of back buffer
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	//set handle for window to render to
	swapChainDesc.OutputWindow = hwnd;

	//multisampling off
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.Windowed = true;

	//set scan line ordering and scaling to unspecified
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//discard back buffer content after present
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	//no advanced flag
	swapChainDesc.Flags = 0;

	//dx11 feature level
	featureLevel = D3D_FEATURE_LEVEL_11_0;


	//create swap chain, direct3d device, and device context
	//set 4th parameter to D3D11_CREATE_DEVICE_DEBUG if you have any problem debugging the code
	result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &swapChain, device, nullptr, &deviceContext);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to create device and swapchain");
		return false;
	}

	//get ptr to back buffer
	result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to get the pointer to the back buffer.\r\n ");
		return false;
	}
	//create render target view with back buffer ptr
	result = (*device)->CreateRenderTargetView(backBufferPtr, nullptr, &renderTargetView);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to create render target view with back buffer pointer.\r\n ");
		return false;
	}

	//cleanup back buffer ptr
	backBufferPtr->Release();
	backBufferPtr = nullptr;

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
	if (FAILED(result)){
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
	if (FAILED(result)){
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
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to create depth stencil view. \r\n ");
		return false;
	}

	//bind render target view
	deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	//setup raster description which will determine how and what polygons will be drawn
	normalRasterDesc.AntialiasedLineEnable = true;
	normalRasterDesc.CullMode = D3D11_CULL_BACK;
	normalRasterDesc.DepthBias = 0;
	normalRasterDesc.DepthBiasClamp = 0.f;
	normalRasterDesc.DepthClipEnable = true;
	normalRasterDesc.FillMode = D3D11_FILL_SOLID;
	normalRasterDesc.FrontCounterClockwise = true;
	normalRasterDesc.MultisampleEnable = true;
	normalRasterDesc.ScissorEnable = false;
	normalRasterDesc.SlopeScaledDepthBias = 0.0f;


	//create rasterizer state from description above
	result = (*device)->CreateRasterizerState(&normalRasterDesc, &normalState);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to get normal rasterizer state. \r\n ");
		return false;
	}
	
	//set rasterizer state
	deviceContext->RSSetState(normalState);

	//setup and create viewport
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	deviceContext->RSSetViewports(1, &viewport);

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



