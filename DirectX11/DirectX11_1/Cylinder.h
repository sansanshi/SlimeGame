#pragma once
#include"Primitive.h"
#include<D3D11.h>
#include<memory>
class Camera;
class Cylinder:public Primitive
{
private:
	ID3D11Buffer* _vertexBuffer;//�~���{��

	ID3D11Buffer* _hatchBuffer;//�W
	unsigned int _hatchVertCnt;


	unsigned int _vertexCnt;
	

	float angle;//�Ƃ肠������]������p
	float _radius;
	float _height;
	unsigned int _div;

	XMMATRIX _camView;
	XMMATRIX _camProj;
	XMMATRIX _lightView;
	XMMATRIX _lightProj;

	void CalculateTangentBinormal();

public:
	Cylinder(float radius,float height,unsigned int div,const std::shared_ptr<Camera> camera);
	Cylinder(const std::shared_ptr<Camera> camera);
	~Cylinder();

	ID3D11Buffer* VertexBuffer(){ return _vertexBuffer; };


	void Draw();
	void DrawLightView();
	void DrawCameraDepth();

	void Update();

};

