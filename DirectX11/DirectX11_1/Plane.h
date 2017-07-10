#pragma once
#include"Primitive.h"
#include"Define.h"
#include"Camera.h"
#include<memory>
#include"ResourceManager.h"

struct ID3D11Buffer;

class Plane
	:public Primitive
{
private:
	std::weak_ptr<Camera> _cameraPtr;
	WorldAndCamera _worldAndCamera;
	XMMATRIX _modelMatrix;
	ID3D11Buffer* _matrixBuffer;
	D3D11_MAPPED_SUBRESOURCE _mappedMatrixies;

	std::weak_ptr<ID3D11ShaderResourceView*> _mainTex;
	std::weak_ptr<ID3D11ShaderResourceView*> _subTex;
	std::weak_ptr<ID3D11ShaderResourceView*> _normalTex;
	std::weak_ptr<ID3D11ShaderResourceView*> _flowTex;
	ID3D11SamplerState* _samplerState;

	float rot;
public:
	Plane(const std::shared_ptr<Camera> cam);
	~Plane();
	Plane(float width, float height, Vector3 normal,
		const std::shared_ptr<Camera>& camera);
	
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

