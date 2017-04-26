#pragma once
//PMDMeshオブジェクトを生成し、クライアントに提供する
#include<D3D11.h>
#include<string>
#include<map>
#include"PMDMesh.h"
#include"Define.h"

struct PMDHeader{
	//signatureが3バイト→コンピュータは4バイトずつ読み込む
	//→コンパイラがメモリを詰めて処理しようとして1バイトずつズレてしまう
	//char signature[3];//シグネチャ　どういうファイルなのか
	float version;//バージョン
	char name[20];//モデルの名前
	char comment[256];//コメント
	//unsigned int vertexCount;//頂点数 
};

struct TempVertex
{
	Vector3 pos;
	Vector2 uv;
	Vector3 normal;
};

class PMDLoader
{
private:
	std::map<std::string, PMDMesh*> _map;
	PMDMesh* LoadPMD(std::string filepath);

	ID3D11ShaderResourceView* _nulltexture;
	ID3D11ShaderResourceView* _nulltextureb;

	//読み込むときにmeshの持つmaterials[i]を渡す
	//void LoadTexture(std::string filename,std::string foldername,PMDMesh::Material& material);

	PMDHeader _header;
	char signature[3];//ここだけ分けて読み込む

	void CalculateTangentBinormal(TempVertex* v0, TempVertex* v1, TempVertex* v2,
		std::vector<PMDVertForBuff>& verts,const std::vector<unsigned short>& indices,int idx);

	void CalculateTangent(TempVertex * v0, TempVertex* v1, TempVertex* v2,
		std::vector<PMDVertForBuff>& verts, const std::vector<unsigned short>& indices, int idx);
	
public:
	PMDLoader();
	~PMDLoader();
		
	//ファイルパスからPmdmeshを生成して返す
	//戻り値：PMDMeshオブジェクトへのポインタ
	//ファイルがなければnullptrを返す
	PMDMesh* Load(std::string filePath);

	void LoadTexture(std::string, std::string, PMDMaterial&);

};

