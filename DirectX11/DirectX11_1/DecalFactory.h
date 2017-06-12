#pragma once
#include<D3DX11.h>
#include<xnamath.h>
#include<vector>
#include<memory>

class Camera;
class DecalBox;

class DecalFactory
{
public:
	DecalFactory(Camera* cameraPtr);
	~DecalFactory();

	void CreateDecalBox(const XMFLOAT3& pos);
	void CreateDecalBox(const XMFLOAT3& pos, const XMFLOAT3& rot,const XMFLOAT3& scale);
	void Update();
	void Draw();
private:
	std::vector<DecalBox*> _decalBoxes;
	//ID3D11ShaderResourceView* _decalTex;

	std::shared_ptr<ID3D11ShaderResourceView*> _decalTex;

	Camera* _cameraPtr;

	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* _indexBuffer;
	unsigned int _indicesCnt;
	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;
	ID3D11InputLayout* _inputlayout;
};

