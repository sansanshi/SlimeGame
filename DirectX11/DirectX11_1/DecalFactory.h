#pragma once
#include<D3DX11.h>
#include<xnamath.h>
#include<vector>
#include<memory>
#include"Define.h"

class Camera;
class DecalBox;

class DecalFactory
{
public:
	DecalFactory(const std::shared_ptr<Camera> cameraPtr);
	~DecalFactory();

	void CreateDecalBox(const XMFLOAT3& pos);
	void CreateDecalBox(const XMFLOAT3& pos, const XMFLOAT3& rot,const XMFLOAT3& scale);
	void CreateDecalBox(const XMMATRIX& worldMat);
	void Update();
	void Draw();
private:
	std::vector<DecalBox*> _decalBoxes;
	//ID3D11ShaderResourceView* _decalTex;

	std::weak_ptr<ID3D11ShaderResourceView*> _decalTex;

	std::weak_ptr<Camera> _cameraPtr;

	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* _indexBuffer;
	unsigned int _indicesCnt;
	std::weak_ptr<ID3D11VertexShader*> _vertexShader;
	std::weak_ptr<ID3D11PixelShader*> _pixelShader;
	std::weak_ptr<ID3D11InputLayout*> _inputlayout;


	XMMATRIX _worldMatrixes[300];
	XMMATRIX _invWorldMatrixes[300];
	unsigned int _decalBoxCnt;

	MatrixiesForDecalFac _matrixies;
	ID3D11Buffer* _matrixBuffer;
	D3D11_MAPPED_SUBRESOURCE _mappedMatrixies;
	ID3D11SamplerState* _samplerState;
};

