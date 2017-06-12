#include "DecalFactory.h"
#include"DecalBox.h"
#include"Camera.h"
#include"DeviceDx11.h"
#include"ShaderGenerator.h"

const int MAX_DECAL_NUM = 200;

DecalFactory::DecalFactory(Camera* cameraPtr):_cameraPtr(cameraPtr)
{
	HRESULT result = S_OK;
	DeviceDx11& dev = DeviceDx11::Instance();
	_decalBoxes.reserve(MAX_DECAL_NUM);
	ID3D11ShaderResourceView* temp;
	D3DX11CreateShaderResourceViewFromFile(dev.Device(), "texture/bloodhand.png", nullptr, nullptr,
		&temp, &result);

	_decalTex = std::make_shared<ID3D11ShaderResourceView*>(temp);

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
	ShaderGenerator::CreateVertexShader("Decal.hlsl", "DecalBoxVS", "vs_5_0",
		_vertexShader, inputElementDescs, sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC), _inputlayout);
	ShaderGenerator::CreatePixelShader("Decal.hlsl", "DecalBoxPS", "ps_5_0", _pixelShader);

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
		DecalBox* d = new DecalBox(pos, rot, scale, _cameraPtr, _decalTex,
			_vertexShader, _pixelShader, _inputlayout,
			_vertexBuffer, _indexBuffer, _indicesCnt);
		_decalBoxes.push_back(d);
	}
}

void
DecalFactory::Update()
{
	for (auto& d : _decalBoxes)
	{
		d->Update();
	}
}

void 
DecalFactory::Draw()
{
	for (auto& d : _decalBoxes)
	{
		d->Draw();
	}
}