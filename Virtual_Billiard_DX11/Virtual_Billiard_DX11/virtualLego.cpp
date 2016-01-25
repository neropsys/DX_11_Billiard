/*!
* \file virtualLego.cpp
* \date 2016/01
*
* \author Hyuk-jae Chang, 장혁재
* Contact: neropsys@gmail.com
*/
#include "d3dUtility.h"
#include "CWall.h"
#include "CSphere.h"
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
CSphere g_sphere[4];
CSphere g_target_blueball;

const XMVECTORF32 sphereColor[4] = {Colors::Red, Colors::Red, Colors::Yellow, Colors::White};

const float spherePos[4][2] = { { -2.7f, 0 }, { +2.4f, 0 }, { 3.3f, 0 }, { -2.7f, -0.9f } };
// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
bool Display(float timeDelta)
{
	int i = 0;
	int j = 0;
	if (device)
	{

		d3d::BeginScene();

		// update the position of each ball.during update, check whether each ball hit by walls.
		for (i = 0; i < 4; i++)
		{
			g_sphere[i].ballUpdate(timeDelta);
			for (j = 0; j < 4; j++)
			{
				g_legowall[i].hitBy(g_sphere[j]);
			}
		}


		//check whether any two balls hit together and update the direction of balls

		for (i = 0; i < 4; i++)
		{
			for (j = 0; j < 4; j++)
			{
				if (i >= j) continue;
				g_sphere[i].hitBy(g_sphere[j]);
			}
		}

		//draw plane, walls and spheres
		g_legoPlane.draw(g_mWorld, g_mView, g_mProj, wireMode);

		for (int i = 0; i < 4; i++)
		{
			g_legowall[i].draw(g_mWorld, g_mView, g_mProj, wireMode);
			g_sphere[i].draw(g_mWorld, g_mView, g_mProj, wireMode);
		}

		g_target_blueball.draw(g_mWorld, g_mView, g_mProj, wireMode);

		d3d::EndScene();
		return true;	
	}
	return false;


}
bool Setup(){
	int i;

	//setup model
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

	if (false == g_target_blueball.create(Colors::Blue)) return false;
	g_target_blueball.setCenter(.0f, float(M_RADIUS), .0f);

	for (i = 0; i < 4; i++)
	{
		if (false == g_sphere[i].create(sphereColor[i])) return false;
		g_sphere[i].setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
		g_sphere[i].setPower(0, 0);
	}

	//setup position and aim the camera
	
	XMFLOAT3 pos(0.f, 9.f, 7.f);
	XMFLOAT3 target(0.f, 0.f, 0.f);
	XMFLOAT3 up(0.f, -2.f, 0.f);
	

	g_mView = XMMatrixLookAtLH(XMLoadFloat3(&pos), XMLoadFloat3(&target), XMLoadFloat3(&up));
	g_mWorld = XMMatrixIdentity();
	g_mProj = XMMatrixPerspectiveFovLH(XM_PI / 4, (float)Width / (float)Height, 1.f, 100.f);


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
			
			auto targetpos = g_target_blueball.getCenter();
			auto whitepos = g_sphere[3].getCenter();
			double theta = acos(sqrt(pow(targetpos.x - whitepos.x, 2)) / sqrt(pow(targetpos.x - whitepos.x, 2) +
			pow(targetpos.z - whitepos.z, 2)));		// 기본 1 사분면
			if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x >= 0) { theta = -theta; }	//4 사분면
			if (targetpos.z - whitepos.z >= 0 && targetpos.x - whitepos.x <= 0) { theta = XM_PI - theta; } //2 사분면
			if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x <= 0){ theta = XM_PI + theta; } // 3 사분면
			double distance = sqrt(pow(targetpos.x - whitepos.x, 2) + pow(targetpos.z - whitepos.z, 2));
			g_sphere[3].setPower(distance * cos(theta), distance * sin(theta));
			
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
					dx *= -1;
					mX = XMMatrixRotationY(dx);
					mY = XMMatrixRotationX(dy);
					g_mWorld = g_mWorld * mX * mY;

					break;
				}
			}


			
		}
		else {
			
			isReset = true;

			if (LOWORD(wParam) & MK_RBUTTON) {
				dx = (old_x - new_x);// * 0.01f;
				dy = (old_y - new_y);// * 0.01f;

				dy *= -1;

				auto coord3d = g_target_blueball.getCenter();
				g_target_blueball.setCenter(coord3d.x + dx*(-0.007f), coord3d.y, coord3d.z + dy*0.007f);
			}
			old_x = new_x;
			old_y = new_y;

			move = WORLD_MOVE;
			
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