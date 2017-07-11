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
	std::weak_ptr<Camera> _cameraPtr;
	//MATRIXIES _mvp;
	WorldAndCamera _worldAndCamera;
	XMMATRIX _modelMatrix;
	ID3D11Buffer* _matrixBuffer;
	D3D11_MAPPED_SUBRESOURCE _mappedMatrixies;

	float rot;

	std::weak_ptr<ID3D11ShaderResourceView*> _mainTex;
	std::weak_ptr<ID3D11ShaderResourceView*> _subTex;
	std::weak_ptr<ID3D11ShaderResourceView*> _subTex2;
	std::weak_ptr<ID3D11ShaderResourceView*> _displacementTex;

	ID3D11SamplerState* _samplerState_Wrap;
public:
	TessPlane(const std::shared_ptr<Camera>& cam);
	~TessPlane();
	TessPlane(float width, float height, Vector3 normal, const std::shared_ptr<Camera>& camera);

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