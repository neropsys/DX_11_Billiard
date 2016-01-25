/*!
* \file CSphere.h
* \date 2016/01
*
* \author Hyuk-jae Chang, ¿Â«ı¿Á
* Contact: neropsys@gmail.com
*/

#pragma once
#include "GeometricPrimitive.h"
#include "d3dUtility.h"
#include <DirectXMath.h>
#include <memory>

#define M_RADIUS 0.21
#define DECREASE_RATE 0.9982
class CSphere{
public:
	CSphere();
	~CSphere();

	bool create(const DirectX::XMVECTORF32& color);
	
	void destroy();
	
	void draw(const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj, bool wireframe);
	
	bool hasIntersected(CSphere& ball);
	
	void hitBy(CSphere& ball);

	void ballUpdate(float timeDiff);

	inline double getVelocity_X(){ return this->m_velocity_x; }
	inline double getVelocity_Z(){ return this->m_velocity_z; }
	
	void setPower(double vx, double vz);
	
	void setCenter(float x, float y, float z);

	inline float getRadius() const { return (float)(M_RADIUS); }

	DirectX::XMFLOAT3 getCenter() const;

	inline void setLocalTransform(const DirectX::XMMATRIX& mLocal) { m_mLocal = mLocal; }
	


private:

	std::unique_ptr<DirectX::GeometricPrimitive> m_pBoundMesh;

	DirectX::XMVECTORF32 m_color;

	DirectX::XMMATRIX m_mLocal;

	float center_x, center_y, center_z;
	float m_radius;
	float m_velocity_x;
	float m_velocity_z;

	

};