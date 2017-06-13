#pragma once
#include<D3D11.h>
#include"Camera.h"
#include"Define.h"
#include<memory>
class DecalBox
{

private:
	XMFLOAT3 _pos;
	XMFLOAT3 _rot;
	XMFLOAT3 _scale;

	ID3D11VertexShader* _vertexShader;
	ID3D11InputLayout* _inputlayout;
	ID3D11PixelShader* _pixelShader;

	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* _indexBuffer;
	unsigned int _indicesCnt;

	Camera* _cameraPtr;
	MatrixiesForDecal _matrixies;
	XMMATRIX _modelMatrix;
	ID3D11Buffer* _matrixBuffer;
	D3D11_MAPPED_SUBRESOURCE _mappedMatrixies;

	ID3D11SamplerState* _samplerState;

	ID3D11ShaderResourceView* _decalTexture;
	std::shared_ptr<ID3D11ShaderResourceView*> _decalTex;

public:
	DecalBox(float width,float height,float length,Camera* cameraPtr);
	DecalBox(const XMFLOAT3& pos, const XMFLOAT3& rot,const XMFLOAT3& scale,
		Camera* cameraPtr,std::shared_ptr<ID3D11ShaderResourceView*> texPtr,
		ID3D11VertexShader* vs,ID3D11PixelShader* ps,ID3D11InputLayout* lauout,
		ID3D11Buffer* vertBuff,ID3D11Buffer* indexBuff,unsigned int indicesCnt);
	~DecalBox();


	ID3D11Buffer* VertexBuffer(){ return _vertexBuffer; };
	void Update();

	void Draw();
	void DebugDraw();

	void SetPos(const XMFLOAT3 pos) { _pos = pos; };
	void SetPYR(const XMFLOAT3 pitchYawRoll) { _rot = pitchYawRoll; };
	void SetScale(const XMFLOAT3 scale) { _scale = scale; };

	const XMFLOAT3 GetRotation() { return _rot; };
	const XMFLOAT3 GetScale() { return _scale; };
	const XMFLOAT3 GetPos() { return _pos; };
};

