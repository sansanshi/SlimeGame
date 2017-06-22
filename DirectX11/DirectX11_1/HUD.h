#pragma once
#include <D3D11.h>
#include<xnamath.h>
#include"Define.h"
#include<memory>

class Camera;
class HUD
{
public:
	HUD(const std::shared_ptr<Camera>& camPtr,const float top,const float left,const float width,const float height);
	~HUD();

	void SetPos(XMFLOAT3&);
	XMFLOAT3& GetPos();

	void Draw();

	void Update();

	ID3D11Buffer* CreateHUDVertexBuffer(float top, float left, float width, float height);

	HRESULT CreateHUDShader(ID3D11VertexShader*&, ID3D11InputLayout*&, ID3D11PixelShader*&);
	XMMATRIX CreateHUDMatrix(float width, float height, float offsetx, float offsety );

	void Offset(const float x,const float y);

private:
	ID3D11ShaderResourceView* _texture;
	ID3D11Buffer* _vertexBuffer;
	XMFLOAT3 _pos;
	std::weak_ptr<Camera> _cameraPtr;
	ID3D11VertexShader* _vs;
	ID3D11PixelShader* _ps;
	ID3D11InputLayout* _inputLayout;

	ID3D11Buffer* _matrixBuffer;
	WorldAndCamera _worldAndCamera;
	D3D11_MAPPED_SUBRESOURCE _mem;

	XMMATRIX _modelMatrix;

	XMMATRIX _hudMatrix;
	XMMATRIX _hudMatrixIdentity;

};

