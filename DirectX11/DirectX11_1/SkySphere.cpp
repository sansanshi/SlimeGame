#include "SkySphere.h"
#include<vector>
#include"DeviceDx11.h"
#include"ShaderDefine.h"
#include"Camera.h"
#include"ResourceManager.h"

SkySphere::SkySphere(unsigned int divNum, float radius,const std::shared_ptr<Camera>& cam) 
{
	InitTransform();
	_cameraPtr = cam;
	DeviceDx11& dev = DeviceDx11::Instance();
	ResourceManager& resourceMgr = ResourceManager::Instance();

	float theta;
	float phi;
	float angleDelta;

	int nIndex;
	int nIndexX;


	angleDelta = 2.0f*XM_PI / divNum;
	nIndex = 0;
	theta = 0.0f;
	phi = 0.0f;//横　こっちはぐるっと1周する

	std::vector<PrimitiveVertex> vertices;
	int n = (divNum / 2 + 1)*(divNum + 1);
	vertices.resize(n);

	for (int i = 0; i < divNum + 1; i++)
	{
		theta = 0.0f;//縦
		for (int j = 0; j < divNum / 2 + 1; j++)
		{
			vertices[nIndex].pos = Vector3(radius*sinf(theta)*cosf(phi),
				radius*cosf(theta),
				radius*sinf(theta)*sinf(phi));
			Vector3 vec = vertices[nIndex].pos - Vector3(0, 0, 0);
			vertices[nIndex].normal = vec.Normalize();
			//vertices[nIndex].tangent = cosf(theta);
			//vertices[nIndex].binormal = sinf()
			vertices[nIndex].uv = Vector2(-phi / (2.0f * XM_PI), theta / XM_PI);
			if (i > divNum - 1) {
				float test = phi / (2.0f * XM_PI);
				int j = 0;
			}
			nIndex++;
			theta += angleDelta;
		}
		phi += angleDelta;
	}
	int check = 0;

	/*for (int i = 0; i < divNum / 2 + 1; i++)
	{
	phi = 0.0f;
	for (int j = 0; j < divNum + 1; j++)
	{
	vertices[nIndex].pos = Vector3(radius*sinf(theta)*cosf(phi),
	radius*cosf(theta),
	radius*sinf(theta)*sinf(phi));
	Vector3 vec = vertices[nIndex].pos - Vector3(0, 0, 0);
	vertices[nIndex].normal = vec.Normalize();
	vertices[nIndex].uv = Vector2(phi / (2.0f / XM_PI), theta / XM_PI);
	nIndex++;
	phi += angleDelta;
	}
	theta += angleDelta;
	}*/
	int jj = nIndex;
	_verticesCnt = vertices.size();
	int k = 0;
	//インデックスデータ作成
	_indicesCnt = 0;
	int nIndexOffset = 0;
	std::vector<unsigned short> indices;
	indices.resize(divNum * divNum / 2 * 6);
	nIndex = 0;
	for (int i = 0; i < divNum; i++)//横
	{
		for (int j = 0; j < divNum / 2; j++)//縦
		{
			nIndexX = i*(divNum / 2 + 1);
			//ポリゴンの切れ目を消そうとすると最後の部分のポリゴンのUV値が1.0～0.0になる
			indices[nIndex+2] = (nIndexX + j);//&((divNum/2+1)*(divNum));
			indices[nIndex + 1] = (nIndexX + (divNum / 2 + 1) + j);//& ((divNum/2 + 1)*(divNum ));
			indices[nIndex ] = (nIndexX + j + 1);//& ((divNum/2 + 1)*(divNum ));
			nIndex += 3;
			indices[nIndex+2] = (nIndexX + j + 1);//& ((divNum/2 + 1)*(divNum + 1));
			indices[nIndex + 1] = (nIndexX + (divNum / 2 + 1) + j);//& ((divNum/2 + 1)*(divNum ));
			indices[nIndex ] = (nIndexX + (divNum / 2 + 1) + j + 1);//& ((divNum/2 + 1)*(divNum ));
			nIndex += 3;
		}
	}
	int jk = nIndex;
	_indicesCnt = indices.size();

	for (auto& v : vertices)
	{
		Vector3 up = Vector3(0, 1, 0);
		v.tangent = v.normal.Cross(up);
		v.binormal = v.tangent.Cross(v.normal);
		int check = 0;
	}

	



	D3D11_BUFFER_DESC vertBuffDesc = {};
	vertBuffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertBuffDesc.ByteWidth = sizeof(PrimitiveVertex)*vertices.size();
	vertBuffDesc.CPUAccessFlags = 0;
	vertBuffDesc.Usage = D3D11_USAGE_DEFAULT;
	vertBuffDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertData;
	vertData.pSysMem = &vertices[0];

	HRESULT result = S_OK;
	result = dev.Device()->CreateBuffer(&vertBuffDesc, &vertData, &_vertexBuffer);

	D3D11_BUFFER_DESC indexBuffDesc = {};
	indexBuffDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBuffDesc.ByteWidth = sizeof(unsigned short)*indices.size();
	indexBuffDesc.StructureByteStride = sizeof(unsigned short);
	indexBuffDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBuffDesc.CPUAccessFlags = 0;
	indexBuffDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = &indices[0];
	result = dev.Device()->CreateBuffer(&indexBuffDesc, &indexData, &_indexBuffer);


	//シェーダに渡す用のマテリアル構造体
	_material = {};
	_material.diffuse = { 1.0f, 1.0f, 1.0f };
	_material.alpha = 0.8f;
	_material.ambient = { 0.5f, 0.5f, 0.5f };
	_material.specularColor = { 0.5f, 0.5f, 0.5f };
	_material.specularity = 5.0f;


	D3D11_BUFFER_DESC materialBuffDesc = {};
	materialBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//コンスタントバッファのByteWidthは16の倍数でないといけない
	materialBuffDesc.ByteWidth = sizeof(Material) + (16 - sizeof(Material) % 16) % 16;
	materialBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	materialBuffDesc.Usage = D3D11_USAGE_DYNAMIC;
	materialBuffDesc.StructureByteStride = sizeof(Material);

	D3D11_SUBRESOURCE_DATA materialData;
	materialData.pSysMem = &_material;

	result = dev.Device()->CreateBuffer(&materialBuffDesc, &materialData, &_materialBuffer);

	dev.Context()->Map(_materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mappedMaterial);
	memcpy(_mappedMaterial.pData, (void*)(&_material), sizeof(_material));
	dev.Context()->Unmap(_materialBuffer, 0);

	dev.Context()->VSSetConstantBuffers(1, 1, &_materialBuffer);




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
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	resourceMgr.LoadVS("Skysphere_VS", "SkySphere.hlsl", "SkySphereVS", "vs_5_0",
		_vertexShader, inputElementDescs, sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		_inputlayout);
	resourceMgr.LoadPS("Skysphere_PS", "SkySphere.hlsl", "SkySpherePS", "ps_5_0", _pixelShader);
	
	
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

	ApplyMatrixBuffer();

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);

	//テクスチャ
	_mainTex = resourceMgr.LoadSRV("Skysphere_main", "height00.png");



	//サンプラの設定
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	dev.Device()->CreateSamplerState(&samplerDesc, &_samplerState_Wrap);

	dev.Context()->PSSetSamplers(0, 1, &_samplerState_Wrap);
	dev.Context()->VSSetSamplers(1, 1, &_samplerState_Wrap);

	moveForward = 0;
	moveRight = 0;

}

