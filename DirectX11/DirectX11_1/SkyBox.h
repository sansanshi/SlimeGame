#pragma once
#include<D3D11.h>
#include<xnamath.h>
class SkyBox
{
private:
	struct Material{
		ID3D11ShaderResourceView* texture;
		unsigned int indicesNum;
		unsigned int offset;
	};

	ID3D11Buffer* _vertBuffer;
	ID3D11Buffer* _indexBuffer;

	
public:
	SkyBox();
	~SkyBox();
};

