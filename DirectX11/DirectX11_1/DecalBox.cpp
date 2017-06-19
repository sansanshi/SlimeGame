#include "DecalBox.h"
#include<vector>
#include"ShaderGenerator.h"
#include"DeviceDx11.h"
#include<xnamath.h>
#include"Camera.h"

#include"ShaderDefine.h"

DecalBox::DecalBox(float width, float height, float length, Camera* cameraPtr) 
	:_pos(0.f,0.f,0.f),_rot(45.f,0.f,0.f),_scale(16.f,16.f,16.f),_cameraPtr(cameraPtr)
{
	_pos = XMFLOAT3(0, 0, 0);
	DeviceDx11& dev = DeviceDx11::Instance();

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

	ShaderGenerator::CreateVertexShader("Decal.hlsl", "DecalBoxVS_Debug", "vs_5_0",
		_vertexShader, inputElementDescs, sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC), _inputlayout);
	ShaderGenerator::CreatePixelShader("Decal.hlsl", "DecalBoxPS_Debug", "ps_5_0", _pixelShader);


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
	//*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違えるとメモリがぐちゃぐちゃになる
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

	D3DX11CreateShaderResourceViewFromFile(dev.Device(), "texture/bloodhand.png", nullptr, nullptr,
		&_decalTexture, &result);


}
DecalBox::DecalBox(const XMFLOAT3& pos, const XMFLOAT3& rot, const XMFLOAT3& scale,
	Camera* cameraPtr,std::shared_ptr<ID3D11ShaderResourceView*> texPtr,
	ID3D11VertexShader* vs,ID3D11PixelShader* ps,ID3D11InputLayout* layout,
	ID3D11Buffer* vertBuff,ID3D11Buffer* indexBuff,unsigned int indicesCnt) 
	:_pos(pos),_rot(rot),_scale(scale),_cameraPtr(cameraPtr)
{
	DeviceDx11& dev = DeviceDx11::Instance();
	HRESULT result = S_OK;
	_decalTex = texPtr;
	_vertexShader = vs;
	_pixelShader = ps;
	_inputlayout = layout;

	_vertexBuffer = vertBuff;
	_indexBuffer = indexBuff;
	_indicesCnt = indicesCnt;

	//std::vector<XMFLOAT3> verts(8);
	//verts[0] = { -0.5f,0.5f,0.5f };//{ -width / 2.0f, height / 2.0f, length / 2.0f };
	//verts[1] = { 0.5f,0.5f,0.5f };//{ width / 2.0f, height / 2.0f, length / 2.0f };
	//verts[2] = { -0.5f,0.5f,-0.5f };//{ -width / 2.0f, height / 2.0f, -length / 2.0f };
	//verts[3] = { 0.5f, 0.5f, -0.5f };//{ width / 2.0f, height / 2.0f, -length / 2.0f };

	//verts[4] = { -0.5f,-0.5f,0.5f };//{ -width / 2.0f, -height / 2.0f, length / 2.0f };
	//verts[5] = { 0.5f,-0.5f,0.5f };//{ width / 2.0f, -height / 2.0f, length / 2.0f };
	//verts[6] = { -0.5f,-0.5f,-0.5f };//{ -width / 2.0f, -height / 2.0f, -length / 2.0f };
	//verts[7] = { 0.5f,-0.5f,-0.5f };//{ width / 2.0f, -height / 2.0f, -length / 2.0f };

	//D3D11_SUBRESOURCE_DATA data;
	//data.pSysMem = &verts[0];

	//D3D11_BUFFER_DESC desc = {};
	//desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//desc.ByteWidth = sizeof(XMFLOAT3)*verts.size();
	//desc.Usage = D3D11_USAGE_DEFAULT;
	//desc.CPUAccessFlags = 0;
	//desc.MiscFlags = 0;
	//desc.StructureByteStride = sizeof(XMFLOAT3);

	
	//result = dev.Device()->CreateBuffer(&desc, &data, &_vertexBuffer);

	/*D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};*/

	/*ShaderGenerator::CreateVertexShader("Decal.hlsl", "DecalBoxVS", "vs_5_0",
		_vertexShader, inputElementDescs, sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC), _inputlayout);
	ShaderGenerator::CreatePixelShader("Decal.hlsl", "DecalBoxPS", "ps_5_0", _pixelShader);
*/

	/*std::vector<unsigned short> indices(36);
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
	result = dev.Device()->CreateBuffer(&indexBuffDesc, &indexData, &_indexBuffer);*/


	_modelMatrix = XMMatrixIdentity();
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
	//*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違えるとメモリがぐちゃぐちゃになる
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

	/*D3DX11CreateShaderResourceViewFromFile(dev.Device(), "texture/bloodhand.png", nullptr, nullptr,
		&_decalTexture, &result);*/


}

DecalBox::~DecalBox()
{
}

