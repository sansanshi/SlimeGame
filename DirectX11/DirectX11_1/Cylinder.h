#pragma once
#include"Primitive.h"
#include<D3D11.h>
#include"Camera.h"
class Cylinder:public Primitive
{
private:
	ID3D11Buffer* _vertexBuffer;//‰~’Œ–{‘Ì
	ID3D11InputLayout* _inputlayout;

	ID3D11Buffer* _hatchBuffer;//ŠW
	unsigned int _hatchVertCnt;


	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;

	ID3D11VertexShader* _lightviewVS;
	ID3D11PixelShader* _lightviewPS;
	ID3D11InputLayout* _lightviewInputLayout;

	unsigned int _vertexCnt;
	Camera& _cameraRef;

	//MATRIXIES _mvp;
	WorldAndCamera _worldAndCamera;
	XMMATRIX _modelMatrix;

	ID3D11Buffer* _matrixBuffer;
	D3D11_MAPPED_SUBRESOURCE _mappedMatrixies;

	float angle;//‚Æ‚è‚ ‚¦‚¸‰ñ“]‚³‚¹‚é—p
	float _radius;
	float _height;
	unsigned int _div;

	void CalculateTangentBinormal();

public:
	Cylinder(float radius,float height,unsigned int div,Camera& camera);
	Cylinder(Camera& camera);
	~Cylinder();

	ID3D11Buffer* VertexBuffer(){ return _vertexBuffer; };

	void Draw();
	void DrawLightView();
	void DrawCameraDepth();

	void Update();

	XMMATRIX testView;
	XMMATRIX testProj;
	int frame;
};

