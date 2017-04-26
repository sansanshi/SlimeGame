#include "Cylinder.h"
#include<vector>
#include<D3D11.h>
#include<xnamath.h>
#include"DeviceDx11.h"
#include"ShaderGenerator.h"


Cylinder::Cylinder(float radius, float height, unsigned int div, Camera& camera) :_cameraRef(camera)
{
	angle = 0.0f;
	_height = height;
	_radius = radius;
	_div = div;

	_vertexCnt = div * 2 + 2;
	std::vector<PrimitiveVertex> vertices(div * 2 + 2);
	float angle = 0.0f;
	for (int i = 0; i <= (int)div*2; i+=2)
	{
		//左下の点
		vertices[i].pos = { cos(angle)*radius, 0, sin(angle)*radius };
		vertices[i].normal = { cos(angle), 0, sin(angle) };
		vertices[i].uv = { (1.0f / (float)div)*(float)i, 1.0f };

		vertices[i+1].pos = { cos(angle)*radius, height, sin(angle)*radius };
		vertices[i+1].normal = { cos(angle), 0, sin(angle) };
		vertices[i+1].uv = { (1.0f / (float)div)*(float)i, 0.0f };

		vertices[i].binormal = Vector3(0, -1, 0);
		vertices[i].tangent = vertices[i].normal.Cross(Vector3(0, 1, 0));
		vertices[i+1].binormal = Vector3(0, -1, 0);
		vertices[i+1].tangent = vertices[i+1].normal.Cross(Vector3(0, 1, 0));


		angle += XM_2PI / (float)div;
	}
	HRESULT result = S_OK;

	DeviceDx11& dev = DeviceDx11::Instance();

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &vertices[0];

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = sizeof(PrimitiveVertex)*vertices.size();
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = sizeof(PrimitiveVertex);


	result = dev.Device()->CreateBuffer(&desc, &data, &_vertexBuffer);


	float rad = 0;
	_hatchVertCnt = div;
	std::vector<PrimitiveVertex> hatchVerts(div);
	for (int i = 0; i < (int)hatchVerts.size();i+=2)
	{
		hatchVerts[i].pos = { XMFLOAT3(cos(rad)*radius, height, sin(rad)*radius) };
		hatchVerts[i].normal = XMFLOAT3(0, 1, 0);
		rad += (360 / div)*XM_PI / 180;
		hatchVerts[i+1].pos = {XMFLOAT3(cos(-rad)*radius,height,sin(-rad)*radius)};
		hatchVerts[i+1].normal = XMFLOAT3(0, 1, 0);
	}
	D3D11_SUBRESOURCE_DATA hatchData;
	hatchData.pSysMem = &hatchVerts[0];

	D3D11_BUFFER_DESC hatchDesc = {};
	hatchDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	hatchDesc.ByteWidth = sizeof(PrimitiveVertex)*hatchVerts.size();
	hatchDesc.Usage = D3D11_USAGE_DEFAULT;
	hatchDesc.CPUAccessFlags = 0;
	hatchDesc.MiscFlags = 0;
	hatchDesc.StructureByteStride = sizeof(PrimitiveVertex);


	result = dev.Device()->CreateBuffer(&hatchDesc, &hatchData, &_hatchBuffer);


	D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	D3D11_INPUT_ELEMENT_DESC lightViewInputElementDescs[]=
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ShaderGenerator::CreateVertexShader("BaseShader.hlsl", "PrimitiveVS", "vs_5_0",
		_vertexShader, inputElementDescs, sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC), _inputlayout);
	ShaderGenerator::CreatePixelShader("BaseShader.hlsl", "PrimitivePS", "ps_5_0", _pixelShader);


	//カメラからの描画に使ったinputElementDescsを使っても描画できた
	//問題が起きた時はLightview用に新しくバーテックスバッファ作ってこの辺も書き換える
	ShaderGenerator::CreateVertexShader("lightview.hlsl", "PrimitiveLightViewVS", "vs_5_0",
		_lightviewVS, inputElementDescs, sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC), _lightviewInputLayout);
	ShaderGenerator::CreatePixelShader("lightview.hlsl", "PrimitiveLightViewPS", "ps_5_0", _lightviewPS);

	_modelMatrix = XMMatrixIdentity();
	/*_mvp.worldMatrix = _modelMatrix;
	_mvp.viewMatrix = _cameraRef.GetMatrixies().view;
	_mvp.projectionMatrix = _cameraRef.GetMatrixies().projection;*/
	_worldAndCamera.world = _modelMatrix;
	_worldAndCamera.cameraView = _cameraRef.CameraView();
	_worldAndCamera.cameraProj = _cameraRef.CameraProjection();
	_worldAndCamera.lightView = _cameraRef.LightView();
	_worldAndCamera.lightProj = _cameraRef.LightProjection();

	//mvp行列用のバッファ作る
	D3D11_BUFFER_DESC matBuffDesc = {};
	matBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matBuffDesc.ByteWidth = sizeof(MATRIXIES);
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

}
Cylinder::Cylinder(Camera& camera) :_cameraRef(camera)
{
}


