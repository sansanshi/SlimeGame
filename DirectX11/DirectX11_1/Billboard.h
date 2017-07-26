#pragma once
#include <D3D11.h>
#include<xnamath.h>
#include"Define.h"
#include<memory>
#include<string>

class Camera;

class Billboard
{
private:
	std::weak_ptr<ID3D11ShaderResourceView*> _texture;
	ID3D11SamplerState* _samplerState;
	ID3D11Buffer* _vertexBuffer;
	XMFLOAT3 _pos;
	std::weak_ptr<Camera> _cameraPtr;
	std::weak_ptr<ID3D11VertexShader*> _vs;
	std::weak_ptr<ID3D11PixelShader*> _ps;
	std::weak_ptr<ID3D11InputLayout*> _inputLayout;

	ID3D11Buffer* _matrixBuffer;
	WorldAndCamera _worldAndCamera;
	D3D11_MAPPED_SUBRESOURCE _mem;

	XMMATRIX _modelMatrix;

public:

#pragma pack(1)
	struct BillBoardVertex{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};
#pragma pack()

	Billboard(const std::shared_ptr<Camera> cameraPtr,float width,float height);
	Billboard(const std::string textureName, const std::shared_ptr<Camera> cameraPtr, float width, float height);
	~Billboard();

	void SetPos(XMFLOAT3&);
	XMFLOAT3& GetPos();

	void Draw();

	void Update();

	ID3D11Buffer* CreateBillBoardVertexBuffer(float width, float height);

	void SetPos(const XMFLOAT3& pos);
	
};

