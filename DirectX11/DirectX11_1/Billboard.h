#pragma once
#include <D3D11.h>
#include<xnamath.h>
#include"Define.h"

class Camera;

class Billboard
{
private:
	ID3D11ShaderResourceView* _texture;
	ID3D11Buffer* _vertexBuffer;
	XMFLOAT3 _pos;
	Camera* _cameraPtr;
	ID3D11VertexShader* _vs;
	ID3D11PixelShader* _ps;
	ID3D11InputLayout* _inputLayout;

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

	Billboard(Camera* cameraPtr,float width,float height);
	~Billboard();

	void SetPos(XMFLOAT3&);
	XMFLOAT3& GetPos();

	void Draw();

	void Update();

	ID3D11Buffer* CreateBillBoardVertexBuffer(float width, float height);

	HRESULT CreateBillBoardShader(ID3D11VertexShader*&, ID3D11InputLayout*&, ID3D11PixelShader*&);

	//ワールドビュープロジェクションバッファ生成
	//後で消す　　「3d空間内に頂点を持つオブジェクトで共有する」ため
	void CreateWVPConstBuffer();
};

