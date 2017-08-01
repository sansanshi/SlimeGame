#include "Primitive.h"
#include"DeviceDx11.h"
#include"Camera.h"
#include"ShaderDefine.h"

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
	float calcRad = XM_PI / 180.0f;
	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(_rot.x*calcRad, _rot.y*calcRad, _rot.z*calcRad);

	modelMatrix = XMMatrixMultiply(XMMatrixMultiply( scaleMat,rotMat),transMatrix);


	_worldAndCamera.world = modelMatrix;
	_worldAndCamera.cameraView = _cameraPtr.lock()->CameraView();
	_worldAndCamera.cameraProj = _cameraPtr.lock()->CameraProjection();
}

void
Primitive::ApplyCameraShaders()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	if (std::shared_ptr<ID3D11VertexShader*> r = _vertexShader.lock())
	{
		dev.Context()->VSSetShader(*_vertexShader.lock(), nullptr, 0);
	}
	if (std::shared_ptr<ID3D11InputLayout*> r = _inputlayout.lock())
	{
		dev.Context()->IASetInputLayout(*_inputlayout.lock());
	}
	if (std::shared_ptr<ID3D11PixelShader*> r = _pixelShader.lock())
	{
		dev.Context()->PSSetShader(*_pixelShader.lock(), nullptr, 0);
	}
	if (std::shared_ptr<ID3D11HullShader*> r = _hullShader.lock())
	{
		dev.Context()->HSSetShader(*_hullShader.lock(), nullptr, 0);
	}
	if (std::shared_ptr<ID3D11DomainShader*> r = _domainShader.lock())
	{
		dev.Context()->DSSetShader(*_domainShader.lock(), nullptr, 0);
	}
}

void
Primitive::ApplyDepthShaders()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	if (std::shared_ptr<ID3D11VertexShader*> r = _lightviewVS.lock())
	{
		dev.Context()->VSSetShader(*_lightviewVS.lock(), nullptr, 0);
	}
	if (std::shared_ptr<ID3D11InputLayout*> r = _lightviewInputLayout.lock())
	{
		dev.Context()->IASetInputLayout(*_lightviewInputLayout.lock());
	}
	if (std::shared_ptr<ID3D11PixelShader*> r = _lightviewPS.lock())
	{
		dev.Context()->PSSetShader(*_lightviewPS.lock(), nullptr, 0);
	}
	if (std::shared_ptr<ID3D11HullShader*> r = _hullShader.lock())
	{
		dev.Context()->HSSetShader(*_hullShader.lock(), nullptr, 0);
	}
	if (std::shared_ptr<ID3D11DomainShader*> r = _domainShader.lock())
	{
		dev.Context()->DSSetShader(*_domainShader.lock(), nullptr, 0);
	}
}

void
Primitive::ApplyTextures()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	if (std::shared_ptr<ID3D11ShaderResourceView*> t = _mainTex.lock())
	{
		dev.Context()->PSSetShaderResources(TEXTURE_MAIN, 1, t._Get());
	}
	if (std::shared_ptr<ID3D11ShaderResourceView*> t = _subTex.lock())
	{
		dev.Context()->PSSetShaderResources(TEXTURE_SUB, 1, t._Get());
	}
	if (std::shared_ptr<ID3D11ShaderResourceView*> t = _normalTex.lock())
	{
		dev.Context()->PSSetShaderResources(TEXTURE_NORMAL, 1, t._Get());
	}
	if (std::shared_ptr<ID3D11ShaderResourceView*> t = _flowTex.lock())
	{
		dev.Context()->PSSetShaderResources(TEXTURE_FLOW, 1, t._Get());
	}
	if (std::shared_ptr<ID3D11ShaderResourceView*> t = _dispMask.lock())
	{
		dev.Context()->PSSetShaderResources(TEXTURE_MASK, 1, t._Get());
		dev.Context()->VSSetShaderResources(TEXTURE_MASK, 1, t._Get());
	}
	if (std::shared_ptr<ID3D11ShaderResourceView*> t = _displacementTex.lock())
	{
		dev.Context()->PSSetShaderResources(TEXTURE_DISPLACEMENT, 1, t._Get());
		dev.Context()->VSSetShaderResources(TEXTURE_DISPLACEMENT, 1, t._Get());
	}
	if (std::shared_ptr<ID3D11ShaderResourceView*> t = _heightMap.lock())
	{
		dev.Context()->PSSetShaderResources(TEXTURE_HEIGHT, 1, t._Get());
	}
	if (std::shared_ptr<ID3D11ShaderResourceView*> t = _subTex2.lock())
	{
		dev.Context()->PSSetShaderResources(TEXTURE_SUB2, 1, t._Get());
	}
}

void
Primitive::Rotate(const XMFLOAT3 pyr)
{
	_rot.x += pyr.x;
	_rot.y += pyr.y;
	_rot.z += pyr.z;
}