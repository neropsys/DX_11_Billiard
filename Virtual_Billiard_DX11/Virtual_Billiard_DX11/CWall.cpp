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
	shape = GeometricPrimitive::CreateTeapot(d3d::deviceContext);

	return false;
}

void CWall::draw(const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj)
{
	shape->Draw(world, view, proj, Colors::AliceBlue);
}