void
SkySphere::CalculateTangentBinormal(TempVertex* v0, TempVertex* v1, TempVertex* v2,
	std::vector<PrimitiveVertex>& vertsForBuff, const std::vector<unsigned short>& indices, int idx)
{
	//後でVector3 Vector2のdouble型を作って試す
	//ついでに平均も取ってみる

	Vector3 vec01, vec02;//頂点0から頂点1、頂点0から頂点2のベクトル
	Vector2 uvVec01, uvVec02;

	vec01 = v1->pos - v0->pos;
	vec02 = v2->pos - v0->pos;

	uvVec01 = v1->uv - v0->uv;
	uvVec02 = v2->uv - v0->uv;

	float den = 1.0f / (uvVec01.x*uvVec02.y - uvVec02.x*uvVec01.y);

	Vector3 tangent;
	Vector3 binormal;

	tangent = vec01*uvVec02.y - vec02*uvVec01.y;//↑3はこっちでもいい

												//binormal.x = (uvVec01.x*vec02.x - uvVec02.x*vec01.x)*den;
												//binormal.y = (uvVec01.x*vec02.y - uvVec02.x*vec01.y)*den;
												//binormal.z = (uvVec01.x*vec02.z - uvVec02.x*vec01.z)*den;
	binormal = vec02*uvVec01.x - vec01*uvVec02.x;//↑3はこっちでもいい


	tangent = tangent.Normalize();
	binormal = binormal.Normalize();


	vertsForBuff[indices[idx]].tangent = tangent;
	vertsForBuff[indices[idx]].binormal = binormal;
	vertsForBuff[indices[idx + 1]].tangent = tangent;
	vertsForBuff[indices[idx + 1]].binormal = binormal;
	vertsForBuff[indices[idx + 2]].tangent = tangent;
	vertsForBuff[indices[idx + 2]].binormal = binormal;


}


SkySphere::~SkySphere()
{
}

void
SkySphere::Update()
{
	UpdateMatrixies();
}
void
SkySphere::Draw()
{
	DeviceDx11& dev = DeviceDx11::Instance();

	dev.Context()->PSSetSamplers(0, 1, &_samplerState_Wrap);

	ApplyCameraShaders();
	ApplyTextures();
	ApplyConstantBuffer(_matrixBuffer, _mappedMatrixies, _worldAndCamera);

	unsigned int stride = sizeof(float) * 14;
	unsigned int offset = 0;

	_worldAndCamera.cameraView = _cameraPtr.lock()->CameraView();
	_worldAndCamera.cameraProj = _cameraPtr.lock()->CameraProjection();
	_worldAndCamera.lightView = _cameraPtr.lock()->LightView();
	_worldAndCamera.lightProj = _cameraPtr.lock()->LightProjection();

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);


	dev.Context()->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	dev.Context()->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	dev.Context()->VSSetConstantBuffers(1, 1, &_materialBuffer);
	dev.Context()->Map(_materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mappedMaterial);
	memcpy(_mappedMaterial.pData, (void*)(&_material), sizeof(_material));
	dev.Context()->Unmap(_materialBuffer, 0);

	dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dev.Context()->DrawIndexed(_indicesCnt, 0, 0);
}
