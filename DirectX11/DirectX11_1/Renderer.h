#pragma once
#include"D3D11.h"


class Renderer
{
private:
	ID3D11RenderTargetView* _mainRTV;
	ID3D11Texture2D* _mainDS;//�f�v�X�X�e���V���p�e�N�X�`��
	ID3D11DepthStencilView* _mainDSV;
	D3D11_VIEWPORT _mainVP;//�r���[�|�[�g

	ID3D11DepthStencilState* _depthStencilState;
	ID3D11DepthStencilState* _depthStencilState_ZWriteOff;//Z
	ID3D11DepthStencilState* _depthStencilState_ZOff;
	D3D11_DEPTH_STENCIL_DESC _depthStencilDesc;
	D3D11_DEPTH_STENCILOP_DESC _depthStencilOpDesc_Front;
	D3D11_DEPTH_STENCILOP_DESC _depthStencilOpDesc_Back;
	

	ID3D11RenderTargetView* _lightRTV;
	ID3D11DepthStencilView* _lightDSV;

	//�|�X�g�G�t�F�N�g�p��RTV
	ID3D11RenderTargetView* _colorRTV;
	ID3D11DepthStencilView* _colorDSV;


	//�J��������̐[�x�e�N�X�`�������p
	ID3D11RenderTargetView* _cameraZ_RTV;
	ID3D11DepthStencilView* _cameraZ_DSV;


	ID3D11BlendState* _blendState;

	ID3D11RasterizerState* _rasterizerState_CullNone;
	ID3D11RasterizerState* _rasterizerState_CullBack;

	ID3D11ShaderResourceView* _shaderResourceViewForShadow;//���C�g����̃f�v�X
	ID3D11ShaderResourceView* _shaderResourceView_PostEffect;//�X���C���̉e�i�������j�p
	ID3D11ShaderResourceView* _shaderResourceView_CameraDepth;

public:
	Renderer();
	~Renderer();

	void Init();

	//�����_�[�^�[�Q�b�g�����C�g�r���[�A�J�����r���[�ɐ؂�ւ�
	void ChangeRT_Camera();
	void ChangeRT_Light();
	void ChangeRT_PostEffect();//�J��������̎��_�i�J���[
	void ChangeRT_CameraDepth();

	//�v���~�e�B�u�g�|���W�̐؂�ւ�
	//PMD�p
	void ChangePTForPMD();
	//��{�`�p
	void ChangePTForPrimitive();

	//���C�g����̃����_�����O���ʂ��e�N�X�`���Ƃ��ĕۑ����Ă��郊�\�[�X��Ԃ�
	ID3D11ShaderResourceView* LightDepthShaderResource(){ return _shaderResourceViewForShadow; };


	ID3D11ShaderResourceView* TestShaderResource(){ return _shaderResourceView_PostEffect; };

	//�J��������̐[�x���������񂾃e�N�X�`����Ԃ�
	ID3D11ShaderResourceView* CameraDepthShaderResource(){ return _shaderResourceView_CameraDepth; };

	void CullNone();

	void CullBack();


	void ZWriteOn();
	void ZWriteOff();

	void DepthEnable();//�[�x�o�b�t�@�L��
	void DepthDisable();//�[�x�o�b�t�@����

};

