#pragma once
#include"Define.h"

struct PrimitiveVertex{
	Vector3 pos;
	Vector3 normal;
	Vector2 uv;
	Vector3 tangent;
	Vector3 binormal;
	PrimitiveVertex()
		:pos(Vector3(0,0,0)),normal(Vector3(0,0,0)),uv(Vector2(0,0)),tangent(Vector3(0,0,0)),binormal(Vector3(0,0,0))
	{
	}

	PrimitiveVertex(const Vector3& p, const Vector3& norm,const Vector2& coord,Vector3& tang,Vector3& binorm)
		:pos(p), normal(norm), uv(coord),tangent(tang),binormal(binorm){
	}

	//Å´ÇﬂÇÒÇ«Ç≠Ç≥Ç¢ÇµëΩï™égÇÌÇ»Ç¢ÇÃÇ≈ï˙íu
	/*PrimitiveVertex(float x, float y, float z, float nx, float ny, float nz, float u, float v)
		:pos(x, y, z), normal(nx, ny, nz), uv(u, v){
	}*/

};

class Primitive
{
public:
	Primitive();
	~Primitive();
};

