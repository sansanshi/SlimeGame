#pragma once
#pragma once
#include"Primitive.h"
#include"Primitive.h"
#include"Define.h"
class Camera;

struct ID3D11Buffer;

class TessPlane
	:public Primitive
{
private:
	Camera* _cameraPtr;
	//MATRIXIES _mvp;
	WorldAndCamera _worldAndCamera;
	XMMATRIX _modelMatrix;
	ID3D11Buffer* _matrixBuffer;
	D3D11_MAPPED_SUBRESOURCE _mappedMatrixies;

	float rot;

	ID3D11ShaderResourceView* _groundTex;
	ID3D11ShaderResourceView* _displacementTex;

	ID3D11SamplerState* _samplerState_Wrap;
public:
	TessPlane(Camera*);
	~TessPlane();
	TessPlane(float width, float height, Vector3 normal, Camera* camera);

	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* VertexBuffer(){ return _vertexBuffer; };

	ID3D11VertexShader* _vertexShader;
	ID3D11InputLayout* _inputlayout;
	ID3D11PixelShader* _pixelShader;
	ID3D11HullShader* _hullShader;
	ID3D11DomainShader* _domainShader;

	ID3D11VertexShader* _lightviewVS;
	ID3D11InputLayout* _lightviewInputLayout;
	ID3D11PixelShader* _lightviewPS;

	ID3D11PixelShader* _cameraDepthPS;

	void Update();

	void Draw();
	void DrawLightView();
	void DrawCameraDepth();

};