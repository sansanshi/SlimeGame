#pragma once
#include <D3D11.h>
#include<xnamath.h>
#include"Define.h"
#include<memory>

class Camera;

class BlurFilter
{
private:
	std::weak_ptr<ID3D11ShaderResourceView*> _texture;
	ID3D11Buffer* _vertexBuffer;
	
	std::weak_ptr<ID3D11VertexShader*> _blurXvs;
	std::weak_ptr<ID3D11PixelShader*> _blurXps;
	std::weak_ptr<ID3D11InputLayout*> _inputLayout;
	std::weak_ptr<ID3D11VertexShader*> _blurYvs;
	std::weak_ptr<ID3D11PixelShader*> _blurYps;

	ID3D11Buffer* _blurWeightBuffer;
	BlurWeight _blurWeight;
	D3D11_MAPPED_SUBRESOURCE _mappedWeight;

	ID3D11Buffer* _matrixBuffer;
	WorldAndCamera _worldAndCamera;
	D3D11_MAPPED_SUBRESOURCE _mem;

	XMMATRIX _modelMatrix;

public:

#pragma pack(1)
	struct BlurFilterVertex {
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};
#pragma pack()

	BlurFilter();
	~BlurFilter();


	void DrawBlurX();
	void DrawBlurY();

	void Update();

	ID3D11Buffer* CreateBlurFilterVertexBuffer();

};

