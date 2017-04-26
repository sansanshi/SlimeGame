#pragma once
#include <D3D11.h>
#include<xnamath.h>

class Camera;

class Billboard
{
private:
	ID3D11ShaderResourceView* _texture;
	ID3D11Buffer* _vertexBuffer;
	XMFLOAT3 _pos;
	Camera& _camera;
	ID3D11VertexShader* _vs;
	ID3D11PixelShader* _ps;
	ID3D11InputLayout* _inputLayout;

	ID3D11Buffer* _wvpBuffer;
	XMMATRIX _modelMatrix;

public:

#pragma pack(1)
	struct BillBoardVertex{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};
#pragma pack()

	struct WorldAndCamera{
		XMMATRIX world;
		XMMATRIX camera;
	};
	Billboard(Camera& camera,float width,float height);
	~Billboard();

	void SetPos(XMFLOAT3&);
	XMFLOAT3& GetPos();

	void Draw();

	void Update();

	//ワールドビュープロジェクションバッファ生成
	//後で消す　　「3d空間内に頂点を持つオブジェクトで共有する」ため
	void CreateWVPConstBuffer();
};

