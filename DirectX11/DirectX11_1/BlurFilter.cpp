#include "BlurFilter.h"
#include "DeviceDx11.h"
#include<vector>
#include"Camera.h"
#include"ShaderDefine.h"
#include"ResourceManager.h"
#include<math.h>

BlurFilter::BlurFilter() 
{
	DeviceDx11& dev = DeviceDx11::Instance();
	HRESULT result = S_OK;
	ResourceManager& resourceMgr = ResourceManager::Instance();

	//ビルボードテスト
	_vertexBuffer = nullptr;
	_vertexBuffer = CreateBlurFilterVertexBuffer();
	_texture = resourceMgr.LoadSRV("BlurFilter_main", "uvCheck_transparent.png");

	D3D11_INPUT_ELEMENT_DESC inputElementDescs[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	resourceMgr.LoadVS("BlurFilter_VS",
		"blur.hlsl",
		"BlurFilterVS",
		"vs_5_0",
		_blurXvs,
		inputElementDescs,
		sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		_inputLayout);
	resourceMgr.LoadPS("BlurXFilter_PS",
		"blur.hlsl",
		"XBlurFilterPS",
		"ps_5_0",
		_blurXps);

	resourceMgr.LoadVS("BlurFilter_VS",
		"blur.hlsl",
		"BlurFilterVS",
		"vs_5_0",
		_blurYvs,
		inputElementDescs,
		sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		_inputLayout);
	resourceMgr.LoadPS("BlurYFilter_PS",
		"blur.hlsl",
		"YBlurFilterPS",
		"ps_5_0",
		_blurYps);


	


	D3D11_BUFFER_DESC blurWeightBuffDesc = {};
	blurWeightBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	blurWeightBuffDesc.ByteWidth = sizeof(BlurWeight) + (16 - sizeof(BlurWeight) % 16) % 16;
	//sizeof(Material) + (16 - sizeof(Material) % 16) % 16;
	blurWeightBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;//バッファの中身はCPUで書き換える
	blurWeightBuffDesc.Usage = D3D11_USAGE_DYNAMIC;//CPUによる書き込み、GPUによる読み込みが行われるという意味
											//matBuffDesc.ByteWidth = sizeof(XMMATRIX);
											//matBuffDesc.StructureByteStride = sizeof(XMMATRIX);

	//とりあえず決め打ち　後で直す
	float weight_[8] = {};
	float t = 0.0f;
	float d = 30.0f*30.0f / 100.0f;
	for (int i = 0; i < 8; ++i)
	{
		float r = 1.0f + 2.0f*i;
		float w = exp(-0.5f*(r*r) / d);
		weight_[i] = w;
		if (i > 0) { w *= 2.0f; }
		t += w;
	}
	for (int i = 0; i < 8; ++i)
	{
		weight_[i] /= t;
	}
	_blurWeight.weight0 = weight_[0];
	_blurWeight.weight1 = weight_[1];
	_blurWeight.weight2 = weight_[2];
	_blurWeight.weight3 = weight_[3];
	_blurWeight.weight4 = weight_[4];
	_blurWeight.weight5 = weight_[5];
	_blurWeight.weight6 = weight_[6];
	_blurWeight.weight7 = weight_[7];

	D3D11_SUBRESOURCE_DATA dat;
	dat.pSysMem = &_blurWeight;

	result = dev.Device()->CreateBuffer(&blurWeightBuffDesc, &dat, &_blurWeightBuffer);

	dev.Context()->Map(_blurWeightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mappedWeight);
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mappedWeight.pData, (void*)(&_blurWeight), sizeof(BlurWeight));

	dev.Context()->Unmap(_blurWeightBuffer, 0);

}


BlurFilter::~BlurFilter()
{

}


void
BlurFilter::DrawBlurX()
{
	DeviceDx11& dev = DeviceDx11::Instance();


	dev.Context()->VSSetShader(*_blurXvs.lock(), nullptr, 0);
	dev.Context()->PSSetShader(*_blurXps.lock(), nullptr, 0);
	dev.Context()->IASetInputLayout(*_inputLayout.lock());

	dev.Context()->PSSetConstantBuffers(0, 1, &_blurWeightBuffer);

	unsigned int hudstride = sizeof(HUDVertex);
	unsigned int hudoffset = 0;
	dev.Context()->IASetVertexBuffers(0, 1, &_vertexBuffer, &hudstride, &hudoffset);

	dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	dev.Context()->Draw(4, 0);
}
void
BlurFilter::DrawBlurY()
{
	DeviceDx11& dev = DeviceDx11::Instance();


	dev.Context()->VSSetShader(*_blurYvs.lock(), nullptr, 0);
	dev.Context()->PSSetShader(*_blurYps.lock(), nullptr, 0);
	dev.Context()->IASetInputLayout(*_inputLayout.lock());

	dev.Context()->PSSetConstantBuffers(0, 1, &_blurWeightBuffer);

	unsigned int hudstride = sizeof(HUDVertex);
	unsigned int hudoffset = 0;
	dev.Context()->IASetVertexBuffers(0, 1, &_vertexBuffer, &hudstride, &hudoffset);

	dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	dev.Context()->Draw(4, 0);
}


void
BlurFilter::Update()
{

}


//ビルボードの頂点バッファ
//@param ビルボード中心X座標(3D座標系
//@param　ビルボード中心Y座標
//@param ビルボード幅（3D座標系
//@param ビルボード高さ(3D座標系
//@param note 幅と高さは中心からの広がり 左上はx-width/2,y+height/2
ID3D11Buffer*
BlurFilter::CreateBlurFilterVertexBuffer()
{
	DeviceDx11& dev = DeviceDx11::Instance();

	std::vector<HUDVertex> vertices(4);
	{
		vertices[0] = { XMFLOAT3(-1, 1, 0), XMFLOAT2(0.f, 0.f) };//左上
		vertices[1] = { XMFLOAT3(1 , 1 , 0), XMFLOAT2(1.0f, 0.0f) };//右上
		vertices[2] = { XMFLOAT3(-1, -1, 0), XMFLOAT2(0.f, 1.f) };//左下
		vertices[3] = { XMFLOAT3(1, -1, 0), XMFLOAT2(1.f, 1.f) };//右下
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

	return vertexBuffer;

}


