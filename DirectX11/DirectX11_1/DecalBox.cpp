#include "DecalBox.h"
#include<vector>
#include<xnamath.h>
#include"DeviceDx11.h"
#include"Camera.h"
#include"ShaderDefine.h"
#include"ResourceManager.h"

DecalBox::DecalBox(float width, float height, float length, const std::shared_ptr<Camera>& cameraPtr) 
	:_cameraPtr(cameraPtr)
{
	_pos = XMFLOAT3(0, 0, 0);
	_rot = XMFLOAT3(45.f, 0.f, 0.f);
	_scale = XMFLOAT3(16.f, 16.f, 16.f);

	DeviceDx11& dev = DeviceDx11::Instance();
	ResourceManager& resourceMgr = ResourceManager::Instance();

	std::vector<XMFLOAT3> verts(8);
	verts[0] = { -width / 2.0f, height / 2.0f, length / 2.0f };
	verts[1] = { width / 2.0f, height / 2.0f, length / 2.0f };
	verts[2] = { -width / 2.0f, height / 2.0f, -length / 2.0f };
	verts[3] = { width / 2.0f, height / 2.0f, -length / 2.0f };

	verts[4] = { -width / 2.0f, -height / 2.0f, length / 2.0f };
	verts[5] = { width / 2.0f, -height / 2.0f, length / 2.0f };
	verts[6] = { -width / 2.0f, -height / 2.0f, -length / 2.0f };
	verts[7] = { width / 2.0f, -height / 2.0f, -length / 2.0f };

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &verts[0];

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = sizeof(XMFLOAT3)*verts.size();
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = sizeof(XMFLOAT3);

	HRESULT result = S_OK;
	result = dev.Device()->CreateBuffer(&desc, &data, &_vertexBuffer);

	D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	resourceMgr.LoadVS("DecalBoxVS_Debug",
		"Decal.hlsl", "DecalBoxVS_Debug", "vs_5_0",
		_vertexShader, inputElementDescs, sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		_inputlayout);
	resourceMgr.LoadPS("DecalBoxPS_Debug",
		"Decal.hlsl", "DecalBoxPS_Debug", "ps_5_0",
		_pixelShader);

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


	_modelMatrix = XMMatrixIdentity();
	_matrixies.world = _modelMatrix;
	_matrixies.view = _cameraPtr.lock()->CameraView();
	_matrixies.proj = _cameraPtr.lock()->CameraProjection();

	XMVECTOR dummy;
	XMMATRIX view = _cameraPtr.lock()->CameraView();
	_matrixies.invView = XMMatrixInverse(&dummy, view);
	XMMATRIX world = _matrixies.world;
	_matrixies.invWorld = XMMatrixInverse(&dummy, world);

	XMMATRIX proj = _cameraPtr.lock()->CameraProjection();
	XMMATRIX vp = XMMatrixMultiply(proj, view);
	XMMATRIX wvp = XMMatrixMultiply(vp, world);
	XMMATRIX invWVP = XMMatrixInverse(&dummy, wvp);


	//mvp行列用のバッファ作る
	//
	D3D11_BUFFER_DESC matBuffDesc = {};
	matBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matBuffDesc.ByteWidth = sizeof(MatrixiesForDecal);
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
	
	dev.Context()->Unmap(_matrixBuffer, 0);

	dev.Context()->VSSetConstantBuffers(1, 1, &_matrixBuffer);


	//サンプラの設定
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	dev.Device()->CreateSamplerState(&samplerDesc, &_samplerState);

	dev.Context()->PSSetSamplers(0, 1, &_samplerState);
	dev.Context()->VSSetSamplers(1, 1, &_samplerState);

	_decalTexture = resourceMgr.LoadSRV("Decal_main", "bloodhand.png");


}
DecalBox::DecalBox(const XMFLOAT3& pos, const XMFLOAT3& rot, const XMFLOAT3& scale,
	const std::shared_ptr<Camera>& cameraPtr,std::shared_ptr<ID3D11ShaderResourceView*> texPtr,
	std::weak_ptr<ID3D11VertexShader*> vs,std::weak_ptr<ID3D11PixelShader*> ps,std::weak_ptr<ID3D11InputLayout*> layout,
	ID3D11Buffer* vertBuff,ID3D11Buffer* indexBuff,unsigned int indicesCnt) 
	:_cameraPtr(cameraPtr)
{
	_pos = pos;
	_rot = rot;
	_scale = scale;
	DeviceDx11& dev = DeviceDx11::Instance();
	HRESULT result = S_OK;
	_decalTexture = texPtr;
	_vertexShader = vs;
	_pixelShader = ps;
	_inputlayout = layout;

	_vertexBuffer = vertBuff;
	_indexBuffer = indexBuff;
	_indicesCnt = indicesCnt;

	

	_modelMatrix = XMMatrixIdentity();
	_matrixies.world = _modelMatrix;
	_matrixies.view = _cameraPtr.lock()->CameraView();
	_matrixies.proj = _cameraPtr.lock()->CameraProjection();

	XMVECTOR dummy;
	XMMATRIX view = _cameraPtr.lock()->CameraView();
	_matrixies.invView = XMMatrixInverse(&dummy, view);
	XMMATRIX world = _matrixies.world;
	_matrixies.invWorld = XMMatrixInverse(&dummy, world);

	XMMATRIX proj = _cameraPtr.lock()->CameraProjection();
	XMMATRIX vp = XMMatrixMultiply(proj, view);
	XMMATRIX wvp = XMMatrixMultiply(vp, world);
	XMMATRIX invWVP = XMMatrixInverse(&dummy, wvp);


	//mvp行列用のバッファ作る
	//
	D3D11_BUFFER_DESC matBuffDesc = {};
	matBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matBuffDesc.ByteWidth = sizeof(MatrixiesForDecal);
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
	
	dev.Context()->Unmap(_matrixBuffer, 0);

	dev.Context()->VSSetConstantBuffers(1, 1, &_matrixBuffer);


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

DecalBox::~DecalBox()
{
}

void
DecalBox::DebugDraw()
{
	DeviceDx11& dev = DeviceDx11::Instance();


	unsigned int stride = sizeof(float) * 3;
	unsigned int offset = 0;
	ID3D11VertexShader* temp = *_vertexShader.lock();
	dev.Context()->VSSetShader(*_vertexShader.lock(), nullptr, 0);//ＰＭＤモデル表示用シェーダセット
	dev.Context()->PSSetShader(*_pixelShader.lock(), nullptr, 0);//PMDモデル表示用シェーダセット
	dev.Context()->IASetInputLayout(*_inputlayout.lock());


	dev.Context()->PSSetShaderResources(TEXTURE_DECAL, 1, _decalTexture._Get());



	dev.Context()->VSSetConstantBuffers(1, 1, &_matrixBuffer);
	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mappedMatrixies);
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mappedMatrixies.pData, (void*)(&_matrixies), sizeof(_matrixies));
	
	dev.Context()->Unmap(_matrixBuffer, 0);
	dev.Context()->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	dev.Context()->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R16_UINT, 0);


	dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dev.Context()->DrawIndexed(_indicesCnt, 0, 0);
}
void
DecalBox::Draw()
{
	DeviceDx11& dev = DeviceDx11::Instance();


	unsigned int stride = sizeof(float) * 3;
	unsigned int offset = 0;

	dev.Context()->VSSetShader(*_vertexShader._Get(), nullptr, 0);//ＰＭＤモデル表示用シェーダセット
	dev.Context()->PSSetShader(*_pixelShader._Get(), nullptr, 0);//PMDモデル表示用シェーダセット
	dev.Context()->IASetInputLayout(*_inputlayout._Get());

	ID3D11ShaderResourceView** temp = _decalTexture._Get();
	dev.Context()->PSSetShaderResources(TEXTURE_DECAL, 1, temp);



	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);
	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mappedMatrixies);
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mappedMatrixies.pData, (void*)(&_matrixies), sizeof(_matrixies));
	
	dev.Context()->Unmap(_matrixBuffer, 0);
	dev.Context()->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	dev.Context()->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R16_UINT, 0);


	dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dev.Context()->DrawIndexed(_indicesCnt, 0, 0);
}

