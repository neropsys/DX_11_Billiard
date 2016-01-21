#pragma once
#include "d3dUtility.h"


IDXGISwapChain* swapChain=nullptr;
ID3D11DeviceContext* deviceContext;
ID3D11RenderTargetView* renderTargetView;
ID3D11Texture2D* depthStencilBuffer;
ID3D11DepthStencilState* depthStencilState;
ID3D11DepthStencilView* depthStencilView;
ID3D11RasterizerState* normalState;
ID3D11RasterizerState* wireframeState;