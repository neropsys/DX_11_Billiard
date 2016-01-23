#include "CWall.h"

using namespace DirectX;


CWall::CWall()
{

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

	m_pBoundMesh = GeometricPrimitive::CreateBox(d3d::deviceContext, XMFLOAT3(iwidth, idepth, iheight), false, true);

	return true;
}

void CWall::setPosition(float x, float y, float z)
{
	XMMATRIX m;
	this->m_x = x;
	this->m_z = z;
	m = XMMatrixTranslation(x, z, y);
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

