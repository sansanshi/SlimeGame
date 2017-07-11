#pragma once
#include"Primitive.h"
#include<D3D11.h>
#include<memory>
class Camera;
class Cylinder:public Primitive
{
private:
	ID3D11Buffer* _vertexBuffer;//円柱本体
	ID3D11InputLayout* _inputlayout;

	ID3D11Buffer* _hatchBuffer;//蓋
	unsigned int _hatchVertCnt;


	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;
	std::weak_ptr<ID3D11ShaderResourceView*> _mainTex;
	std::weak_ptr<ID3D11ShaderResourceView*> _subTex;
	std::weak_ptr<ID3D11ShaderResourceView*> _normalTex;

	ID3D11VertexShader* _lightviewVS;
	ID3D11PixelShader* _lightviewPS;
	ID3D11InputLayout* _lightviewInputLayout;

	unsigned int _vertexCnt;
	std::weak_ptr<Camera> _cameraPtr;

	//MATRIXIES _mvp;
	WorldAndCamera _worldAndCamera;
	XMMATRIX _modelMatrix;

	ID3D11Buffer* _matrixBuffer;
	D3D11_MAPPED_SUBRESOURCE _mappedMatrixies;

	float angle;//とりあえず回転させる用
	float _radius;
	float _height;
	unsigned int _div;

	XMMATRIX _camView;
	XMMATRIX _camProj;
	XMMATRIX _lightView;
	XMMATRIX _lightProj;

	void CalculateTangentBinormal();

public:
	Cylinder(float radius,float height,unsigned int div,const std::shared_ptr<Camera>& camera);
	Cylinder(const std::shared_ptr<Camera>& camera);
	~Cylinder();

	ID3D11Buffer* VertexBuffer(){ return _vertexBuffer; };

	void Draw();
	void DrawLightView();
	void DrawCameraDepth();

	void Update();

};

