#pragma once
#include"Primitive.h"
#include"Define.h"
class Camera;

struct ID3D11Buffer;

class DecalPlane
{
private:
	Camera* _cameraPtr;
	//MATRIXIES _mvp;
	MatrixiesForDecal _matrixies;
	XMMATRIX _modelMatrix;
	ID3D11Buffer* _matrixBuffer;
	D3D11_MAPPED_SUBRESOURCE _mappedMatrixies;


	ID3D11VertexShader* _vertexShader;
	ID3D11InputLayout* _inputlayout;
	ID3D11PixelShader* _pixelShader;
	ID3D11Buffer* _vertexBuffer;

	ID3D11ShaderResourceView* _texture;

	float _rot;
	XMFLOAT3 _pos;
public:
	struct DecalPlaneVertex
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
	};
	DecalPlane(Camera*);
	~DecalPlane();
	DecalPlane(float width, float height, XMFLOAT3 normal, Camera* camera);

	ID3D11Buffer* VertexBuffer(){ return _vertexBuffer; };


	void Update();

	void Draw();

	XMVECTOR Normal(){ return XMLoadFloat3(new XMFLOAT3(0, 1, 0)); };

	void SetPos(XMFLOAT3 pos);

};