void
DecalBox::DebugDraw()
{
	_matrixies.world = _modelMatrix;
	_matrixies.view = _cameraPtr->CameraView();
	_matrixies.proj = _cameraPtr->CameraProjection();

	XMVECTOR dummy;
	XMMATRIX view = _cameraPtr->CameraView();
	XMMATRIX invView = XMMatrixInverse(&dummy, view);
	_matrixies.invView = invView;
	XMMATRIX world = _matrixies.world;
	XMMATRIX invWorld = XMMatrixInverse(&dummy, world);
	_matrixies.invWorld = invWorld;
	XMMATRIX proj = _matrixies.proj;
	XMMATRIX invProj = XMMatrixInverse(&dummy, proj);
	_matrixies.invProj = invProj;

	XMMATRIX tes = XMMatrixMultiply(invProj, proj);

	XMMATRIX vp = XMMatrixMultiply(view, proj);
	XMMATRIX wvp = XMMatrixMultiply(world, vp);
	XMMATRIX invWVP = XMMatrixInverse(&dummy, wvp);
	_matrixies.invWVP = invWVP;
	_matrixies.wvp = wvp;

	
	DeviceDx11& dev = DeviceDx11::Instance();


	unsigned int stride = sizeof(float) * 3;
	unsigned int offset = 0;

	dev.Context()->VSSetShader(_vertexShader, nullptr, 0);//ＰＭＤモデル表示用シェーダセット
	dev.Context()->PSSetShader(_pixelShader, nullptr, 0);//PMDモデル表示用シェーダセット
	dev.Context()->IASetInputLayout(_inputlayout);


	dev.Context()->PSSetShaderResources(TEXTURE_DECAL, 1, &_decalTexture);



	dev.Context()->VSSetConstantBuffers(1, 1, &_matrixBuffer);
	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mappedMatrixies);
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mappedMatrixies.pData, (void*)(&_matrixies), sizeof(_matrixies));
	//↑　*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違えるとメモリがぐちゃぐちゃになる
	dev.Context()->Unmap(_matrixBuffer, 0);
	dev.Context()->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	dev.Context()->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R16_UINT, 0);


	dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dev.Context()->DrawIndexed(_indicesCnt, 0, 0);
}
void
DecalBox::Draw()
{
	/*XMMATRIX transMatrix = XMMatrixTranslation(_pos.x, _pos.y, _pos.z);
	_modelMatrix = transMatrix;*/

	_matrixies.world = _modelMatrix;
	_matrixies.view = _cameraPtr->CameraView();
	_matrixies.proj = _cameraPtr->CameraProjection();

	XMVECTOR dummy;
	XMMATRIX view = _cameraPtr->CameraView();
	XMMATRIX invView = XMMatrixInverse(&dummy, view);
	_matrixies.invView = invView;
	XMMATRIX world = _matrixies.world;
	XMMATRIX invWorld = XMMatrixInverse(&dummy, world);
	_matrixies.invWorld = invWorld;
	XMMATRIX proj = _matrixies.proj;
	XMMATRIX invProj = XMMatrixInverse(&dummy, proj);
	_matrixies.invProj = invProj;

	XMMATRIX tes = XMMatrixMultiply(invProj, proj);

	XMMATRIX vp = XMMatrixMultiply(view, proj);
	XMMATRIX wvp = XMMatrixMultiply(world,vp);
	XMMATRIX invWVP = XMMatrixInverse(&dummy, wvp);
	_matrixies.invWVP = invWVP;
	_matrixies.wvp = wvp;

	XMMATRIX invtest = XMMatrixMultiply(view, invView);

	XMFLOAT4 postest = { 4, 4, -4 , 1 };
	XMVECTOR v = XMLoadFloat4(&postest);
	XMFLOAT3 ret;

	v = XMVector3TransformCoord(v, world);
	v = XMVector3TransformCoord(v, view);
	
	v = XMVector3TransformCoord(v, proj);//ワールド座標
	//v = XMVector3TransformCoord(v, invWorld);//ローカル座標
	XMStoreFloat3(&ret, v);

	v = XMVector3TransformCoord(v, invProj);
	v = XMVector3TransformCoord(v, invView);
	v = XMVector3TransformCoord(v, invWorld);

	XMStoreFloat3(&ret, v);
	


	DeviceDx11& dev = DeviceDx11::Instance();


	unsigned int stride = sizeof(float) * 3;
	unsigned int offset = 0;

	dev.Context()->VSSetShader(_vertexShader, nullptr, 0);//ＰＭＤモデル表示用シェーダセット
	dev.Context()->PSSetShader(_pixelShader, nullptr, 0);//PMDモデル表示用シェーダセット
	dev.Context()->IASetInputLayout(_inputlayout);

	ID3D11ShaderResourceView** temp = _decalTex.get();
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
	dev.Context()->DrawIndexed(_indicesCnt, 0, 0);
}

void 
DecalBox::Update()
{
	XMMATRIX transMatrix = XMMatrixTranslation(_pos.x, _pos.y+4.0f, _pos.z);
	float calcDeg = 1.0f;//XM_PI / 180.0f;
	XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(_rot.x* calcDeg,_rot.y*calcDeg , _rot.z*calcDeg);
	XMMATRIX scaleMatrix = XMMatrixScaling(_scale.x, _scale.y, _scale.z);

	_modelMatrix =  XMMatrixMultiply(rotMatrix, scaleMatrix);
	_modelMatrix = XMMatrixMultiply(_modelMatrix, transMatrix);
	_matrixies.world = _modelMatrix;
	//_matrixies.view = _cameraPtr->CameraView();
	//_matrixies.proj = _cameraPtr->CameraProjection();

	//XMVECTOR dummy;
	//XMMATRIX view = _cameraPtr->CameraView();
	//_matrixies.invView = XMMatrixInverse(&dummy, view);
	//XMMATRIX world = _matrixies.world;
	//_matrixies.invWorld = XMMatrixInverse(&dummy, world);
	//XMMATRIX proj = _matrixies.proj;
	//_matrixies.invProj = XMMatrixInverse(&dummy, proj);


	//XMMATRIX vp = XMMatrixMultiply(proj, view);
	//XMMATRIX wvp = XMMatrixMultiply(vp,world);
	//XMMATRIX invWVP = XMMatrixInverse(&dummy, wvp);
	//_matrixies.invWVP = invWVP;
	//_matrixies.wvp = wvp;
	////とりあえずinvWVPは多分合ってる

	//int k = 0;
}

