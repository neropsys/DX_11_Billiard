
#include "d3dUtility.h"
#include "CWall.h"
#include <DirectXMath.h>
#include <ctime>


using namespace DirectX;

ID3D11Device* device = nullptr;

// window size
const int Width = 800;
const int Height = 600;

XMMATRIX g_mWorld;
XMMATRIX g_mView;
XMMATRIX g_mProj;

static bool wireMode = false;

CWall g_legoPlane;
CWall g_legowall[4];

// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
bool Display(float timeDelta)
{
	int i = 0;
	if (device)
	{

		d3d::BeginScene();

		//render shit
		g_legoPlane.draw(g_mWorld, g_mView, g_mProj, wireMode);

		for (int i = 0; i < 4; i++){
			g_legowall[i].draw(g_mWorld, g_mView, g_mProj, wireMode);
		}


		d3d::EndScene();
		return true;	
	}
	return false;


}
bool Setup(){

	//setup model and some shit
	if (false == g_legoPlane.create(9, 0.03f, 6, Colors::DarkGreen)) return false;
	g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f);

	if (false == g_legowall[0].create(9, 0.3f, 0.12f, Colors::DarkRed)) return false;
	g_legowall[0].setPosition(0.0f, 0.12f, -3.06f);

	if (false == g_legowall[1].create(9, 0.3f, 0.12f, Colors::DarkRed)) return false;
	g_legowall[1].setPosition(0.0f, 0.12f, 3.06f);

	if (false == g_legowall[2].create(0.12f, 0.3f, 6.24f, Colors::DarkRed)) return false;
	g_legowall[2].setPosition(4.56f, 0.12f, 0.0f);

	if (false == g_legowall[3].create(0.12f, 0.3f, 6.24f, Colors::DarkRed)) return false;
	g_legowall[3].setPosition(-4.56f, 0.12f, 0.0f);



	//setup position and aim the camera
	XMFLOAT3 pos(0.f, 8.f, 8.f);
	XMFLOAT3 target(0.f, 0.f, 0.f);
	XMFLOAT3 up(0.f, -2.f, 0.f);

	g_mView = XMMatrixLookAtLH(XMLoadFloat3(&pos), XMLoadFloat3(&target), XMLoadFloat3(&up));
	g_mWorld = d3d::getWorldMatrix();
	g_mProj = d3d::getProjectionMatrix();


	return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){

	static bool isReset = true;
	static int old_x = 0;
	static int old_y = 0;
	static enum { WORLD_MOVE, LIGHT_MOVE, BLOCK_MOVE } move = WORLD_MOVE;

	switch (msg) {
	case WM_DESTROY:
	{
		::PostQuitMessage(0);
		break;
	}
	case WM_KEYDOWN:
	{
		switch (wParam) {
		case VK_ESCAPE:
			::DestroyWindow(hwnd);
			break;
		case VK_RETURN:
			if (NULL != device) {
				wireMode = !wireMode;				
			}
			break;
		case VK_SPACE:
			//game event
			/*
			D3DXVECTOR3 targetpos = g_target_blueball.getCenter();
			D3DXVECTOR3	whitepos = g_sphere[3].getCenter();
			double theta = acos(sqrt(pow(targetpos.x - whitepos.x, 2)) / sqrt(pow(targetpos.x - whitepos.x, 2) +
			pow(targetpos.z - whitepos.z, 2)));		// 기본 1 사분면
			if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x >= 0) { theta = -theta; }	//4 사분면
			if (targetpos.z - whitepos.z >= 0 && targetpos.x - whitepos.x <= 0) { theta = PI - theta; } //2 사분면
			if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x <= 0){ theta = PI + theta; } // 3 사분면
			double distance = sqrt(pow(targetpos.x - whitepos.x, 2) + pow(targetpos.z - whitepos.z, 2));
			g_sphere[3].setPower(distance * cos(theta), distance * sin(theta));
			*/
			break;
		}
		break;
	}

	case WM_MOUSEMOVE:
	{
		int new_x = LOWORD(lParam);
		int new_y = HIWORD(lParam);
		float dx;
		float dy;

		if (LOWORD(wParam) & MK_LBUTTON) {
			
			if (isReset) {
				isReset = false;
			}
			else {
				XMFLOAT3 vDist;
				XMFLOAT3 vTrans;
				XMMATRIX mTrans;
				XMMATRIX mX;
				XMMATRIX mY;
				switch (move) {
				case WORLD_MOVE:
					dx = (old_x - new_x) * 0.01f;
					dy = (old_y - new_y) * 0.01f;
					mX = XMMatrixRotationY(dx);
					mY = XMMatrixRotationX(dy);
					g_mWorld = g_mWorld * mX * mY;
					d3d::setWorldMatrix(g_mWorld);

					break;
				}
			}


			
		}
		else {
			/*/
			isReset = true;

			if (LOWORD(wParam) & MK_RBUTTON) {
				dx = (old_x - new_x);// * 0.01f;
				dy = (old_y - new_y);// * 0.01f;

				D3DXVECTOR3 coord3d = g_target_blueball.getCenter();
				g_target_blueball.setCenter(coord3d.x + dx*(-0.007f), coord3d.y, coord3d.z + dy*0.007f);
			}
			old_x = new_x;
			old_y = new_y;

			move = WORLD_MOVE;
			*/
		}
		old_x = new_x;
		old_y = new_y;

		break;
	}
	}

	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{
	srand(static_cast<unsigned int>(time(NULL)));
	if (!d3d::InitD3D(hinstance, Width, Height, true, &device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}
	if (!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}
	d3d::EnterMsgLoop(Display);
	d3d::CleanUp();

	return 0;
}