#pragma once
#include"Primitive.h"
#include"Define.h"
#include<vector>
class Camera;
struct ID3D11Buffer;

class SkySphere
	:public Primitive
{
private:
	XMFLOAT3 _pos;
	BYTE _keystate[256];
	BYTE _lastkeystate[256];
	int moveForward;
	int moveRight;

	Camera* _cameraPtr;
	WorldAndCamera _worldAndCamera;
	XMMATRIX _modelMatrix;
	ID3D11Buffer* _matrixBuffer;
	D3D11_MAPPED_SUBRESOURCE _mappedMatrixies;
	D3D11_MAPPED_SUBRESOURCE _mappedMaterial;

	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* _indexBuffer;
	ID3D11Buffer* _materialBuffer;
	float rot;

	unsigned int _indicesCnt;
	unsigned int _verticesCnt;

	Material _material;

	ID3D11SamplerState* _samplerState_Wrap;
	//ID3D11SamplerState* _samplerState_Clamp;

	struct TempVertex
	{
		Vector3 pos;
		Vector2 uv;
		Vector3 normal;
	};
	void CalculateTangentBinormal
	(TempVertex*, TempVertex*, TempVertex*, std::vector<PrimitiveVertex>&, const std::vector<unsigned short>& indices, int idx);
	/*void CalculateTangent(TempVertex * v0, TempVertex* v1, TempVertex* v2,
		std::vector<PrimitiveVertex>& vertsForBuff, const std::vector<unsigned short>& indices, int idx);*/

	ID3D11ShaderResourceView* _texture;
public:
	SkySphere(unsigned int divNum, float radius,Camera* cam);
	~SkySphere();

	ID3D11Buffer* VertexBuffer() { return _vertexBuffer; };

	ID3D11VertexShader* _vertexShader;
	ID3D11InputLayout* _inputlayout;
	ID3D11PixelShader* _pixelShader;

	

	XMFLOAT3 GetPos() { return _pos; };
	void SetPos(const XMFLOAT3& pos) { _pos = pos; };

	void Update();

	void Draw();
};

