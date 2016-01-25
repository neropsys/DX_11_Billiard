/*!
* \file CSphere.cpp
* \date 2016/01
*
* \author Hyuk-jae Chang, ÀåÇõÀç
* Contact: neropsys@gmail.com
*/

#include "CSphere.h"
using namespace DirectX;

CSphere::CSphere()
{
	m_mLocal = XMMatrixIdentity();
	ZeroMemory(&m_color, sizeof(m_color));
	m_radius = 0;
	m_velocity_x = 0;
	m_velocity_z = 0;
	m_pBoundMesh = nullptr;
}

CSphere::~CSphere(){}

bool CSphere::create(const DirectX::XMVECTORF32& color)
{
	if (d3d::deviceContext == nullptr)
		return false;
	m_color = color;
	
	m_pBoundMesh = GeometricPrimitive::CreateSphere(d3d::deviceContext, getRadius() * 2, 16U, false, true);

	return true;
}

void CSphere::destroy()
{
	if (m_pBoundMesh != nullptr){
		m_pBoundMesh.reset();
	}
}

void CSphere::draw(const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj, bool wireframe)
{
	m_pBoundMesh->Draw(m_mLocal * world, view, proj, m_color, nullptr, wireframe);
}

bool CSphere::hasIntersected(CSphere& ball)
{
	// Insert your code here.
	return false;
}

void CSphere::hitBy(CSphere& ball)
{
	// Insert your code here.
}

void CSphere::ballUpdate(float timeDiff)
{
	const float TIME_SCALE = 3.3;
	XMFLOAT3 cord = this->getCenter();
	double vx = abs(this->getVelocity_X());
	double vz = abs(this->getVelocity_Z());

	if (vx > 0.01 || vz > 0.01)
	{
		float tX = cord.x + TIME_SCALE*timeDiff*m_velocity_x;
		float tZ = cord.z + TIME_SCALE*timeDiff*m_velocity_z;

		//correction of position of ball
		// Please uncomment this part because this correction of ball position is necessary when a ball collides with a wall
		if(tX >= (4.5 - M_RADIUS))
		tX = 4.5 - M_RADIUS;
		else if(tX <=(-4.5 + M_RADIUS))
		tX = -4.5 + M_RADIUS;
		else if(tZ <= (-3 + M_RADIUS))
		tZ = -3 + M_RADIUS;
		else if(tZ >= (3 - M_RADIUS))
		tZ = 3 - M_RADIUS;
		
		this->setCenter(tX, cord.y, tZ);
	}
	else { this->setPower(0, 0); }
	double rate = 1 - (1 - DECREASE_RATE)*timeDiff * 400;
	if (rate < 0)
		rate = 0;
	this->setPower(getVelocity_X() * rate, getVelocity_Z() * rate);
}

void CSphere::setPower(double vx, double vz)
{
	this->m_velocity_x = vx;
	this->m_velocity_z = vz;
}

void CSphere::setCenter(float x, float y, float z)
{
	XMMATRIX m;
	center_x = x;
	center_y = y;
	center_z = z;
	m = XMMatrixTranslation(x, z, y);
	setLocalTransform(m);
}

DirectX::XMFLOAT3 CSphere::getCenter() const
{
	XMFLOAT3 org(center_x, center_y, center_z);
	return org;
}
