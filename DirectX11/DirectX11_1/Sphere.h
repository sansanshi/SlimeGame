#pragma once
#include"Primitive.h"
#include"Define.h"
#include<vector>
#include<memory>
class Camera;

struct ID3D11Buffer;

class Sphere
	:public Primitive
{
private:
	Vector3 pos;
	BYTE _keystate[256];
	BYTE _lastkeystate[256];
	int moveForward;
	int moveRight;

	std::weak_ptr<Camera> _cameraPtr;
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
	void CalculateTangent(TempVertex * v0, TempVertex* v1, TempVertex* v2,
		std::vector<PrimitiveVertex>& vertsForBuff, const std::vector<unsigned short>& indices, int idx);

	std::weak_ptr<ID3D11ShaderResourceView*> _normalTex;
	std::weak_ptr<ID3D11ShaderResourceView*> _dispMask;
	std::weak_ptr<ID3D11ShaderResourceView*> _displaysmentMap;
	std::weak_ptr<ID3D11ShaderResourceView*> _heightMap;
public:
	Sphere(unsigned int divNum,float radius,const std::shared_ptr<Camera>& cam );
	~Sphere();

	ID3D11Buffer* VertexBuffer(){ return _vertexBuffer; };

	ID3D11VertexShader* _vertexShader;
	ID3D11InputLayout* _inputlayout;
	ID3D11PixelShader* _pixelShader;

	ID3D11VertexShader* _lightviewVS;
	ID3D11InputLayout* _lightviewInputLayout;
	ID3D11PixelShader* _lightviewPS;

	Vector3& GetPos(){ return pos; };

	void Update();

	void Draw();
	void DrawLightView();
	void DrawLightView_color();
	void DrawCameraDepth();
};

