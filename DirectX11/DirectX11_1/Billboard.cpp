#include "Billboard.h"
#include "DeviceDx11.h"
#include<vector>
#include"Camera.h"
#include"ShaderGenerator.h"
#include"ShaderDefine.h"


Billboard::Billboard(const std::shared_ptr<Camera>& cam, float width, float height) :_cameraPtr(cam)
{
	DeviceDx11& dev = DeviceDx11::Instance();
	HRESULT result = S_OK;

	
	//ビルボードテスト
	_vertexBuffer = nullptr;
	_vertexBuffer = CreateBillBoardVertexBuffer(width, height);
	_texture = nullptr;
	D3DX11CreateShaderResourceViewFromFile(dev.Device(), "uvCheck_transparent.png", nullptr, nullptr, &_texture, &result);
	_vs = nullptr;
	_ps = nullptr;
	_inputLayout = nullptr;
	result=CreateBillBoardShader(_vs, _inputLayout, _ps);


	_worldAndCamera.world = XMMatrixIdentity();
	_worldAndCamera.cameraView = _cameraPtr.lock()->CameraView();
	_worldAndCamera.cameraProj = _cameraPtr.lock()->CameraProjection();

	D3D11_BUFFER_DESC matBuffDesc = {};
	matBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matBuffDesc.ByteWidth = sizeof(WorldAndCamera);
	matBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;//バッファの中身はCPUで書き換える
	matBuffDesc.Usage = D3D11_USAGE_DYNAMIC;//CPUによる書き込み、GPUによる読み込みが行われるという意味
											//matBuffDesc.ByteWidth = sizeof(XMMATRIX);
											//matBuffDesc.StructureByteStride = sizeof(XMMATRIX);

	D3D11_SUBRESOURCE_DATA d;
	d.pSysMem = &_worldAndCamera;

	result = dev.Device()->CreateBuffer(&matBuffDesc, &d, &_matrixBuffer);

	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
	//*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違
	dev.Context()->Unmap(_matrixBuffer, 0);

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);


}


Billboard::~Billboard()
{

}


void 
Billboard::Draw()
{
	DeviceDx11& dev = DeviceDx11::Instance();


	//XMMATRIX view = _cameraPtr.lock()->CameraView();
	//XMFLOAT3 trans = { view._41, view._42, view._43 };
	//view._41 = view._42 = view._43 = 0;
	//XMMATRIX invView = XMMatrixTranspose(view);
	//XMMATRIX proj = _cameraPtr.lock()->CameraProjection();
	////そのままカメラ回転逆行列を掛けるとワールド平行移動が影響を受けてしまうため
	////一時的にワールドの平行移動を無効にして、カメラ逆回転を掛けた後で平行移動成分を元に戻す
	////Wr * [Wt * Vr^-1] * Vr * Vt * proj
	////　　　　　↑ここが問題　ワールド平行移動の後に回転行列が掛けられるので
	////						　　平行移動後の座標を基準に回転されてしまう
	//XMFLOAT3 worldTrans = { _modelMatrix._41, _modelMatrix._42, _modelMatrix._43 };
	//_modelMatrix._41 = _modelMatrix._42 = _modelMatrix._43 = 0.0f;
	//XMMATRIX temp = XMMatrixMultiply(_modelMatrix, invView);
	//temp._41 = worldTrans.x; temp._42 = worldTrans.y; temp._43 = worldTrans.z;
	//_modelMatrix= temp;
	//XMMATRIX v = _cameraPtr.lock()->CameraView();
	//XMMATRIX p = _cameraPtr.lock()->CameraProjection();

	

	dev.Context()->VSSetShader(_vs, nullptr, 0);
	dev.Context()->PSSetShader(_ps, nullptr, 0);
	dev.Context()->IASetInputLayout(_inputLayout);

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);

	dev.Context()->PSSetShaderResources(TEXTURE_MAIN, 1, &_texture);
	unsigned int hudstride = sizeof(HUDVertex);
	unsigned int hudoffset = 0;
	dev.Context()->IASetVertexBuffers(0, 1, &_vertexBuffer, &hudstride, &hudoffset);

	dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	dev.Context()->Draw(4, 0);
}

