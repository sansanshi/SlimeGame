#include "DecalPlane.h"
#include<vector>
#include"ShaderGenerator.h"
#include"DeviceDx11.h"
#include"Camera.h"

DecalPlane::DecalPlane(Camera* camera) :_cameraPtr(camera)
{
};

DecalPlane::DecalPlane(float width, float depth, XMFLOAT3 normal, Camera* camera) :_cameraPtr(camera)
{


	DeviceDx11& dev = DeviceDx11::Instance();

	Vector3 o;
	o = XMFLOAT3(0, 0, 0);

	std::vector<DecalPlaneVertex> vertices(4);
	vertices[0] = { XMFLOAT3(-width / 2, 0, -depth / 2), XMFLOAT3(0, 1, 0), XMFLOAT2(0, 1) };
	vertices[1] = { XMFLOAT3(-width / 2, 0, depth / 2), XMFLOAT3(0, 1, 0), XMFLOAT2(0, 0)};
	vertices[2] = { XMFLOAT3(width / 2, 0, -depth / 2), XMFLOAT3(0, 1, 0), XMFLOAT2(1, 1)};
	vertices[3] = { XMFLOAT3(width / 2, 0, depth / 2), XMFLOAT3(0, 1, 0), XMFLOAT2(1, 0)};

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &vertices[0];

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = sizeof(DecalPlaneVertex)*vertices.size();
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = sizeof(DecalPlaneVertex);

	HRESULT result = S_OK;
	result = dev.Device()->CreateBuffer(&desc, &data, &_vertexBuffer);

	D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }

	};

	D3DX11CreateShaderResourceViewFromFile(dev.Device(), "texture/bloodhand.png", nullptr, nullptr, &_texture, &result);


	ShaderGenerator::CreateVertexShader("Decal.hlsl", "DecalPlaneVS", "vs_5_0",
		_vertexShader, inputElementDescs, sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC), _inputlayout);
	ShaderGenerator::CreatePixelShader("Decal.hlsl", "DecalPlanePS", "ps_5_0", _pixelShader);

	_modelMatrix = XMMatrixIdentity();
	_matrixies.world = _modelMatrix;
	_matrixies.view = _cameraPtr->CameraView();
	_matrixies.proj = _cameraPtr->CameraProjection();
	//_mvp.worldMatrix = _modelMatrix;//cameraのUpdateでカメラのworldMatrixを変えるようになったら2つを乗算する
	//_mvp.viewMatrix = _cameraPtr.lock()->GetMatrixies().view;
	//_mvp.projectionMatrix = _cameraPtr.lock()->GetMatrixies().projection;

	XMVECTOR dummy;
	XMMATRIX view = _cameraPtr->CameraView();
	_matrixies.invView = XMMatrixInverse(&dummy, view);
	XMMATRIX world = _matrixies.world;
	_matrixies.invWorld = XMMatrixInverse(&dummy, world);

	XMMATRIX proj = _cameraPtr->CameraProjection();
	XMMATRIX vp = XMMatrixMultiply(proj, view);
	XMMATRIX wvp = XMMatrixMultiply(vp, world);
	XMMATRIX invWVP = XMMatrixInverse(&dummy, wvp);
	_matrixies.invWVP = invWVP;
	_matrixies.wvp = wvp;

	_rot = 0.0f;

	//mvp行列用のバッファ作る
	//
	D3D11_BUFFER_DESC matBuffDesc = {};
	matBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matBuffDesc.ByteWidth = sizeof(WorldAndCamera);
	matBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;//バッファの中身はCPUで書き換える
	matBuffDesc.Usage = D3D11_USAGE_DYNAMIC;//CPUによる書き込み、GPUによる読み込みが行われるという意味
	//matBuffDesc.ByteWidth = sizeof(XMMATRIX);
	//matBuffDesc.StructureByteStride = sizeof(XMMATRIX);

	D3D11_SUBRESOURCE_DATA d;
	d.pSysMem = &_matrixies;

	result = dev.Device()->CreateBuffer(&matBuffDesc, &d, &_matrixBuffer);

	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mappedMatrixies);
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mappedMatrixies.pData, (void*)(&_matrixies), sizeof(_matrixies));
	//*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違えるとメモリがぐちゃぐちゃになる
	dev.Context()->Unmap(_matrixBuffer, 0);

	//デカールテクスチャセット
	dev.Context()->PSSetShaderResources(9, 1, &_texture);

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);
}


DecalPlane::~DecalPlane()
{
}

void
DecalPlane::Draw()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);

	dev.Context()->PSSetShaderResources(9, 1, &_texture);


	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mappedMatrixies);
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mappedMatrixies.pData, (void*)(&_matrixies), sizeof(_matrixies));
	//↑　*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違えるとメモリがぐちゃぐちゃになる
	dev.Context()->Unmap(_matrixBuffer, 0);


	dev.Context()->VSSetShader(_vertexShader, nullptr, 0);
	dev.Context()->IASetInputLayout(_inputlayout);
	dev.Context()->PSSetShader(_pixelShader, nullptr, 0);
	//プリミティブトポロジの切り替えを忘れない　切り替えを頻発させるのは良くない
	dev.Context()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	unsigned int stride = sizeof(float) * 8;
	unsigned int offset = 0;
	dev.Context()->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	dev.Context()->Draw(4, 0);
}

void
DecalPlane::Update()
{
	_rot = 0;// -1 * XM_PI / 180;
	XMMATRIX transMat = XMMatrixTranslation(_pos.x, _pos.y, _pos.z);
	XMMATRIX rotMatrix = XMMatrixIdentity();// XMMatrixRotationX(-90 * XM_PI / 180);
	_modelMatrix = rotMatrix;
	_modelMatrix = transMat;
	_modelMatrix = XMMatrixMultiply( rotMatrix,transMat);
	_matrixies.world = _modelMatrix;
	_matrixies.view = _cameraPtr->CameraView();
	_matrixies.proj = _cameraPtr->CameraProjection();

	XMVECTOR dummy;
	XMMATRIX view = _cameraPtr->CameraView();
	_matrixies.invView = XMMatrixInverse(&dummy, view);
	XMMATRIX world = _matrixies.world;
	_matrixies.invWorld = XMMatrixInverse(&dummy, world);

	XMMATRIX proj = _cameraPtr->CameraProjection();
	XMMATRIX vp = XMMatrixMultiply(proj, view);
	XMMATRIX wvp = XMMatrixMultiply(vp, world);
	XMMATRIX invWVP = XMMatrixInverse(&dummy, wvp);
	_matrixies.invWVP = invWVP;
	_matrixies.wvp = wvp;

	int kj = 0;
}

void
DecalPlane::SetPos(XMFLOAT3 pos)
{
	_pos = pos;
}