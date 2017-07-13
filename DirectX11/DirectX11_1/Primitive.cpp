#include "Primitive.h"
#include"DeviceDx11.h"
#include"Camera.h"

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

void 
Primitive::ApplyMatrixBuffer()
{
	DeviceDx11& dev = DeviceDx11::Instance();

	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mappedMatrixies);
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mappedMatrixies.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
	//*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違えるとメモリがぐちゃぐちゃになる
	dev.Context()->Unmap(_matrixBuffer, 0);
	return;
}

void
Primitive::UpdateMatrixies()
{
	XMMATRIX modelMatrix = XMMatrixIdentity();
	XMMATRIX transMatrix = XMMatrixTranslation(_pos.x, _pos.y , _pos.z);
	XMMATRIX scaleMat = XMMatrixScaling(_scale.x, _scale.y, _scale.z);
	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(_rot.x, _rot.y, _rot.z);

	modelMatrix = XMMatrixMultiply(transMatrix, XMMatrixMultiply(rotMat, scaleMat));


	_worldAndCamera.world = modelMatrix;
	_worldAndCamera.cameraView = _cameraPtr.lock()->CameraView();
	_worldAndCamera.cameraProj = _cameraPtr.lock()->CameraProjection();
}