void
Billboard::Update()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	//ビルボード表示

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);
	//world書き換え（hudMatrixに）
	_worldAndCamera.world = XMMatrixTranslation(-10, 15, 10);

	XMMATRIX w = _worldAndCamera.world;
	XMMATRIX view = _cameraPtr.lock()->CameraView();
	XMFLOAT3 trans = { view._41, view._42, view._43 };
	view._41 = view._42 = view._43 = 0;
	XMMATRIX invView = XMMatrixTranspose(view);
	//invView._41 = trans.x; invView._42 = trans.y; invView._43 = trans.z;
	XMMATRIX proj = _cameraPtr.lock()->CameraProjection();
	//そのままカメラ回転逆行列を掛けるとワールド平行移動が影響を受けてしまうため
	//一時的にワールドの平行移動を無効にして、カメラ逆回転を掛けた後で平行移動成分を元に戻す
	//Wr * [Wt * Vr^-1] * Vr * Vt * proj
	//　　　　　↑ここが問題　ワールド平行移動の後に回転行列が掛けられるので
	//						　　平行移動後の座標を基準に回転されてしまう
	XMFLOAT3 worldTrans = { w._41, w._42, w._43 };
	w._41 = w._42 = w._43 = 0.0f;
	XMMATRIX temp = XMMatrixMultiply(w, invView);
	temp._41 = worldTrans.x; temp._42 = worldTrans.y; temp._43 = worldTrans.z;
	_worldAndCamera.world = temp;
	_worldAndCamera.cameraView = _cameraPtr.lock()->CameraView();
	_worldAndCamera.cameraProj = _cameraPtr.lock()->CameraProjection();

	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
	//*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違
	dev.Context()->Unmap(_matrixBuffer, 0);

	


}

void
Billboard::SetPos(XMFLOAT3& pos)
{
	_pos = pos;
}

//ビルボードの頂点バッファ
//@param ビルボード中心X座標(3D座標系
//@param　ビルボード中心Y座標
//@param ビルボード幅（3D座標系
//@param ビルボード高さ(3D座標系
//@param note 幅と高さは中心からの広がり 左上はx-width/2,y+height/2
ID3D11Buffer*
Billboard::CreateBillBoardVertexBuffer(float width, float height)
{
	DeviceDx11& dev = DeviceDx11::Instance();

	std::vector<HUDVertex> vertices(4);
	{
		vertices[0] = { XMFLOAT3(-width / 2, height / 2, 0), XMFLOAT2(0.f, 0.f) };//左上
		vertices[1] = { XMFLOAT3(width / 2 , height / 2 , 0), XMFLOAT2(1.0f, 0.0f) };//右上
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
	desc.ByteWidth = sizeof(HUDVertex) * 4;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = sizeof(HUDVertex);

	HRESULT result = S_OK;
	ID3D11Buffer* vertexBuffer = nullptr;
	result = dev.Device()->CreateBuffer(&desc, &data, &vertexBuffer);


	XMFLOAT3 pa[4] = {
		vertices[0].pos,
		vertices[1].pos,
		vertices[2].pos,
		vertices[3].pos
	};


	return vertexBuffer;

}


HRESULT
Billboard::CreateBillBoardShader(
	ID3D11VertexShader*& vs,
	ID3D11InputLayout*& layout,
	ID3D11PixelShader*& ps)
{
	HRESULT result;
	D3D11_INPUT_ELEMENT_DESC inputElementDescs[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	int hh = sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	result = ShaderGenerator::CreateVertexShader(
		"BaseShader.hlsl",
		"BillBoardVS",
		"vs_5_0",
		vs,
		inputElementDescs,
		sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		layout);

	result = ShaderGenerator::CreatePixelShader(
		"BaseShader.hlsl",
		"BillBoardPS",
		"ps_5_0",
		ps);

	return result;
}