#pragma once
#include"Primitive.h"
#include"Define.h"
#include"Camera.h"

struct ID3D11Buffer;

class Plane
	:public Primitive
{
private:
	Camera& _cameraRef;
	//MATRIXIES _mvp;
	WorldAndCamera _worldAndCamera;
	XMMATRIX _modelMatrix;
	ID3D11Buffer* _matrixBuffer;
	D3D11_MAPPED_SUBRESOURCE _mappedMatrixies;

	float rot;
public:
	Plane(Camera&);
	~Plane();
	Plane(float width, float height, Vector3 normal,Camera& camera);
	
	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* VertexBuffer(){ return _vertexBuffer; };

	ID3D11VertexShader* _vertexShader;
	ID3D11InputLayout* _inputlayout;
	ID3D11PixelShader* _pixelShader;

	ID3D11VertexShader* _lightviewVS;
	ID3D11InputLayout* _lightviewInputLayout;
	ID3D11PixelShader* _lightviewPS;

	void Update();

	void Draw();
	void DrawLightView();
	void DrawCameraDepth();

	XMVECTOR Normal(){ return XMLoadFloat3(new XMFLOAT3(0, 1, 0)); };

};

