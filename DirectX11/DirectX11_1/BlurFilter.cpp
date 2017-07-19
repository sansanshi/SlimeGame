#include "BlurFilter.h"
#include "DeviceDx11.h"
#include<vector>
#include"Camera.h"
#include"ShaderDefine.h"
#include"ResourceManager.h"


BlurFilter::BlurFilter() 
{
	DeviceDx11& dev = DeviceDx11::Instance();
	HRESULT result = S_OK;
	ResourceManager& resourceMgr = ResourceManager::Instance();

	//�r���{�[�h�e�X�g
	_vertexBuffer = nullptr;
	_vertexBuffer = CreateBlurFilterVertexBuffer();
	_texture = resourceMgr.LoadSRV("BlurFilter_main", "uvCheck_transparent.png");

	D3D11_INPUT_ELEMENT_DESC inputElementDescs[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	resourceMgr.LoadVS("BlurXFilter_VS",
		"blur.hlsl",
		"BlurXFilterVS",
		"vs_5_0",
		_blurXvs,
		inputElementDescs,
		sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		_inputLayout);
	resourceMgr.LoadPS("BlurXFilter_PS",
		"blur.hlsl",
		"BlurXFilterPS",
		"ps_5_0",
		_blurXps);

	resourceMgr.LoadVS("BlurYFilter_VS",
		"blur.hlsl",
		"BlurYFilterVS",
		"vs_5_0",
		_blurYvs,
		inputElementDescs,
		sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		_inputLayout);
	resourceMgr.LoadPS("BlurXFilter_PS",
		"blur.hlsl",
		"BlurYFilterPS",
		"ps_5_0",
		_blurYps);


	D3D11_BUFFER_DESC matBuffDesc = {};
	matBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matBuffDesc.ByteWidth = sizeof(WorldAndCamera);
	matBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;//�o�b�t�@�̒��g��CPU�ŏ���������
	matBuffDesc.Usage = D3D11_USAGE_DYNAMIC;//CPU�ɂ�鏑�����݁AGPU�ɂ��ǂݍ��݂��s����Ƃ����Ӗ�
											//matBuffDesc.ByteWidth = sizeof(XMMATRIX);
											//matBuffDesc.StructureByteStride = sizeof(XMMATRIX);

	D3D11_SUBRESOURCE_DATA d;
	d.pSysMem = &_worldAndCamera;

	result = dev.Device()->CreateBuffer(&matBuffDesc, &d, &_matrixBuffer);

	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
	//�����ł��̃������̉�ɁA�}�g���b�N�X�̒l���R�s�[���Ă��
	memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));

	dev.Context()->Unmap(_matrixBuffer, 0);

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);


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

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);

	dev.Context()->PSSetShaderResources(TEXTURE_MAIN, 1, _texture._Get());
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

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);

	dev.Context()->PSSetShaderResources(TEXTURE_MAIN, 1, _texture._Get());
	unsigned int hudstride = sizeof(HUDVertex);
	unsigned int hudoffset = 0;
	dev.Context()->IASetVertexBuffers(0, 1, &_vertexBuffer, &hudstride, &hudoffset);

	dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	dev.Context()->Draw(4, 0);
}


void
BlurFilter::Update()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	//�r���{�[�h�\��

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);
	

}


//�r���{�[�h�̒��_�o�b�t�@
//@param �r���{�[�h���SX���W(3D���W�n
//@param�@�r���{�[�h���SY���W
//@param �r���{�[�h���i3D���W�n
//@param �r���{�[�h����(3D���W�n
//@param note ���ƍ����͒��S����̍L���� �����x-width/2,y+height/2
ID3D11Buffer*
BlurFilter::CreateBlurFilterVertexBuffer()
{
	DeviceDx11& dev = DeviceDx11::Instance();

	std::vector<HUDVertex> vertices(4);
	{
		vertices[0] = { XMFLOAT3(-1, 1, 0), XMFLOAT2(0.f, 0.f) };//����
		vertices[1] = { XMFLOAT3(1 , 1 , 0), XMFLOAT2(1.0f, 0.0f) };//�E��
		vertices[2] = { XMFLOAT3(-1, -1, 0), XMFLOAT2(0.f, 1.f) };//����
		vertices[3] = { XMFLOAT3(1, -1, 0), XMFLOAT2(1.f, 1.f) };//�E��
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


