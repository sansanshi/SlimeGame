#pragma once
#include<D3D11.h>
#include<xnamath.h>
#include<memory>
#include"Define.h"

class Camera;

struct PrimitiveVertex{
	Vector3 pos;
	Vector3 normal;
	Vector2 uv;
	Vector3 tangent;
	Vector3 binormal;
	PrimitiveVertex()
		:pos(Vector3(0,0,0)),normal(Vector3(0,0,0)),uv(Vector2(0,0)),tangent(Vector3(0,0,0)),binormal(Vector3(0,0,0))
	{
	}

	PrimitiveVertex(const Vector3& p, const Vector3& norm,const Vector2& coord,Vector3& tang,Vector3& binorm)
		:pos(p), normal(norm), uv(coord),tangent(tang),binormal(binorm){
	}


};

class Primitive
{
private:

protected:

	XMFLOAT3 _pos;
	XMFLOAT3 _rot;
	XMFLOAT3 _scale;

	std::weak_ptr<ID3D11VertexShader*> _vertexShader;
	std::weak_ptr<ID3D11InputLayout*> _inputlayout;
	std::weak_ptr<ID3D11PixelShader*> _pixelShader;

	std::weak_ptr<ID3D11VertexShader*> _lightviewVS;
	std::weak_ptr<ID3D11PixelShader*> _lightviewPS;
	std::weak_ptr<ID3D11InputLayout*> _lightviewInputLayout;


	std::weak_ptr<ID3D11HullShader*> _hullShader;
	std::weak_ptr<ID3D11DomainShader*> _domainShader;

	std::weak_ptr<ID3D11ShaderResourceView*> _mainTex;
	std::weak_ptr<ID3D11ShaderResourceView*> _subTex;
	std::weak_ptr<ID3D11ShaderResourceView*> _normalTex;
	std::weak_ptr<ID3D11ShaderResourceView*> _flowTex;
	std::weak_ptr<ID3D11ShaderResourceView*> _dispMask;
	std::weak_ptr<ID3D11ShaderResourceView*> _displacementTex;
	std::weak_ptr<ID3D11ShaderResourceView*> _heightMap;
	std::weak_ptr<ID3D11ShaderResourceView*> _subTex2;

	void InitTransform();

	unsigned int _vertBuffStride;
	unsigned int _vertBuffOffs;

	ID3D11Buffer* _matrixBuffer;
	D3D11_MAPPED_SUBRESOURCE _mappedMatrixies;
	WorldAndCamera _worldAndCamera;

	std::weak_ptr<Camera> _cameraPtr;

public:
	Primitive();
	~Primitive();

	void SetPos(const XMFLOAT3 pos);
	XMFLOAT3 GetPos();
	
	void SetRotate(const XMFLOAT3 yawPitchRoll);
	XMFLOAT3 GetRotate();

	void SetScale(const XMFLOAT3 scale);
	XMFLOAT3 GetScale();

	//GPUが使うバッファの中身（行列バッファ）を更新(GPUにセットはしない）
	void ApplyMatrixBuffer();

	template <typename T>
	void ApplyConstantBuffer(ID3D11Buffer* buffer, D3D11_MAPPED_SUBRESOURCE& mappedResource, const T& data)
	{
		DeviceDx11& dev = DeviceDx11::Instance();

		dev.Context()->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
		memcpy(mappedResource.pData, (void*)(&data), sizeof(data));
		//*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違えるとメモリがぐちゃぐちゃになる
		dev.Context()->Unmap(buffer, 0);
		return;
	}

	//WVP行列を更新
	void UpdateMatrixies();
	void Update();

	//カメラ用シェーダ切り替え
	void ApplyCameraShaders();
	//深度値シェーダ切り替え
	void ApplyDepthShaders();

	//持っているテクスチャをGPUに渡す
	void ApplyTextures();
};

