#pragma once
#include<D3D11.h>
#include"Define.h"
#include<memory>
#include"Primitive.h"
class Camera;
class DecalBox : public Primitive
{

private:
	

	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* _indexBuffer;
	unsigned int _indicesCnt;

	std::weak_ptr<Camera> _cameraPtr;
	MatrixiesForDecal _matrixies;
	XMMATRIX _modelMatrix;
	ID3D11Buffer* _matrixBuffer;
	D3D11_MAPPED_SUBRESOURCE _mappedMatrixies;

	ID3D11SamplerState* _samplerState;

	std::weak_ptr<ID3D11ShaderResourceView*> _decalTexture;


public:
	DecalBox(float width,float height,float length,const std::shared_ptr<Camera>& cameraPtr);
	DecalBox(const XMFLOAT3& pos, const XMFLOAT3& rot,const XMFLOAT3& scale,
		const std::shared_ptr<Camera>& cameraPtr,std::shared_ptr<ID3D11ShaderResourceView*> texPtr,
		std::weak_ptr<ID3D11VertexShader*> vs,std::weak_ptr<ID3D11PixelShader*> ps,std::weak_ptr<ID3D11InputLayout*> lauout,
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

	const XMMATRIX& GetWorldMatrix() { return _modelMatrix; };

	void ChangeScale(float value);
};

