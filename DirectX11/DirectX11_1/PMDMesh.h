#pragma once
#include<D3D11.h>
#include<xnamath.h>
#include<vector>
#include"Define.h"
#include<map>

class PMDLoader;

//マテリアル情報の読み込み
#pragma pack(1)
struct MATERIAL {
	XMFLOAT4 diffuse;//ディフューズカラー成分
	float specularity;//スペキュラ強度
	XMFLOAT3 specularColor;//スペキュラカラー成分
	XMFLOAT3 ambient;//環境光成分
	BYTE toonIdx;//トーンマッピングインデックス
	BYTE edgeFlag;//輪郭線フラグ　ここでアライメントに気を付ける
	unsigned int indicesNum;//対象インデックス数
	char textureName[20];//テクスチャファイル名

};
#pragma pack()

// 01/24に新しく作った
struct Bone{
	std::string name;//無駄なようだけど「ひざ」を検索するため
	XMFLOAT3 headpos;
	XMFLOAT3 tailpos;

};


#pragma pack(1)
struct PMDMaterial {
	XMFLOAT3 diffuse;//ディフューズカラー成分
	float alpha;
	float specularity;//スペキュラ強度
	XMFLOAT3 specularColor;//スペキュラカラー成分
	XMFLOAT3 ambient;//環境光成分
	unsigned int offset;
	unsigned int indicesNum;//対象インデックス数
	ID3D11ShaderResourceView* texture;
	ID3D11ShaderResourceView* spa;
	ID3D11ShaderResourceView* sph;

};
#pragma pack()

#pragma pack(1)
struct PMDVertForBuff//計62
{
	XMFLOAT3 pos;//頂点座標 12バイト
	XMFLOAT3 normal;//法線ベクトル　12バイト
	Vector2 uv;//8バイト
	WORD boneId[2];//unsigned short * 2 = 4バイト
	BYTE boneWeight;//1バイト
	BYTE edgeFlag;//1バイト
	Vector3 binormal;//12
	Vector3 tangent;//12
	//XMMATRIX uvzMatrix;//4*4*4 64バイト やっぱ要らない
};
#pragma pack()

//ボーン構造体（読み込み
#pragma pack(1)
struct BONE{
	char boneName[20];
	unsigned short parentBoneIdx;
	unsigned short tailPosBoneIdx;
	BYTE boneType;
	unsigned short ikParentBoneIdx;
	XMFLOAT3 bonePos;
};
#pragma pack()

struct BoneVertForBuff
{
	XMFLOAT3 pos;
	unsigned int boneId;
};

#pragma pack(1)
struct BoneVert
{
	std::string name;//無駄なようだけど「ひざ」を検索するため
	BoneVertForBuff head;
	BoneVertForBuff tail;
};
#pragma pack()

class PMDMesh
{
	friend PMDLoader;

public:
	struct IKList{
		std::vector<int> ikboneIndices;
		int ikchainLen;
		int iterations;
		//float controlWeight;//↓と同じ
		float limitangle;
	};

private:
	std::string _folderPath;

	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* _indexBuffer;
	ID3D11Buffer* _boneVertBuffer;

	ID3D11Buffer* _boneMatrixBuff;

	unsigned int _vertexSize;
	unsigned int _indexSize;
	unsigned int _materialSize;
	unsigned short _boneSize;
	const unsigned int _vertexStride;
	const unsigned int _boneVertStride;

	std::vector<PMDMaterial> _materials;
	std::vector<ID3D11ShaderResourceView*> _textures;
	std::vector<BONE> _bones;
	std::vector<BoneVert> _boneVerts;

	std::map<std::string, int> _boneMap;

	std::map<std::string, IKList> _ikMap;

	std::vector<XMMATRIX> _boneMatrixies;

public:
	PMDMesh();
	~PMDMesh();


	

	ID3D11Buffer* GetVertexBuffer(){ return _vertexBuffer; };
	ID3D11Buffer* GetIndexBuffer(){ return _indexBuffer; };
	ID3D11Buffer* GetBoneVertBuffer(){ return _boneVertBuffer; };

	ID3D11Buffer* GetBoneMatrixBuffer(){ return _boneMatrixBuff; };

	unsigned int GetVertexStride()const{ return _vertexStride; };
	unsigned int GetBoneVertStride()const{ return _boneVertStride; };

	unsigned int VertexSize()const { return _vertexSize; };
	unsigned int VertexStride()const{ return _vertexStride; };
	unsigned int IndexSize()const{ return _indexSize; };
	unsigned int MaterialSize()const{ return _materialSize; };
	unsigned short BoneSize()const { return _boneSize; };

	std::vector<PMDMaterial>& GetMaterials(){ return _materials; };
	std::vector<ID3D11ShaderResourceView*>& GetTextures(){ return _textures; };
	std::vector<BONE>& GetBones(){ return _bones; };
	std::vector<BoneVert>& GetBoneVertices(){ return _boneVerts; };
	std::map<std::string, int>& GetBoneMap(){ return _boneMap; };
	std::vector<XMMATRIX>& BoneMatrixies(){ return _boneMatrixies; };

	std::string FolderPath(){ return _folderPath; };

	void Draw();

	std::map<std::string, IKList>& IKListMap(){return _ikMap;}
		
	
};

