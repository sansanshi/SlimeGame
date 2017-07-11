#include "DecalFactory.h"
#include"DecalBox.h"
#include"Camera.h"
#include"DeviceDx11.h"
#include"ShaderGenerator.h"
#include"ShaderDefine.h"
#include"ResourceManager.h"

const int MAX_DECAL_NUM = 300;

DecalFactory::DecalFactory(const std::shared_ptr<Camera>& cameraPtr):_cameraPtr(cameraPtr),_decalBoxCnt(0)
{
	HRESULT result = S_OK;
	DeviceDx11& dev = DeviceDx11::Instance();
	ResourceManager& resourceMgr = ResourceManager::Instance();

	_decalBoxes.reserve(MAX_DECAL_NUM);

	_decalTex = resourceMgr.LoadSRV("Decal_main", "bloodhand.png");


	//バーテックスバッファ作成
	std::vector<XMFLOAT3> verts(8);
	verts[0] = { -0.5f,0.5f,0.5f };//{ -width / 2.0f, height / 2.0f, length / 2.0f };
	verts[1] = { 0.5f,0.5f,0.5f };//{ width / 2.0f, height / 2.0f, length / 2.0f };
	verts[2] = { -0.5f,0.5f,-0.5f };//{ -width / 2.0f, height / 2.0f, -length / 2.0f };
	verts[3] = { 0.5f, 0.5f, -0.5f };//{ width / 2.0f, height / 2.0f, -length / 2.0f };

	verts[4] = { -0.5f,-0.5f,0.5f };//{ -width / 2.0f, -height / 2.0f, length / 2.0f };
	verts[5] = { 0.5f,-0.5f,0.5f };//{ width / 2.0f, -height / 2.0f, length / 2.0f };
	verts[6] = { -0.5f,-0.5f,-0.5f };//{ -width / 2.0f, -height / 2.0f, -length / 2.0f };
	verts[7] = { 0.5f,-0.5f,-0.5f };//{ width / 2.0f, -height / 2.0f, -length / 2.0f };

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &verts[0];

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = sizeof(XMFLOAT3)*verts.size();
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = sizeof(XMFLOAT3);

	result = dev.Device()->CreateBuffer(&desc, &data, &_vertexBuffer);

	//インデックスバッファ作成
	std::vector<unsigned short> indices(36);
	indices = { 2, 0, 1, 1, 3, 2, 4, 0, 2, 2, 6, 4,
		5, 1, 0, 0, 4, 5, 7, 3, 1, 1, 5, 7, 6, 2, 3, 3, 7, 6, 4, 6, 7, 7, 5, 4 };
	_indicesCnt = indices.size();

	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = &indices[0];

	D3D11_BUFFER_DESC indexBuffDesc = {};
	indexBuffDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBuffDesc.ByteWidth = sizeof(unsigned short) * 36;
	indexBuffDesc.CPUAccessFlags = 0;
	indexBuffDesc.MiscFlags = 0;
	indexBuffDesc.Usage = D3D11_USAGE_DEFAULT;
	result = dev.Device()->CreateBuffer(&indexBuffDesc, &indexData, &_indexBuffer);


	//シェーダ作成
	D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	resourceMgr.LoadVS("DecalBoxVS",
		"Decal.hlsl", "DecalBoxVS", "vs_5_0",
		_vertexShader, inputElementDescs, sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		_inputlayout);
	resourceMgr.LoadPS("DecalBoxPS",
		"Decal.hlsl", "DecalBoxPS", "ps_5_0",
		_pixelShader);
	/*ShaderGenerator::CreateVertexShader("Decal.hlsl", "DecalBoxVS", "vs_5_0",
		_vertexShader, inputElementDescs, sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC), _inputlayout);
	ShaderGenerator::CreatePixelShader("Decal.hlsl", "DecalBoxPS", "ps_5_0", _pixelShader);*/

	for (auto& w : _matrixies.world)
	{
		w = XMMatrixIdentity();
	}
	for (auto& invW : _matrixies.invWorld)
	{
		invW = XMMatrixIdentity();
	}
	XMVECTOR dummy;
	XMMATRIX view = _cameraPtr.lock()->CameraView();
	XMMATRIX invView = XMMatrixInverse(&dummy, view);
	_matrixies.invView = invView;
	XMMATRIX proj = _cameraPtr.lock()->CameraProjection();
	XMMATRIX invProj = XMMatrixInverse(&dummy, proj);
	_matrixies.invProj = invProj;

	_matrixies.view = view;
	_matrixies.proj = proj;

	D3D11_BUFFER_DESC matBuffDesc = {};
	matBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matBuffDesc.ByteWidth = sizeof(MatrixiesForDecalTest);
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

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);


	//サンプラの設定
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	dev.Device()->CreateSamplerState(&samplerDesc, &_samplerState);

	dev.Context()->PSSetSamplers(0, 1, &_samplerState);
	dev.Context()->VSSetSamplers(1, 1, &_samplerState);

}


