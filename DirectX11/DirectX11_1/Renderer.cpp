#include "Renderer.h"
#include "DeviceDx11.h"
#include"Define.h"


Renderer::Renderer()
{
	

}
void
Renderer::Init()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	HRESULT result;

	//�o�b�N�o�b�t�@�̃����_�[�^�[�Q�b�g�r���[���쐬
	ID3D11Texture2D *pBack;
	//�o�b�N�o�b�t�@�̃T�[�t�F�C�X�������[�����\�[�X�i�e�N�X�`���j�Ƃ��Ĕ����o��
	dev.SwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBack);
	//���̃e�N�X�`���������_�[�^�[�Q�b�g�Ƃ���悤�ȃ����_�[�^�[�Q�b�g�r���[���쐬
	dev.Device()->CreateRenderTargetView(pBack, nullptr, &_mainRTV);
	pBack->Release();


	//�f�v�X�X�e���V���r���[�iDSV�j���쐬
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = (unsigned int)WINDOW_WIDTH;
	descDepth.Height = (unsigned int)WINDOW_HEIGHT;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	dev.Device()->CreateTexture2D(&descDepth, nullptr, &_mainDS);
	dev.Device()->CreateDepthStencilView(_mainDS, nullptr, &_mainDSV);

	_mainVP = {};
	_mainVP.Width = WINDOW_WIDTH;
	_mainVP.Height = WINDOW_HEIGHT;
	_mainVP.MinDepth = 0.0f;
	_mainVP.MaxDepth = 1.0f;
	_mainVP.TopLeftX = 0;
	_mainVP.TopLeftY = 0;
	dev.Context()->RSSetViewports(1, &_mainVP);

	//�����_�[�^�[�Q�b�g�r���[�ƃf�v�X�X�e���V���r���[���Z�b�g
	dev.Context()->OMSetRenderTargets(1, &_mainRTV, _mainDSV);

	_depthStencilDesc = {};
	_depthStencilDesc.DepthEnable = true;
	_depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	_depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;//�[�x�o�b�t�@�ւ̏������݂��I��

	dev.Device()->CreateDepthStencilState(&_depthStencilDesc, &_depthStencilState);
	dev.Context()->OMSetDepthStencilState(_depthStencilState, 0);

	//�[�x�o�b�t�@�֏������܂Ȃ��X�e���V���X�e�[�g�̍쐬
	_depthStencilDesc.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ZERO;
	dev.Device()->CreateDepthStencilState(&_depthStencilDesc, &_depthStencilState_ZWriteOff);

	//�[�x�o�b�t�@���̂��̂������ȃX�e���V���X�e�[�g�̍쐬
	_depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	_depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	_depthStencilDesc.DepthEnable = false;
	dev.Device()->CreateDepthStencilState(&_depthStencilDesc,&_depthStencilState_ZOff);

	//�A���t�@�L���ׂ̈̐ݒ�
	_blendState = nullptr;
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	D3D11_RENDER_TARGET_BLEND_DESC& blrtdesc = blendDesc.RenderTarget[0];
	blrtdesc.BlendEnable = true;
	blrtdesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blrtdesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blrtdesc.BlendOp = D3D11_BLEND_OP_ADD;
	blrtdesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	blrtdesc.DestBlendAlpha = D3D11_BLEND_ZERO;
	blrtdesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blrtdesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	dev.Device()->CreateBlendState(&blendDesc, &_blendState);
	dev.Context()->OMSetBlendState(_blendState, blendFactor, 0xffffffff);



	//���X�^���C�U�ݒ�
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = false;
	_rasterizerState_CullNone = nullptr;
	result = dev.Device()->CreateRasterizerState(&rasterizerDesc, &_rasterizerState_CullNone);

	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	_rasterizerState_CullBack = nullptr;
	result = dev.Device()->CreateRasterizerState(&rasterizerDesc, &_rasterizerState_CullBack);

	dev.Context()->RSSetState(_rasterizerState_CullNone);

