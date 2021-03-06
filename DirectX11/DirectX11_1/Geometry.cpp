#include<D3D11.h>
#include"Geometry.h"


XMFLOAT3 operator- (XMFLOAT3& lval, XMFLOAT3& rval)
{
	return XMFLOAT3(lval.x - rval.x, lval.y - rval.y, lval.z - rval.z);
}
XMFLOAT3 operator+ (XMFLOAT3& lval, XMFLOAT3& rval)
{
	return XMFLOAT3(lval.x + rval.x, lval.y + rval.y, lval.z + rval.z);
}
XMFLOAT3 operator* (XMFLOAT3& vec, float value)
{
	return XMFLOAT3(vec.x*value, vec.y*value, vec.z*value);
}
XMFLOAT3 operator/ (XMFLOAT3& lval, float div)
{
	return XMFLOAT3(lval.x / div, lval.y / div, lval.z / div);
}

float Length(XMFLOAT3& vec)
{
	return(sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z));
}

//����
float Dot(XMFLOAT3& lval, XMFLOAT3& rval)
{
	return lval.x*rval.x + lval.y*rval.y + lval.z*rval.z;

}
//�O��
XMFLOAT3 Cross(XMFLOAT3& lval, XMFLOAT3& rval)
{
	return XMFLOAT3(
		lval.y*rval.z - lval.z*rval.y,
		lval.z*rval.x - lval.x*rval.z,
		lval.x*rval.y - lval.y*rval.x);
}
//���K��
XMFLOAT3 Normalize(XMFLOAT3& vec)
{
	float len = Length(vec);
	return XMFLOAT3(vec.x / len, vec.y / len, vec.z / len);
}

bool operator== (XMFLOAT3 lval, XMFLOAT3 rval)
{
	return !(lval.x != rval.x || lval.y != rval.y || lval.z != rval.z);
}

XMFLOAT3 operator* (XMFLOAT3 lval, XMMATRIX& mat)
{
	XMVECTOR v = XMLoadFloat3(&lval);
	v = XMVector3Transform(v, mat);
	XMFLOAT3 ret;
	XMStoreFloat3(&ret, v);
	return ret;
}