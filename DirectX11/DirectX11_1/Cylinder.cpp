#include "Cylinder.h"
#include<vector>
#include<D3D11.h>
#include<xnamath.h>
#include"DeviceDx11.h"
#include"Camera.h"
#include"ShaderDefine.h"
#include"ResourceManager.h"

Cylinder::Cylinder(float radius, float height, unsigned int div,const std::shared_ptr<Camera>& camera) 
{
	InitTransform();
	_cameraPtr = camera;
	ResourceManager& resourceMgr = ResourceManager::Instance();
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

		vertices[i].binormal = Vector3(0, 1, 0);
		vertices[i].tangent = vertices[i].normal.Cross(Vector3(0, 1, 0));
		vertices[i+1].binormal = Vector3(0, 1, 0);
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
		hatchVerts[i].uv.x = (hatchVerts[i].pos.x / radius + 1.0f) / 2.0f;
		hatchVerts[i].uv.y = -(hatchVerts[i].pos.z / radius + 1.0f) / 2.0f;
		hatchVerts[i].tangent = XMFLOAT3(1, 0, 0);
		hatchVerts[i].binormal = XMFLOAT3(0, 0, -1);
		rad += (360 / div)*XM_PI / 180;
		hatchVerts[i+1].pos = {XMFLOAT3(cos(-rad)*radius,height,sin(-rad)*radius)};
		hatchVerts[i+1].normal = XMFLOAT3(0, 1, 0);
		hatchVerts[i+1].uv.x = (hatchVerts[i+1].pos.x / radius + 1.0f) / 2.0f;
		hatchVerts[i+1].uv.y = -(hatchVerts[i+1].pos.z / radius + 1.0f) / 2.0f;
		hatchVerts[i + 1].tangent = XMFLOAT3(1, 0, 0);
		hatchVerts[i + 1].binormal = XMFLOAT3(0, 0,-1);
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

	resourceMgr.LoadVS("Cylinder_VS",
		"wood.hlsl", "woodVS", "vs_5_0",
		_vertexShader, inputElementDescs, sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		_inputlayout);
	resourceMgr.LoadPS("Cylinder_PS",
		"wood.hlsl", "woodPS", "ps_5_0", _pixelShader);
	
	_mainTex = resourceMgr.LoadSRV("Cylinder_main", "wood.png");
	_subTex = resourceMgr.LoadSRV("Cylinder_sub", "noise.png");
	_normalTex = resourceMgr.LoadSRV("Cylinder_normal", "normal.png");

	
	//カメラからの描画に使ったinputElementDescsを使っても描画できた
	//問題が起きた時はLightview用に新しくバーテックスバッファ作ってこの辺も書き換える
	resourceMgr.LoadVS("Cylinder_lightVS",
		"lightview.hlsl", "PrimitiveLightViewVS", "vs_5_0",
		_lightviewVS, lightViewInputElementDescs, sizeof(lightViewInputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		_lightviewInputLayout);
	resourceMgr.LoadPS("Cylinder_lightPS",
		"lightview.hlsl", "PrimitiveLightViewPS", "ps_5_0",
		_lightviewPS);

	
	
	UpdateMatrixies();

	//mvp行列用のバッファ作る
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

	ApplyConstantBuffer(_matrixBuffer, _mappedMatrixies, _worldAndCamera);

}
Cylinder::Cylinder(const std::shared_ptr<Camera>& camera)
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

	_worldAndCamera.lightView = _cameraPtr.lock()->LightView();
	_worldAndCamera.lightProj = _cameraPtr.lock()->LightProjection();


	ApplyConstantBuffer(_matrixBuffer, _mappedMatrixies, _worldAndCamera);
	ApplyCameraShaders();
	ApplyTextures();

	//プリミティブトポロジの切り替えを忘れない　切り替えを頻発させるのは良くない
	dev.Context()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	unsigned int stride = sizeof(float) * 14;
	unsigned int offset = 0;
	dev.Context()->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	dev.Context()->Draw(_vertexCnt, 0);

	dev.Context()->IASetVertexBuffers(0, 1, &_hatchBuffer, &stride, &offset);
	dev.Context()->Draw(_hatchVertCnt, 0);

	XMMATRIX trans = XMMatrixTranslation(_pos.x, _pos.y , _pos.z);
	float calcRad = XM_PI / 180.0f;
	XMMATRIX temp = XMMatrixTranslation(0, -_height, 0);
	XMMATRIX rot = XMMatrixRotationRollPitchYaw(_rot.x*calcRad, _rot.y*calcRad , (_rot.z+180.0f)*calcRad );
	XMMATRIX scal = XMMatrixScaling(_scale.x, _scale.y, _scale.z);

	XMMATRIX t = XMMatrixMultiply(temp, XMMatrixMultiply(rot,scal));

	_worldAndCamera.world = XMMatrixMultiply(t, trans);
	ApplyConstantBuffer(_matrixBuffer, _mappedMatrixies, _worldAndCamera);
	dev.Context()->Draw(_hatchVertCnt, 0);

}
void
Cylinder::DrawLightView()//後々プレイヤーからカメラを弄った場合はここでもカメラから変更後の行列を取ってくる
{
	DeviceDx11& dev = DeviceDx11::Instance();

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);

	_worldAndCamera.lightView = _cameraPtr.lock()->LightView();
	_worldAndCamera.lightProj = _cameraPtr.lock()->LightProjection();

	ApplyConstantBuffer(_matrixBuffer, _mappedMatrixies, _worldAndCamera);
	ApplyDepthShaders();

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

	XMMATRIX view = _cameraPtr.lock()->CameraView();
	XMMATRIX proj = _cameraPtr.lock()->CameraProjection();
	_worldAndCamera.lightView = view;
	_worldAndCamera.lightProj = proj;


	ApplyConstantBuffer(_matrixBuffer, _mappedMatrixies, _worldAndCamera);
	ApplyDepthShaders();

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
	UpdateMatrixies();
}
