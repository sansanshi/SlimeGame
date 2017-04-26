#pragma once
#include<D3D11.h>
#include"Camera.h"
#include"Define.h"
class DecalBox
{

private:
	XMFLOAT3 _pos;

	ID3D11VertexShader* _vertexShader;
	ID3D11InputLayout* _inputlayout;
	ID3D11PixelShader* _pixelShader;

	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* _indexBuffer;
	unsigned int _indicesCnt;

	Camera& _cameraRef;
	MatrixiesForDecal _matrixies;
	XMMATRIX _modelMatrix;
	ID3D11Buffer* _matrixBuffer;
	D3D11_MAPPED_SUBRESOURCE _mappedMatrixies;

	ID3D11SamplerState* _samplerState;

	ID3D11ShaderResourceView* _decalTexture;

public:
	DecalBox(float width,float height,float length,Camera& cameraRef);
	~DecalBox();


	ID3D11Buffer* VertexBuffer(){ return _vertexBuffer; };
	void Update();

	void Draw();

	void SetPos(XMFLOAT3 pos);
};

