#pragma once
#include"D3D11.h"


class Renderer
{
private:
	ID3D11RenderTargetView* _mainRTV;
	ID3D11Texture2D* _mainDS;//デプスステンシル用テクスチャ
	ID3D11DepthStencilView* _mainDSV;
	D3D11_VIEWPORT _mainVP;//ビューポート

	ID3D11DepthStencilState* _depthStencilState;
	ID3D11DepthStencilState* _depthStencilState_ZWriteOff;//Z
	ID3D11DepthStencilState* _depthStencilState_ZOff;
	D3D11_DEPTH_STENCIL_DESC _depthStencilDesc;
	D3D11_DEPTH_STENCILOP_DESC _depthStencilOpDesc_Front;
	D3D11_DEPTH_STENCILOP_DESC _depthStencilOpDesc_Back;
	

	ID3D11RenderTargetView* _lightRTV;
	ID3D11DepthStencilView* _lightDSV;

	//ポストエフェクト用のRTV
	ID3D11RenderTargetView* _colorRTV;
	ID3D11DepthStencilView* _colorDSV;


	//カメラからの深度テクスチャ生成用
	ID3D11RenderTargetView* _cameraZ_RTV;
	ID3D11DepthStencilView* _cameraZ_DSV;


	ID3D11BlendState* _blendState;

	ID3D11RasterizerState* _rasterizerState_CullNone;
	ID3D11RasterizerState* _rasterizerState_CullBack;

	ID3D11ShaderResourceView* _shaderResourceViewForShadow;//ライトからのデプス
	ID3D11ShaderResourceView* _shaderResourceView_PostEffect;//スライムの影（半透明）用
	ID3D11ShaderResourceView* _shaderResourceView_CameraDepth;

public:
	Renderer();
	~Renderer();

	void Init();

	//レンダーターゲットをライトビュー、カメラビューに切り替え
	void ChangeRT_Camera();
	void ChangeRT_Light();
	void ChangeRT_PostEffect();//カメラからの視点（カラー
	void ChangeRT_CameraDepth();

	//プリミティブトポロジの切り替え
	//PMD用
	void ChangePTForPMD();
	//基本形用
	void ChangePTForPrimitive();

	//ライトからのレンダリング結果をテクスチャとして保存しているリソースを返す
	ID3D11ShaderResourceView* LightDepthShaderResource(){ return _shaderResourceViewForShadow; };


	ID3D11ShaderResourceView* TestShaderResource(){ return _shaderResourceView_PostEffect; };

	//カメラからの深度を書き込んだテクスチャを返す
	ID3D11ShaderResourceView* CameraDepthShaderResource(){ return _shaderResourceView_CameraDepth; };

	void CullNone();

	void CullBack();


	void ZWriteOn();
	void ZWriteOff();

	void DepthEnable();//深度バッファ有効
	void DepthDisable();//深度バッファ無効

};

