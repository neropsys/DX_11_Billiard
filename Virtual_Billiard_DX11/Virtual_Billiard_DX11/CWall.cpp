#include "CWall.h"

using namespace DirectX;


CWall::CWall()
{

}

CWall::~CWall()
{

}

bool CWall::create(float iwidth, float iheight, float idepth)
{
	shape = GeometricPrimitive::CreateTeapot(d3d::deviceContext, 3.f, 8U, false);

	return false;
}

void CWall::draw(const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj, bool wireframe)
{
	shape->Draw(world, view, proj, Colors::AliceBlue, nullptr, wireframe);
}

