#include "Billboard.h"
#include "DeviceDx11.h"
#include<vector>
#include"Camera.h"


Billboard::Billboard(Camera& cam, float width, float height) :_camera(cam)
{
	DeviceDx11& dev = DeviceDx11::Instance();
	HRESULT result = S_OK;

	std::vector<BillBoardVertex> vertices(4);
	{
		vertices[0] = { XMFLOAT3(-width / 2, height / 2, 0), XMFLOAT2(0.f, 0.f) };//左上
		vertices[1] = { XMFLOAT3(width / 2, height / 2, 0), XMFLOAT2(1.0f, 0.0f) };//右上
		vertices[2] = { XMFLOAT3(-width / 2, -height / 2, 0), XMFLOAT2(0.f, 1.f) };//左下
		vertices[3] = { XMFLOAT3(width / 2, -height / 2, 0), XMFLOAT2(1.f, 1.f) };//右下
	};
	int idxX;
	int idxY;
	idxX / 10.0f;
	idxX + 1 / 10.0f;
	idxY / 3.0f;
	idxY + 1 / 3.0f;
	//idxX = (frame / 4) % 10;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &vertices[0];

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = sizeof(BillBoardVertex) * 4;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = sizeof(BillBoardVertex);

	_vertexBuffer = nullptr;
	result = dev.Device()->CreateBuffer(&desc, &data, &_vertexBuffer);


	D3D11_BUFFER_DESC matrixiesCBDesc = {};
	matrixiesCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixiesCBDesc.ByteWidth = sizeof(XMMATRIX) * 2;
	matrixiesCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixiesCBDesc.MiscFlags = 0;
	matrixiesCBDesc.Usage = D3D11_USAGE_DYNAMIC;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = nullptr;

	dev.Device()->CreateBuffer(&matrixiesCBDesc, &initData, &_wvpBuffer);

}


Billboard::~Billboard()
{

}


void 
Billboard::Draw()
{
	DeviceDx11& dev = DeviceDx11::Instance();


	XMMATRIX view = _camera.CameraView();
	XMFLOAT3 trans = { view._41, view._42, view._43 };
	view._41 = view._42 = view._43 = 0;
	XMMATRIX invView = XMMatrixTranspose(view);
	XMMATRIX proj = _camera.CameraProjection();
	//そのままカメラ回転逆行列を掛けるとワールド平行移動が影響を受けてしまうため
	//一時的にワールドの平行移動を無効にして、カメラ逆回転を掛けた後で平行移動成分を元に戻す
	//Wr * [Wt * Vr^-1] * Vr * Vt * proj
	//　　　　　↑ここが問題　ワールド平行移動の後に回転行列が掛けられるので
	//						　　平行移動後の座標を基準に回転されてしまう
	XMFLOAT3 worldTrans = { _modelMatrix._41, _modelMatrix._42, _modelMatrix._43 };
	_modelMatrix._41 = _modelMatrix._42 = _modelMatrix._43 = 0.0f;
	XMMATRIX temp = XMMatrixMultiply(_modelMatrix, invView);
	temp._41 = worldTrans.x; temp._42 = worldTrans.y; temp._43 = worldTrans.z;
	_modelMatrix= temp;
	XMMATRIX v = _camera.CameraView();
	XMMATRIX p = _camera.CameraProjection();

	

}

void
Billboard::Update()
{

}

void
Billboard::SetPos(XMFLOAT3& pos)
{
	_pos = pos;
}