/*!
* \file CWall.cpp
* \date 2016/01
*
* \author Hyuk-jae Chang, ÀåÇõÀç
* Contact: neropsys@gmail.com
*/
#include "CWall.h"

using namespace DirectX;

CWall::CWall()
{
	m_mLocal = XMMatrixIdentity();
	ZeroMemory(&m_color, sizeof(m_color));
	m_width = 0;
	m_depth = 0;
	m_pBoundMesh = nullptr;
}

CWall::~CWall()
{
}

bool CWall::create(float iwidth, float iheight, float idepth, const DirectX::XMVECTORF32& color)
{
	if (d3d::deviceContext == nullptr)
		return false;

	m_width = iwidth;
	m_depth = idepth;
	m_color = color;

	m_pBoundMesh = GeometricPrimitive::CreateBox(d3d::deviceContext, XMFLOAT3(iwidth, iheight, idepth), false, true);

	return true;
}

void CWall::setPosition(float x, float y, float z)
{
	XMMATRIX m;
	this->m_x = x;
	this->m_z = z;
	m = XMMatrixTranslation(x, y, z);
	setLocalTransform(m);

}

void CWall::draw(const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj, bool wireframe)
{
	m_pBoundMesh->Draw(m_mLocal * world, view, proj, m_color, nullptr, wireframe);
}

void CWall::destroy()
{
	if (m_pBoundMesh != nullptr){
		m_pBoundMesh.reset();
	}
}

void CWall::hitBy(CSphere& ball)
{
	// Insert your code here.
}
bool CWall::hasIntersected(CSphere& ball)
{
	//Insert your code here.
	return false;
}
