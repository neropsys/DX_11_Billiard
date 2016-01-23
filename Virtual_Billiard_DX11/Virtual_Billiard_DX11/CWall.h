
#pragma once
#include "GeometricPrimitive.h"
#include "d3dUtility.h"
#include <DirectXMath.h>
#include <memory>


class CWall{
public:
	CWall();
	~CWall();
	
	bool create(float iwidth,
				float iheight,
				float idepth,
				const DirectX::XMVECTORF32& color = DirectX::Colors::White);

	void setPosition(float x, float y, float z);

	void draw(const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj, bool wireframe);

	void destroy();

private:

	inline void setLocalTransform(const DirectX::XMMATRIX& mLocal){ m_mLocal = mLocal; }

	std::unique_ptr<DirectX::GeometricPrimitive> m_pBoundMesh;

	DirectX::XMVECTORF32 m_color;

	DirectX::XMMATRIX m_mLocal;

	float					m_x;
	float					m_z;
	float                   m_width;
	float                   m_depth;
	float					m_height;

};