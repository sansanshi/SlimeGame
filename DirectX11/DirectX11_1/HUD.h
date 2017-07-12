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

	XMMATRIX CreateHUDMatrix(float width, float height, float offsetx, float offsety );

	void Offset(const float x,const float y);

private:
	std::weak_ptr<ID3D11ShaderResourceView*> _texture;
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

	XMMATRIX _hudMatrix;
	XMMATRIX _hudMatrixIdentity;

};

