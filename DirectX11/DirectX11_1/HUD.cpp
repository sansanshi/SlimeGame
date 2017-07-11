#include "HUD.h"
#include"DeviceDx11.h"
#include<vector>
#include"ShaderGenerator.h"
#include"Camera.h"

HUD::HUD(const std::shared_ptr<Camera>& cam,const float top,const float left,const float width,const float height):_cameraPtr(cam)
{
	DeviceDx11& dev = DeviceDx11::Instance();
	_vs = nullptr;
	_inputLayout = nullptr;
	_ps = nullptr;
	CreateHUDShader(_vs, _inputLayout, _ps);


	_vertexBuffer = CreateHUDVertexBuffer(top, left, width, height);

	unsigned int hudstride = sizeof(HUDVertex);
	unsigned int hudoffset = 0;

	//mvp行列用のバッファ作る HUD用に作っただけなので後でクラス化したら消す
	D3D11_BUFFER_DESC matBuffDesc = {};
	matBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matBuffDesc.ByteWidth = sizeof(WorldAndCamera);
	matBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;//バッファの中身はCPUで書き換える
	matBuffDesc.Usage = D3D11_USAGE_DYNAMIC;//CPUによる書き込み、GPUによる読み込みが行われるという意味
											//matBuffDesc.ByteWidth = sizeof(XMMATRIX);
											//matBuffDesc.StructureByteStride = sizeof(XMMATRIX);

	_hudMatrix = CreateHUDMatrix(WINDOW_WIDTH, WINDOW_HEIGHT,0,0);
	_hudMatrixIdentity = _hudMatrix;

	_worldAndCamera.world = _hudMatrix;
	_worldAndCamera.cameraView = _cameraPtr.lock()->CameraView();
	_worldAndCamera.cameraProj = _cameraPtr.lock()->CameraProjection();
	D3D11_SUBRESOURCE_DATA d;
	d.pSysMem = &_worldAndCamera;

	HRESULT result = dev.Device()->CreateBuffer(&matBuffDesc, &d, &_matrixBuffer);

	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
	//*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違
	dev.Context()->Unmap(_matrixBuffer, 0);

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);

	D3DX11CreateShaderResourceViewFromFile(dev.Device(), "marker.dds", nullptr, nullptr, &_texture, &result);
}


HUD::~HUD()
{
}


ID3D11Buffer*
HUD::CreateHUDVertexBuffer(float top, float left, float width, float height)
{
	DeviceDx11& dev = DeviceDx11::Instance();

	std::vector<HUDVertex> vertices(4);
	{
		vertices[0] = { XMFLOAT3(left, top, 0.1f), XMFLOAT2(0.f, 0.f) };//左上
		vertices[1] = { XMFLOAT3(left + width, top, 0.1f), XMFLOAT2(1.0f, 0.0f) };//右上
		vertices[2] = { XMFLOAT3(left, top + height, 0.1f), XMFLOAT2(0.f, 1.f) };//左下
		vertices[3] = { XMFLOAT3(left + width, top + height, 0.1f), XMFLOAT2(1.f, 1.f) };//右下
	};
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

	/*XMMATRIX hudMatrix = CreateHUDMatrix(WINDOW_WIDTH, WINDOW_HEIGHT);

	XMFLOAT3 pa[4] = {
		vertices[0].pos,
		vertices[1].pos,
		vertices[2].pos,
		vertices[3].pos
	};

	for (auto& p : pa)
	{
		XMVECTOR pos = XMLoadFloat3(&p);
		XMMATRIX mat = hudMatrix;
		XMVECTOR v = XMVector3Transform(pos, mat);
		XMFLOAT3 r;
		XMStoreFloat3(&r, v);
		int b = 0;
	}*/

	return vertexBuffer;

}

HRESULT
HUD::CreateHUDShader(
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
		"HUDVS",
		"vs_5_0",
		vs,
		inputElementDescs,
		sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		layout);

	result = ShaderGenerator::CreatePixelShader(
		"BaseShader.hlsl",
		"HUDPS",
		"ps_5_0",
		ps);

	return result;
}

XMMATRIX
HUD::CreateHUDMatrix(float width, float height, float offsetx = 0, float offsety = 0)
{
	XMMATRIX HUDMat = XMMatrixIdentity();
	XMMATRIX m0 = XMMatrixScaling(2.0f / width, -2.0f / height, 1);
	XMMATRIX m1 = XMMatrixTranslation(-1, 1, 0);
	HUDMat = XMMatrixMultiply(m0, m1);

	HUDMat._41 = -1.0f + offsetx*2.0f / WINDOW_WIDTH;
	HUDMat._42 = 1.0f + offsety*2.0f / WINDOW_HEIGHT;

	/*HUDMat._11 = 2.0f / (float)width;
	HUDMat._22 = 2.0f / (float)height;
	HUDMat._41 = -1;
	HUDMat._42 = 1;*/
	return HUDMat;
}
void
HUD::Update()
{
	//world書き換え（hudMatrixに）
	_worldAndCamera.world = _hudMatrix;// XMMatrixIdentity();
	_worldAndCamera.cameraView = _cameraPtr.lock()->CameraView();
	_worldAndCamera.cameraProj = _cameraPtr.lock()->CameraProjection();

	
}

void
HUD::Draw()
{
	DeviceDx11& dev = DeviceDx11::Instance();

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);
	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
	//*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違
	dev.Context()->Unmap(_matrixBuffer, 0);

	dev.Context()->VSSetShader(_vs, nullptr, 0);
	dev.Context()->PSSetShader(_ps, nullptr, 0);
	dev.Context()->IASetInputLayout(_inputLayout);
	unsigned int hudstride = sizeof(HUDVertex);
	unsigned int hudoffset = 0;
	dev.Context()->IASetVertexBuffers(0, 1, &_vertexBuffer, &hudstride, &hudoffset);

	dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	dev.Context()->Draw(4, 0);

}

void
HUD::Offset(const float x,const float y)
{
	XMMATRIX screenOfsMatrix = XMMatrixTranslation(x, y, 0);

	XMMATRIX h = _hudMatrixIdentity;
	XMMATRIX m = XMMatrixMultiply(screenOfsMatrix, h);
	_hudMatrix = m;
}