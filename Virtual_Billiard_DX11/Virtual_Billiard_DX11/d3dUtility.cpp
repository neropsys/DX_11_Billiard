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
	//setup window property
	WNDCLASS wc;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)d3d::WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = "Direct3D9App";

	if (!RegisterClass(&wc))
	{
		::MessageBox(0, "RegisterClass() - FAILED", 0, 0);
		return false;
	}

	HWND hwnd = 0;
	hwnd = ::CreateWindow("Direct3D9App",
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
	float fieldOfView, screenAspect;

	//creates dxgiFactory so that we can enumerate adapter
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to create dxgifactory.\r\n ");
		return false;
	}

	//enumerates adapters so that we can
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to to create an adapter for the primary graphics interface.\r\n ");
		return false;
	}
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to enumerate primary adapter output.\r\n ");
		return false;
	}

	//gets displaymode list
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output.\r\n ");
		return false;
	}
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList){
		OutputDebugStringW(L"Failed to create a list to hold all the possible display modes for this monitor/video card combination.\r\n ");
		return false;
	}

	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to fill the display mode list structures.\r\n ");
		return false;
	}

	for (i = 0; i < numModes; i++){
		if (displayModeList[i].Width == (unsigned int)width){
			if (displayModeList[i].Height == (unsigned int)height){
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to get the adapter description.\r\n ");
		return false;
	}
	videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	error = wcstombs_s(&stringLength, videoCardDesc, 128, adapterDesc.Description, 128);
	if (error != 0){
		OutputDebugStringW(L"Failed to get the video card name \r\n");
		return false;

	}

	delete[] displayModeList;
	displayModeList = nullptr;

	adapterOutput->Release();
	adapterOutput = nullptr;

	adapter->Release();
	adapter = nullptr;

	factory->Release();
	factory = nullptr;

	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;

	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;


	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	swapChainDesc.OutputWindow = hwnd;

	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.Windowed = true;

	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	swapChainDesc.Flags = 0;

	featureLevel = D3D_FEATURE_LEVEL_11_0;

	//set 4th parameter to D3D11_CREATE_DEVICE_DEBUG if you have any problem debugging the code

	result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &swapChain, device, nullptr, &deviceContext);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to create device and swapchain");
		return false;
	}
	result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to get the pointer to the back buffer.\r\n ");
		return false;
	}

	result = (*device)->CreateRenderTargetView(backBufferPtr, nullptr, &renderTargetView);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to create render target view with back buffer pointer.\r\n ");
		return false;
	}

	backBufferPtr->Release();
	backBufferPtr = nullptr;

	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

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

	result = (*device)->CreateTexture2D(&depthBufferDesc, NULL, &depthStencilBuffer);
	if (FAILED(result)){
		OutputDebugStringW(L"failed to create texture for the depth buffer using description.\r\n ");
		return false;
	}

	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

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

	result = (*device)->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to create depth stencil state.\r\n ");
		return false;
	}

	deviceContext->OMSetDepthStencilState(depthStencilState, 1);

	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = (*device)->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &depthStencilView);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to create depth stencil view. \r\n ");
		return false;
	}

	deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	normalRasterDesc.AntialiasedLineEnable = false;
	normalRasterDesc.CullMode = D3D11_CULL_BACK;
	normalRasterDesc.DepthBias = 0;
	normalRasterDesc.DepthBiasClamp = 0.f;
	normalRasterDesc.DepthClipEnable = true;
	normalRasterDesc.FillMode = D3D11_FILL_SOLID;
	normalRasterDesc.FrontCounterClockwise = true;
	normalRasterDesc.MultisampleEnable = false;
	normalRasterDesc.ScissorEnable = false;
	normalRasterDesc.SlopeScaledDepthBias = 0.0f;



	result = (*device)->CreateRasterizerState(&normalRasterDesc, &normalState);
	if (FAILED(result)){
		OutputDebugStringW(L"Failed to get normal rasterizer state. \r\n ");
		return false;
	}
	

	deviceContext->RSSetState(normalState);

	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	deviceContext->RSSetViewports(1, &viewport);

	fieldOfView = (float)DirectX::XM_PI *.25f;

	screenAspect = (float)width / (float)height;

	
	g_proj = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, d3d::SCREEN_NEAR, d3d::SCREEN_DEPTH);

	g_world = XMMatrixIdentity();

	//m_orthoMatrix = XMMatrixOrthographicLH(screenWidth, screenHeight, screenNear, screenDepth);

	return true;
}

int d3d::EnterMsgLoop(bool(*ptr_display)(float timeDelta))
{
	MSG msg;
	::ZeroMemory(&msg, sizeof(MSG));
	
	

	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			__int64 currTime;
			QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

			mCurrTime = currTime;

			__int64 deltaTime = (mCurrTime - mPrevTime) * 0.0007;
			ptr_display((float)deltaTime);
			mPrevTime = currTime;
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
	color[0] = 0.5f;//red
	color[1] = 0.5f;//green
	color[2] = 0.5f;//blue
	color[3] = 1;//alpha
	d3d::deviceContext->ClearRenderTargetView(d3d::renderTargetView, color);
	d3d::deviceContext->ClearDepthStencilView(d3d::depthStencilView, D3D11_CLEAR_DEPTH, 1.f, 0);
}

void d3d::EndScene()
{
	//end scene
	d3d::swapChain->Present(0, 0);
}

const DirectX::XMMATRIX d3d::getProjectionMatrix()
{
	//return XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);
	return d3d::g_proj;
}

const DirectX::XMMATRIX d3d::getWorldMatrix()
{
	return d3d::g_world;
}

void d3d::setWorldMatrix(const DirectX::XMMATRIX& matrix)
{
	d3d::g_world = matrix;
}
