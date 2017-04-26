#pragma once
#include<D3D11.h>
#include<xnamath.h>
#include<vector>

#pragma pack(1)
struct PMXMaterial{
	XMFLOAT4 diffuse;
	XMFLOAT3 specular;
	float specularity;
	XMFLOAT3 ambient;

	//ビットフラグ 8bit　各bit 0:OFF 1:ON
	//0x01:両面描画, 0x02:地面影, 0x04:セルフシャドウマップへの描画, 
	//0x08:セルフシャドウの描画, 0x10:エッジ描画
	//多分ＭＭＤ、PMXエディタ等で表示する時の設定なので自前で描画するなら要らない
	BYTE bitFlag;

	XMFLOAT4 edgeColor;
	float edgeScale;

	unsigned int offset;
	unsigned int indicesCnt;

	unsigned int textureIdx;
	unsigned int sphereTexIdx;
	unsigned int toonIdx;
};
#pragma pack()

class PMXLoader;

class PMXMesh
{
	friend PMXLoader;
private:
	//共有toonテクスチャ　toon01~toon10に対応　共有toonフラグが1の時に使う　今はロードしてない
	std::vector<ID3D11ShaderResourceView*> _toonTextures;
	std::vector<ID3D11ShaderResourceView*> _textures;
	std::vector<PMXMaterial> _materials;

	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* _indexBuffer;


public:
	PMXMesh();
	~PMXMesh();
};