void 
DecalBox::Update()
{
	XMMATRIX transMatrix = XMMatrixTranslation(_pos.x, _pos.y+4.0f, _pos.z);
	XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(_rot.x,_rot.y , _rot.z);
	XMMATRIX scaleMatrix = XMMatrixScaling(_scale.x, _scale.y, _scale.z);

	XMMATRIX modelMatrix  =  XMMatrixMultiply(rotMatrix, scaleMatrix);
	modelMatrix = XMMatrixMultiply(modelMatrix, transMatrix);
	_modelMatrix = modelMatrix;
	_matrixies.world = _modelMatrix;



	
	XMMATRIX view = _cameraPtr.lock()->CameraView();
	XMMATRIX proj = _matrixies.proj;
	_matrixies.view = view;// _cameraPtr.lock()->CameraView();
	_matrixies.proj = proj;// _cameraPtr.lock()->CameraProjection();

	XMVECTOR dummy;
	XMMATRIX invView = XMMatrixInverse(&dummy, view);
	_matrixies.invView = invView;
	XMMATRIX invWorld = XMMatrixInverse(&dummy, modelMatrix);
	_matrixies.invWorld = invWorld;
	XMMATRIX invProj = XMMatrixInverse(&dummy, proj);
	_matrixies.invProj = invProj;
	
}

