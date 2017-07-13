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
	

	ID3D11SamplerState* _samplerState;

public:
	Plane(const std::shared_ptr<Camera> cam);
	~Plane();
	Plane(float width, float height, Vector3 normal,
		const std::shared_ptr<Camera> camera);
	
	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* VertexBuffer(){ return _vertexBuffer; };

	void Update();

	void Draw();
	void DrawLightView();
	void DrawCameraDepth();

	XMVECTOR Normal(){ return XMLoadFloat3(new XMFLOAT3(0, 1, 0)); };

};