Cylinder::~Cylinder()
{
}

void
Cylinder::Draw()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);

	//_worldAndCamera.lightView = _cameraRef.LightView();
	//_worldAndCamera.lightProj = _cameraRef.LightProjection();

	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mappedMatrixies);
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mappedMatrixies.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
	//↑　*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違えるとメモリがぐちゃぐちゃになる
	dev.Context()->Unmap(_matrixBuffer, 0);

	dev.Context()->VSSetShader(_vertexShader, nullptr, 0);
	dev.Context()->IASetInputLayout(_inputlayout);
	dev.Context()->PSSetShader(_pixelShader, nullptr, 0);
	//プリミティブトポロジの切り替えを忘れない　切り替えを頻発させるのは良くない
	dev.Context()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	unsigned int stride = sizeof(float) * 14;
	unsigned int offset = 0;
	dev.Context()->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	dev.Context()->Draw(_vertexCnt, 0);

	dev.Context()->IASetVertexBuffers(0, 1, &_hatchBuffer, &stride, &offset);
	dev.Context()->Draw(_hatchVertCnt, 0);
}
void
Cylinder::DrawLightView()//後々プレイヤーからカメラを弄った場合はここでもカメラから変更後の行列を取ってくる
{
	DeviceDx11& dev = DeviceDx11::Instance();
	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);

	//_worldAndCamera.lightView = _cameraRef.LightView();
	//_worldAndCamera.lightProj = _cameraRef.LightProjection();


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
	dev.Context()->Draw(_vertexCnt, 0);
	dev.Context()->IASetVertexBuffers(0, 1, &_hatchBuffer, &stride, &offset);
	dev.Context()->Draw(_hatchVertCnt, 0);
}

void
Cylinder::DrawCameraDepth()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);


	_worldAndCamera.lightView = _cameraRef.CameraView();
	_worldAndCamera.lightProj = _cameraRef.CameraProjection();

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
	dev.Context()->Draw(_vertexCnt, 0);
	dev.Context()->IASetVertexBuffers(0, 1, &_hatchBuffer, &stride, &offset);
	dev.Context()->Draw(_hatchVertCnt, 0);
}

void
Cylinder::Update()
{
	XMMATRIX transMatrix = XMMatrixTranslation(0, -_height/2, 0);
	XMMATRIX modelMatrix = XMMatrixIdentity();
	angle += 1.0f * XM_PI / 180.0f;
	XMMATRIX scaleMat = XMMatrixScaling(1.5f, 0.8f, 1.5f);
	XMMATRIX rotMat = XMMatrixRotationZ(angle);
	rotMat = XMMatrixIdentity();
	//↓筒の中心を原点に移動させて回転、拡大
	modelMatrix = XMMatrixMultiply(transMatrix,XMMatrixMultiply(rotMat, scaleMat));
	transMatrix = XMMatrixTranslation(10, 5, 10);
	modelMatrix = XMMatrixMultiply(transMatrix,modelMatrix);

	_modelMatrix = modelMatrix;

	_worldAndCamera.world = _modelMatrix;
	_worldAndCamera.cameraView = _cameraRef.CameraView();
	_worldAndCamera.cameraProj = _cameraRef.CameraProjection();
	_worldAndCamera.lightView = _cameraRef.LightView();
	_worldAndCamera.lightProj = _cameraRef.LightProjection();
}