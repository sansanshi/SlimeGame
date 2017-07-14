#include "Plane.h"
#include<vector>
#include"DeviceDx11.h"
#include"ShaderDefine.h"
#include"Camera.h"

Plane::Plane(const std::shared_ptr<Camera> camera)
{
};

Plane::Plane(float width, float depth, Vector3 normal,
	const std::shared_ptr<Camera> camera) 
{
	_cameraPtr = camera;
	InitTransform();

	DeviceDx11& dev = DeviceDx11::Instance();
	ResourceManager& resourceMgr = ResourceManager::Instance();
	

	std::vector<PrimitiveVertex> vertices(4);
	vertices[0] = { Vector3(-width / 2, 0, -depth / 2), Vector3(0, 1, 0), Vector2(0, 1), Vector3(1, 0, 0), Vector3(0, 0, 1) };
	vertices[1] = { Vector3(-width / 2, 0, depth / 2), Vector3(0, 1, 0), Vector2(0, 0), Vector3(1, 0, 0), Vector3(0, 0, 1) };
	vertices[2] = { Vector3(width / 2, 0, -depth / 2), Vector3(0, 1, 0), Vector2(1, 1), Vector3(1, 0, 0), Vector3(0, 0, 1) };
	vertices[3] = { Vector3(width / 2, 0, depth / 2), Vector3(0, 1, 0), Vector2(1, 0), Vector3(1, 0, 0), Vector3(0, 0, 1) };

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &vertices[0];

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = sizeof(PrimitiveVertex)*vertices.size();
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = sizeof(PrimitiveVertex);

	HRESULT result=S_OK;
	result = dev.Device()->CreateBuffer(&desc,&data,&_vertexBuffer);
	
	D3D11_INPUT_ELEMENT_DESC inputElementDescs[]=
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


	resourceMgr.LoadVS("Plane_VS", "water.hlsl", "WaterVS", "vs_5_0",
		_vertexShader, inputElementDescs, sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		_inputlayout);
	resourceMgr.LoadPS("Plane_PS", "water.hlsl", "WaterPS", "ps_5_0", _pixelShader);

	resourceMgr.LoadVS("Plane_lightVS", "lightview.hlsl", "PrimitiveLightViewVS", "vs_5_0",
		_lightviewVS, inputElementDescs, sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		_lightviewInputLayout);
	resourceMgr.LoadPS("Plane_lightPS", "lightview.hlsl", "PrimitiveLightViewPS", "ps_5_0", _lightviewPS);

	
	_mainTex = resourceMgr.LoadSRV("Plane_main", "watertest.png");
	_subTex = resourceMgr.LoadSRV("Plane_sub", "noise__.png");
	_normalTex = resourceMgr.LoadSRV("Plane_normal", "normal4.png");
	_flowTex = resourceMgr.LoadSRV("Plane_flow", "flow_.png");


	
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	dev.Device()->CreateSamplerState(&samplerDesc, &_samplerState);

	
	UpdateMatrixies();


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

	ApplyConstantBuffer(_matrixBuffer, _mappedMatrixies, _worldAndCamera);

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);
}


Plane::~Plane()
{
}

void
Plane::Draw()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);

	_worldAndCamera.cameraView = _cameraPtr.lock()->CameraView();
	_worldAndCamera.cameraProj = _cameraPtr.lock()->CameraProjection();
	_worldAndCamera.lightView = _cameraPtr.lock()->LightView();
	_worldAndCamera.lightProj = _cameraPtr.lock()->LightProjection();

	ApplyConstantBuffer(_matrixBuffer, _mappedMatrixies, _worldAndCamera);
	ApplyCameraShaders();
	ApplyTextures();

	dev.Context()->PSSetSamplers(0, 1, &_samplerState);

	//プリミティブトポロジの切り替えを忘れない　切り替えを頻発させるのは良くない
	dev.Context()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	unsigned int stride = sizeof(float) * 14;
	unsigned int offset = 0;
	dev.Context()->IASetVertexBuffers(0, 1, &_vertexBuffer,&stride,&offset);
	dev.Context()->Draw(4,0);
}
void
Plane::DrawLightView()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);

	_worldAndCamera.cameraView = _cameraPtr.lock()->CameraView();
	_worldAndCamera.cameraProj = _cameraPtr.lock()->CameraProjection();
	_worldAndCamera.lightView = _cameraPtr.lock()->LightView();
	_worldAndCamera.lightProj = _cameraPtr.lock()->LightProjection();

	ApplyConstantBuffer(_matrixBuffer, _mappedMatrixies, _worldAndCamera);
	ApplyDepthShaders();

	//プリミティブトポロジの切り替えを忘れない　切り替えを頻発させるのは良くない
	dev.Context()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	unsigned int stride = sizeof(float) * 14;
	unsigned int offset = 0;
	dev.Context()->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	dev.Context()->Draw(4, 0);
}
void 
Plane::DrawCameraDepth()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);

	//
	XMMATRIX cameraView = _cameraPtr.lock()->CameraView();
	_worldAndCamera.lightView = cameraView;
	_worldAndCamera.lightProj = _cameraPtr.lock()->CameraProjection();

	ApplyConstantBuffer(_matrixBuffer, _mappedMatrixies, _worldAndCamera);
	ApplyDepthShaders();
	//プリミティブトポロジの切り替えを忘れない　切り替えを頻発させるのは良くない
	dev.Context()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	unsigned int stride = sizeof(float) * 14;
	unsigned int offset = 0;
	dev.Context()->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	dev.Context()->Draw(4, 0);
}

void
Plane::Update()
{
	UpdateMatrixies();

}