#pragma region ���C�g�r���[
	//���C�g�r���[
	D3D11_TEXTURE2D_DESC rendertexDesc = {};
	rendertexDesc.Width = (unsigned int)WINDOW_WIDTH;
	rendertexDesc.Height = (unsigned int)WINDOW_HEIGHT;
	rendertexDesc.MipLevels = 1;
	rendertexDesc.ArraySize = 1;
	rendertexDesc.Format = DXGI_FORMAT_R32_FLOAT;//DXGI_FORMAT_R8G8B8A8_TYPELESS;//
	rendertexDesc.SampleDesc.Count = 1;
	rendertexDesc.SampleDesc.Quality = 0;
	rendertexDesc.Usage = D3D11_USAGE_DEFAULT;
	rendertexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	rendertexDesc.CPUAccessFlags = 0;
	rendertexDesc.MiscFlags = 0;
	ID3D11Texture2D* rtex = nullptr;
	dev.Device()->CreateTexture2D(&rendertexDesc, nullptr, &rtex);

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R32_FLOAT; //DXGI_FORMAT_R8G8B8A8_UNORM;//
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	result = dev.Device()->CreateRenderTargetView(rtex, &rtvDesc, &_lightRTV);
	D3D11_SHADER_RESOURCE_VIEW_DESC lvSrcDesc = {};
	lvSrcDesc.Format = rtvDesc.Format;
	lvSrcDesc.Texture2D.MipLevels = 1;
	lvSrcDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	result = dev.Device()->CreateShaderResourceView(rtex, &lvSrcDesc, &_shaderResourceViewForShadow);



	//���̃f�v�X�X�e���V���r���[���ǂ��ɂ�����
	D3D11_TEXTURE2D_DESC descLightDepth;
	descLightDepth.Width = (unsigned int)WINDOW_WIDTH;
	descLightDepth.Height = (unsigned int)WINDOW_HEIGHT;
	descLightDepth.MipLevels = 1;
	descLightDepth.ArraySize = 1;
	descLightDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descLightDepth.SampleDesc.Count = 1;
	descLightDepth.SampleDesc.Quality = 0;
	descLightDepth.Usage = D3D11_USAGE_DEFAULT;
	descLightDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descLightDepth.CPUAccessFlags = 0;
	descLightDepth.MiscFlags = 0;

	ID3D11Texture2D* temp;
	dev.Device()->CreateTexture2D(&descLightDepth, nullptr, &temp);
	dev.Device()->CreateDepthStencilView(temp, nullptr, &_lightDSV);//dev.Device()->CreateDepthStencilView(rtex, nullptr, &_lightDSV);
#pragma endregion




#pragma region ���C�g�r���[�i�J���[
	//���C�g�r���[
	rendertexDesc = {};
	rendertexDesc.Width = (unsigned int)WINDOW_WIDTH;
	rendertexDesc.Height = (unsigned int)WINDOW_HEIGHT;
	rendertexDesc.MipLevels = 1;
	rendertexDesc.ArraySize = 1;
	rendertexDesc.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;//DXGI_FORMAT_R16G16B16A16_FLOAT;//
	rendertexDesc.SampleDesc.Count = 1;
	rendertexDesc.SampleDesc.Quality = 0;
	rendertexDesc.Usage = D3D11_USAGE_DEFAULT;
	rendertexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	rendertexDesc.CPUAccessFlags = 0;
	rendertexDesc.MiscFlags = 0;
	rtex = nullptr;
	dev.Device()->CreateTexture2D(&rendertexDesc, nullptr, &rtex);

	rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//DXGI_FORMAT_R16G16B16A16_FLOAT; //
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	result = dev.Device()->CreateRenderTargetView(rtex, &rtvDesc, &_lightRTV_color);
	lvSrcDesc = {};
	lvSrcDesc.Format = rtvDesc.Format;
	lvSrcDesc.Texture2D.MipLevels = 1;
	lvSrcDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	result = dev.Device()->CreateShaderResourceView(rtex, &lvSrcDesc, &_shaderResourceView_color);



	//���̃f�v�X�X�e���V���r���[���ǂ��ɂ�����
	descLightDepth = {};
	descLightDepth.Width = (unsigned int)WINDOW_WIDTH;
	descLightDepth.Height = (unsigned int)WINDOW_HEIGHT;
	descLightDepth.MipLevels = 1;
	descLightDepth.ArraySize = 1;
	descLightDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descLightDepth.SampleDesc.Count = 1;
	descLightDepth.SampleDesc.Quality = 0;
	descLightDepth.Usage = D3D11_USAGE_DEFAULT;
	descLightDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descLightDepth.CPUAccessFlags = 0;
	descLightDepth.MiscFlags = 0;

	
	dev.Device()->CreateTexture2D(&descLightDepth, nullptr, &temp);
	dev.Device()->CreateDepthStencilView(temp, nullptr, &_lightDSV_color);//dev.Device()->CreateDepthStencilView(rtex, nullptr, &_lightDSV);
#pragma endregion

