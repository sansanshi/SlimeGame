#include "Primitive.h"


Primitive::Primitive()
{
}


Primitive::~Primitive()
{
}

void
Primitive::SetPos(const XMFLOAT3 pos)
{
	_pos = pos;
}
XMFLOAT3
Primitive::GetPos()
{
	return _pos;
}

void
Primitive::SetRotate(const XMFLOAT3 yawPitchRoll)
{
	_rot = yawPitchRoll;
}
XMFLOAT3
Primitive::GetRotate()
{
	return _rot;
}

void
Primitive::SetScale(const XMFLOAT3 scale)
{
	_scale = scale;
}
XMFLOAT3
Primitive::GetScale()
{
	return _scale;
}

void 
Primitive::InitTransform()
{
	_pos = XMFLOAT3(0, 0, 0);
	_scale = XMFLOAT3(1, 1, 1);
	_rot = XMFLOAT3(0, 0, 0);
}