#pragma once
#include<Windows.h>
#include<vector>
#include<xnamath.h>
#include<string>
#include<D3D11.h>
#include"PMXMesh.h"

#pragma pack(1)
struct PMXHeader{
	BYTE sigunature[4];
	float version;

	BYTE byteSize;//後続するデータ列のバイトサイズ
	std::vector<BYTE> byteInfo;//↑のbyteSize分の配列　PMX2.0は8で固定
	//モデル情報

};
#pragma pack()


struct TextBuf{
	unsigned int byteNum;//文字列のバイト数

};
#pragma pack(1)
struct ModelInfoU8
{
	int modelNameByteNum;
	std::vector<unsigned char> modelName;
	int modelName_eng_ByteNum;
	std::vector<unsigned char> modelName_eng;
	int commentByteNum;
	std::vector<unsigned char> comment;
	int commentByteNum_eng;
	std::vector<unsigned char> comment_eng;
};
#pragma pack()

#pragma pack(1)
struct PMXVertex
{
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT2 uv;

	

	//std::vector<XMFLOAT4> appendUV;
	//BYTE weightMethod;//ウェイト変形方式　0:BDEF1 1:BDEF2 2:BDEF4 3:SDEF
	//※変形方式により継続データが異なる／参照Indexは-1:非参照の場合があるので注意
	/*
	BDEF1 : int 		| 4   | ボーンのみ
	BDEF2 : int,int,float 	| 4*3 | ボーン2つと、ボーン1のウェイト値(PMD方式)
	BDEF4 : int*4, float*4	| 4*8 | ボーン4つと、それぞれのウェイト値。ウェイト合計が1.0である保障はしない
	SDEF  : int,int,float, float3*3
	| 4*12 | BDEF2に加え、SDEF用のfloat3(Vector3)が3つ。実際の計算ではさらに補正値の算出が必要(一応そのままBDEF2としても使用可能)
	と可変サイズになるので注意。
	*/

};
#pragma pack()




class PMXLoader
{
private:
	FILE* _fp;
	ModelInfoU8 _modelInfo;

	
	std::wstring _folderPath;

public:
	
	PMXLoader();
	~PMXLoader();

	PMXHeader _header = {};

	PMXMesh* LoadPMX(std::wstring filepath);
	void LoadPMX(FILE* fp);
	void LoadHeader(PMXMesh*);
	void LoadModelInfo(PMXMesh*);
	void LoadVertex(PMXMesh*);
	void LoadIndices(PMXMesh*);
	void LoadTextures(PMXMesh*);
	void LoadMaterial(PMXMesh*);
	void LoadBones(PMXMesh*);

};

