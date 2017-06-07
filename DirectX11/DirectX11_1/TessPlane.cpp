#include "TessPlane.h"


#include<vector>
#include"ShaderGenerator.h"
#include"DeviceDx11.h"

TessPlane::TessPlane(Camera& camera) :_cameraRef(camera)
{
};

TessPlane::TessPlane(float width, float depth, Vector3 normal, Camera& camera) :_cameraRef(camera)
{


	DeviceDx11& dev = DeviceDx11::Instance();

	Vector3 o;
	o = XMFLOAT3(0, 0, 0);

	std::vector<PrimitiveVertex> vertices(4);
	vertices[0] = { Vector3(-width / 2, 0, -depth / 2), Vector3(0, 1, 0), Vector2(0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0) };
	vertices[1] = { Vector3(-width / 2, 0, depth / 2), Vector3(0, 1, 0), Vector2(1, 0), Vector3(0, 0, 0), Vector3(0, 0, 0) };
	vertices[3] = { Vector3(width / 2, 0, -depth / 2), Vector3(0, 1, 0), Vector2(0, 1), Vector3(0, 0, 0), Vector3(0, 0, 0) };//コントロールポイントの順番がよく分からん
	vertices[2] = { Vector3(width / 2, 0, depth / 2), Vector3(0, 1, 0), Vector2(1, 1), Vector3(0, 0, 0), Vector3(0, 0, 0) };//とりあえずN字じゃ変になるっぽいので変更

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &vertices[0];

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = sizeof(PrimitiveVertex)*vertices.size();
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = sizeof(PrimitiveVertex);

	HRESULT result = S_OK;
	result = dev.Device()->CreateBuffer(&desc, &data, &_vertexBuffer);

	D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }

	};

	D3D11_INPUT_ELEMENT_DESC lightViewInputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ShaderGenerator::CreateVertexShader("Tessellation.hlsl", "TessVS", "vs_5_0",
		_vertexShader, inputElementDescs, sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC), _inputlayout);
	ShaderGenerator::CreatePixelShader("Tessellation.hlsl", "TessPS", "ps_5_0", _pixelShader);

	//ハルシェーダ
	ShaderGenerator::CreateHullShader("Tessellation.hlsl", "TessHS", "hs_5_0", _hullShader);
	//ドメインシェーダ
	ShaderGenerator::CreateDomainShader("Tessellation.hlsl", "TessDS", "ds_5_0", _domainShader);

	ShaderGenerator::CreateVertexShader("lightview.hlsl", "PrimitiveLightViewVS", "vs_5_0",
		_lightviewVS, inputElementDescs, sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC), _lightviewInputLayout);
	ShaderGenerator::CreatePixelShader("lightview.hlsl", "PrimitiveLightViewPS", "ps_5_0", _lightviewPS);

	_modelMatrix = XMMatrixIdentity();
	_worldAndCamera.world = _modelMatrix;
	_worldAndCamera.cameraView = _cameraRef.CameraView();
	_worldAndCamera.cameraProj = _cameraRef.CameraProjection();
	_worldAndCamera.lightView = _cameraRef.LightView();
	_worldAndCamera.lightProj = _cameraRef.LightProjection();
	//_mvp.worldMatrix = _modelMatrix;//cameraのUpdateでカメラのworldMatrixを変えるようになったら2つを乗算する
	//_mvp.viewMatrix = _cameraRef.GetMatrixies().view;
	//_mvp.projectionMatrix = _cameraRef.GetMatrixies().projection;

	rot = 0.0f;

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
	d.pSysMem = &_worldAndCamera;

	result = dev.Device()->CreateBuffer(&matBuffDesc, &d, &_matrixBuffer);

	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mappedMatrixies);
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mappedMatrixies.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
	//*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違えるとメモリがぐちゃぐちゃになる
	dev.Context()->Unmap(_matrixBuffer, 0);

	dev.Context()->DSSetConstantBuffers(0, 1, &_matrixBuffer);

	//サンプラの設定
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	dev.Device()->CreateSamplerState(&samplerDesc, &_samplerState_Wrap);


	result=D3DX11CreateShaderResourceViewFromFile(dev.Device(), "texture/height.png", nullptr, nullptr, &_groundTex, &result);
	dev.Context()->PSSetShaderResources(0, 1, &_groundTex);

	result = D3DX11CreateShaderResourceViewFromFile(dev.Device(), "texture/asiato.png", nullptr, nullptr, &_displacementTex, &result);
	dev.Context()->DSSetShaderResources(7, 1, &_displacementTex);
}


TessPlane::~TessPlane()
{
}

void
TessPlane::Draw()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	dev.Context()->DSSetConstantBuffers(0, 1, &_matrixBuffer);


	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mappedMatrixies);
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mappedMatrixies.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
	//↑　*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違えるとメモリがぐちゃぐちゃになる
	dev.Context()->Unmap(_matrixBuffer, 0);


	dev.Context()->VSSetShader(_vertexShader, nullptr, 0);
	dev.Context()->IASetInputLayout(_inputlayout);
	dev.Context()->HSSetShader(_hullShader, nullptr, 0);
	dev.Context()->DSSetShader(_domainShader, nullptr, 0);
	dev.Context()->PSSetShader(_pixelShader, nullptr, 0);
	//プリミティブトポロジの切り替えを忘れない　切り替えを頻発させるのは良くない
	//dev.Context()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//テクスチャセット
	dev.Context()->PSSetShaderResources(0, 1, &_groundTex);
	dev.Context()->DSSetShaderResources(7, 1, &_displacementTex);
	dev.Context()->DSSetSamplers(0, 1, &_samplerState_Wrap);

	unsigned int stride = sizeof(float) * 14;
	unsigned int offset = 0;
	dev.Context()->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	dev.Context()->Draw(4, 0);

	dev.Context()->HSSetShader(nullptr, nullptr, 0);
	dev.Context()->DSSetShader(nullptr, nullptr, 0);
}
void
TessPlane::DrawLightView()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);


	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mappedMatrixies);
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mappedMatrixies.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
	//↑　*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違えるとメモリがぐちゃぐちゃになる
	dev.Context()->Unmap(_matrixBuffer, 0);


	dev.Context()->VSSetShader(_lightviewVS, nullptr, 0);
	dev.Context()->IASetInputLayout(_lightviewInputLayout);
	dev.Context()->PSSetShader(_lightviewPS, nullptr, 0);
	//プリミティブトポロジの切り替えを忘れない　切り替えを頻発させるのは良くない
	dev.Context()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	unsigned int stride = sizeof(float) * 14;
	unsigned int offset = 0;
	dev.Context()->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	dev.Context()->Draw(4, 0);
}

void
TessPlane::Update()
{
	rot += 0.2f * XM_PI / 180;
	XMMATRIX rotMatrix = XMMatrixRotationY(rot);
	//_modelMatrix = rotMatrix;
	_modelMatrix = XMMatrixIdentity();
	_worldAndCamera.world = XMMatrixIdentity();
	_worldAndCamera.cameraView = _cameraRef.CameraView();
	_worldAndCamera.cameraProj = _cameraRef.CameraProjection();
	_worldAndCamera.lightView = _cameraRef.LightView();
	_worldAndCamera.lightProj = _cameraRef.LightProjection();

}