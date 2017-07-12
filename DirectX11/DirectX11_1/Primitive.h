#pragma once
#include<D3D11.h>
#include<xnamath.h>
#include<memory>
#include"Define.h"

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


public:
	Primitive();
	~Primitive();
};