DecalFactory::~DecalFactory()
{
	for (auto& d : _decalBoxes)
	{
		delete(d);
	}
}

void
DecalFactory::CreateDecalBox(const XMFLOAT3& pos)
{

}
void
DecalFactory::CreateDecalBox(const XMFLOAT3& pos, const XMFLOAT3& rot, const XMFLOAT3& scale)
{
	if (_decalBoxes.size() < MAX_DECAL_NUM) 
	{
		/*DecalBox* d = new DecalBox(pos, rot, scale, _cameraPtr, _decalTex,
			_vertexShader, _pixelShader, _inputlayout,
			_vertexBuffer, _indexBuffer, _indicesCnt);
		_decalBoxes.push_back(d);*/

		XMMATRIX transMat = XMMatrixTranslation(pos.x, pos.y, pos.z);
		XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
		XMMATRIX scaleMat = XMMatrixScaling(scale.x, scale.y, scale.z);
		XMMATRIX world = XMMatrixMultiply(XMMatrixMultiply(rotMat, scaleMat), transMat);
		XMVECTOR dummy;
		XMMATRIX invWorld = XMMatrixInverse(&dummy, world);

		_worldMatrixes[_decalBoxCnt] = world;
		_invWorldMatrixes[_decalBoxCnt] = invWorld;

		++_decalBoxCnt;
	}
}

void
DecalFactory::CreateDecalBox(const XMMATRIX& worldMat)
{
	if (_decalBoxCnt < MAX_DECAL_NUM)
	{
		XMVECTOR dummy;
		XMMATRIX world = worldMat;
		XMMATRIX invWorld = XMMatrixInverse(&dummy, world);

		_worldMatrixes[_decalBoxCnt] = world;
		_invWorldMatrixes[_decalBoxCnt] = invWorld;

		++_decalBoxCnt;
	}
}

void
DecalFactory::Update()
{
	/*for (auto& d : _decalBoxes)
	{
		d->Update();
	}*/
}

void 
DecalFactory::Draw()
{
	DeviceDx11& dev = DeviceDx11::Instance();


	XMVECTOR dummy;
	XMMATRIX view = _cameraPtr.lock()->CameraView();
	XMMATRIX invView = XMMatrixInverse(&dummy, view);
	_matrixies.invView = invView;
	XMMATRIX proj = _cameraPtr.lock()->CameraProjection();
	XMMATRIX invProj = XMMatrixInverse(&dummy, proj);
	_matrixies.invProj = invProj;

	_matrixies.view = view;
	_matrixies.proj = proj;

	for (int i = 0; i < 300; ++i)
	{
		_matrixies.world[i] = _worldMatrixes[i];
		_matrixies.invWorld[i] = _invWorldMatrixes[i];
	}



	unsigned int stride = sizeof(float) * 3;
	unsigned int offset = 0;

	dev.Context()->VSSetShader(*_vertexShader.lock(), nullptr, 0);//ＰＭＤモデル表示用シェーダセット
	dev.Context()->PSSetShader(*_pixelShader.lock(), nullptr, 0);//PMDモデル表示用シェーダセット
	dev.Context()->IASetInputLayout(*_inputlayout.lock());

	ID3D11ShaderResourceView** temp = _decalTex._Get();
	dev.Context()->PSSetShaderResources(TEXTURE_DECAL, 1, temp);



	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);
	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mappedMatrixies);
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mappedMatrixies.pData, (void*)(&_matrixies), sizeof(_matrixies));
	//↑　*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違えるとメモリがぐちゃぐちゃになる
	dev.Context()->Unmap(_matrixBuffer, 0);
	dev.Context()->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	dev.Context()->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R16_UINT, 0);


	dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//dev.Context()->DrawIndexed(_indicesCnt, 0, 0);
	dev.Context()->DrawIndexedInstanced(_indicesCnt, _decalBoxCnt, 0, 0, 0);


	/*for (auto& d : _decalBoxes)
	{
		d->Draw();
	}*/
}