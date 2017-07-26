#pragma once
#pragma once
#include"Primitive.h"
#include"Primitive.h"
#include"Define.h"
#include<memory>
class Camera;

struct ID3D11Buffer;

class TessPlane
	:public Primitive
{
private:
	

	ID3D11Buffer* _vertexBuffer;

	ID3D11SamplerState* _samplerState_Wrap;
public:
	TessPlane(const std::shared_ptr<Camera> cam);
	~TessPlane();
	TessPlane(float width, float height, Vector3 normal, const std::shared_ptr<Camera> camera);

	ID3D11Buffer* VertexBuffer(){ return _vertexBuffer; };



	//std::weak_ptr<ID3D11PixelShader*> _cameraDepthPS;

	void Update();

	void Draw();
	void DrawLightView();
	void DrawCameraDepth();

};