#pragma region �J��������̃f�v�X
	//���C�g�r���[
	rendertexDesc = {};
	rendertexDesc.Width = (unsigned int)WINDOW_WIDTH;
	rendertexDesc.Height = (unsigned int)WINDOW_HEIGHT;
	rendertexDesc.MipLevels = 1;
	rendertexDesc.ArraySize = 1;
	rendertexDesc.Format = DXGI_FORMAT_R32_FLOAT; //DXGI_FORMAT_R8G8B8A8_TYPELESS;//DXGI_FORMAT_R16G16B16A16_FLOAT;//
	rendertexDesc.SampleDesc.Count = 1;
	rendertexDesc.SampleDesc.Quality = 0;
	rendertexDesc.Usage = D3D11_USAGE_DEFAULT;
	rendertexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	rendertexDesc.CPUAccessFlags = 0;
	rendertexDesc.MiscFlags = 0;
	rtex = nullptr;
	result = dev.Device()->CreateTexture2D(&rendertexDesc, nullptr, &rtex);

	rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;// DXGI_FORMAT_R8G8B8A8_UNORM;//DXGI_FORMAT_R16G16B16A16_FLOAT; //
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	result = dev.Device()->CreateRenderTargetView(rtex, &rtvDesc, &_cameraZ_RTV);
	lvSrcDesc = {};
	lvSrcDesc.Format = rtvDesc.Format;
	lvSrcDesc.Texture2D.MipLevels = 1;
	lvSrcDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	result = dev.Device()->CreateShaderResourceView(rtex, &lvSrcDesc, &_shaderResourceView_CameraDepth);



	//���̃f�v�X�X�e���V���r���[���ǂ��ɂ�����
	descLightDepth = {};
	descLightDepth.Width = (unsigned int)WINDOW_WIDTH;
	descLightDepth.Height = (unsigned int)WINDOW_HEIGHT;
	descLightDepth.MipLevels = 1;
	descLightDepth.ArraySize = 1;
	descLightDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descLightDepth.SampleDesc.Count = 1;
	descLightDepth.SampleDesc.Quality = 0;
	descLightDepth.Usage = D3D11_USAGE_DEFAULT;
	descLightDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descLightDepth.CPUAccessFlags = 0;
	descLightDepth.MiscFlags = 0;


	dev.Device()->CreateTexture2D(&descLightDepth, nullptr, &temp);
	dev.Device()->CreateDepthStencilView(temp, nullptr, &_cameraZ_DSV);//dev.Device()->CreateDepthStencilView(rtex, nullptr, &_lightDSV);
#pragma endregion

}


Renderer::~Renderer()
{
}


void 
Renderer::ChangeRT_Camera()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	float color[4] = { 0.5, 0.5, 0.5, 1 };
	dev.Context()->ClearRenderTargetView(_mainRTV, color);
	dev.Context()->ClearDepthStencilView(_mainDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	dev.Context()->OMSetRenderTargets(1, &_mainRTV, _mainDSV);
}
void
Renderer::ChangeRT_CameraDepth()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	float color[4] = { 0.5, 0.5, 0.5, 1 };
	dev.Context()->ClearRenderTargetView(_cameraZ_RTV, color);
	dev.Context()->ClearDepthStencilView(_cameraZ_DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	dev.Context()->OMSetRenderTargets(1, &_cameraZ_RTV, _cameraZ_DSV);
}

void
Renderer::ChangeRT_Light()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	float color[4] = { 0, 0, 0, 0 };
	dev.Context()->ClearRenderTargetView(_lightRTV, color);
	dev.Context()->ClearDepthStencilView(_lightDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	dev.Context()->OMSetRenderTargets(1, &_lightRTV, _lightDSV);
}
void
Renderer::ChangeRT_LightColor()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	float color[4] = { 0.5, 0.5, 0.5, 1 };
	dev.Context()->ClearRenderTargetView(_lightRTV_color, color);
	dev.Context()->ClearDepthStencilView(_lightDSV_color,D3D11_CLEAR_DEPTH, 1.0f, 0);
	dev.Context()->OMSetRenderTargets(1, &_lightRTV_color, _lightDSV_color);
}

void 
Renderer::ChangePTForPMD()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void 
Renderer::ChangePTForPrimitive()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void
Renderer::CullNone()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	dev.Context()->RSSetState(_rasterizerState_CullNone);
}
void
Renderer::CullBack()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	dev.Context()->RSSetState(_rasterizerState_CullBack);
}
void
Renderer::ZWriteOn()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	dev.Context()->OMSetDepthStencilState(_depthStencilState, 0);
}
void 
Renderer::ZWriteOff()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	dev.Context()->OMSetDepthStencilState(_depthStencilState_ZWriteOff, 0);
}

void
Renderer::DepthDisable()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	dev.Context()->OMSetDepthStencilState(_depthStencilState_ZOff, 0);
}