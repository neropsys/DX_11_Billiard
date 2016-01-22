
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
				float idepth);
	void draw(const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj);
private:
	std::unique_ptr<DirectX::GeometricPrimitive> shape;

};