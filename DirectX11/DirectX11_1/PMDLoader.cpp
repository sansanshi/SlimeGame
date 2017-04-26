#include "PMDLoader.h"
#include<D3D11.h>
#include<D3DX11.h>
#include<vector>
#include<xnamath.h>
#include"DeviceDx11.h"
#include"Define.h"

//PMD頂点構造体
#pragma pack(1)
struct PMDVert{
	XMFLOAT3 pos;//頂点座標 12バイト
	XMFLOAT3 normal;//法線ベクトル　12バイト
	Vector2 uv;//8バイト
	WORD boneId[2];//unsigned short * 2 = 4バイト
	BYTE boneWeight;//1バイト
	BYTE edgeFlag;//1バイト
};
#pragma pack()




PMDLoader::PMDLoader()
{
}


PMDLoader::~PMDLoader()
{
}

PMDMesh*
PMDLoader::LoadPMD(std::string filepath)
{
	PMDMesh* mesh = new PMDMesh();
	HRESULT result;
	DeviceDx11& dev = DeviceDx11::Instance();

	//pmdファイルの読み込み
	FILE* fp = nullptr;
	if ((fp = fopen(filepath.c_str(), "rb")) == nullptr)
	{
		return nullptr;
	}
	fread(&signature, sizeof(signature), 1, fp);
	//↓のfseekは意味なかったけど一応残しておく　freadで読み込んだ時点でfpは読み込んだ分ずれている
	int n = fseek(fp, 3, SEEK_SET);//3バイト分(↑のsignature変数の分)ずらす 
	fread(&_header, sizeof(_header), 1, fp);

	unsigned int vertexCnt;//頂点数
	fread(&vertexCnt, sizeof(vertexCnt), 1, fp);//頂点数読み込み
	//頂点データ読み込み
	std::vector<PMDVert> pmdVertices(vertexCnt);
	for (int i = 0; i < (int)vertexCnt; i++)
	{
		fread(&pmdVertices[i], 38, 1, fp);
	}



	


	//インデックデータ読み込み
	unsigned int indexCnt;
	fread(&indexCnt, sizeof(indexCnt), 1, fp);
	std::vector<unsigned short> indices(indexCnt);
	fread(&indices[0], sizeof(unsigned short), indices.size(), fp);

	D3D11_BUFFER_DESC indexBuffDesc = {};
	indexBuffDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBuffDesc.ByteWidth = sizeof(unsigned short)*indices.size();
	indexBuffDesc.StructureByteStride = sizeof(unsigned short);
	indexBuffDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBuffDesc.CPUAccessFlags = 0;
	indexBuffDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = &indices[0];

	//ID3D11Buffer* indexBuffer;
	result = dev.Device()->CreateBuffer(&indexBuffDesc, &indexData, &mesh->_indexBuffer);
	if (FAILED(result))
	{
		delete mesh;
		return nullptr;
	}

	std::vector<PMDVertForBuff> vertsForBuff(pmdVertices.size());
	for (int i = 0; i < (int)pmdVertices.size(); i++)
	{
		vertsForBuff[i].pos = pmdVertices[i].pos;
		vertsForBuff[i].normal = pmdVertices[i].normal;
		vertsForBuff[i].uv = pmdVertices[i].uv;
		vertsForBuff[i].boneId[0] = pmdVertices[i].boneId[0];
		vertsForBuff[i].boneId[1] = pmdVertices[i].boneId[1];
		vertsForBuff[i].boneWeight = pmdVertices[i].boneWeight;
		vertsForBuff[i].edgeFlag = pmdVertices[i].edgeFlag;

	}

	{//頂点情報に従法線(binormal)、接線(tangent)情報を計算して追加
		int idx = 0;
		int faceCnt = indices.size() / 3;

		std::vector<int> cnt(vertsForBuff.size());//使われる回数
		std::fill(cnt.begin(), cnt.end(), 0);

		for (int i = 0; i < faceCnt; i++)
		{
			TempVertex v0, v1, v2;
			Vector3 tangent, binormal;

			v0.pos = vertsForBuff[indices[idx]].pos;
			v0.uv.x = vertsForBuff[indices[idx]].uv.x;
			v0.uv.y = vertsForBuff[indices[idx]].uv.y;
			v0.normal = vertsForBuff[indices[idx]].normal;
			//++idx;
			++cnt[indices[idx]] ;

			v1.pos = vertsForBuff[indices[idx+1]].pos;
			v1.uv.x = vertsForBuff[indices[idx+1]].uv.x;
			v1.uv.y = vertsForBuff[indices[idx+1]].uv.y;
			v1.normal = vertsForBuff[indices[idx+1]].normal;
			//++idx;
			++cnt[indices[idx + 1]] ;

			v2.pos = vertsForBuff[indices[idx+2]].pos;
			v2.uv.x = vertsForBuff[indices[idx+2]].uv.x;
			v2.uv.y = vertsForBuff[indices[idx+2]].uv.y;
			v2.normal = vertsForBuff[indices[idx+2]].normal;
			//++idx;
			++cnt[indices[idx + 2]] ;

			CalculateTangentBinormal(&v0, &v1, &v2, vertsForBuff,indices,idx);


			if (cnt[23] == 1)
			{
				int g = 0;
			}
			if (cnt[23] == 2)
			{
				int g = 0;
			}
			if (cnt[23] == 3)
			{
				int g = 0;
			}
			if (cnt[23] == 4)
			{
				int g = 0;
			}


			idx += 3;
		}
		int aaaaa = 0;
	}
	D3D11_BUFFER_DESC desc = {};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.ByteWidth = sizeof(PMDVertForBuff) * vertsForBuff.size();// desc.ByteWidth = sizeof(Position) * 3;
	desc.StructureByteStride = sizeof(PMDVertForBuff);//sizeof(Position) * 3 * 3;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &vertsForBuff[0];//配列の先頭アドレス入れる

	result = dev.Device()->CreateBuffer(&desc, &data, &mesh->_vertexBuffer);
	if (FAILED(result))
	{
		delete mesh;
		return nullptr;
	}

	//マテリアル情報読み込み
	unsigned int materialCnt;
	fread(&materialCnt, sizeof(unsigned int), 1, fp);

	std::vector<MATERIAL> materials;
	materials.resize(materialCnt);
	for (int i = 0; i < (int)materials.size(); i++)
	{
		fread(&materials[i], 70, 1, fp);
	}



	//マテリアルはインデックス情報を持っている
	//1つのインデックスに複数のマテリアルが設定されることはないので、
	//全マテリアルに設定されたインデックス数は総インデックス数と同じになる

	int slashIdx = filepath.rfind('/');
	std::string folderPath = filepath.substr(0, slashIdx+1);
	mesh->_folderPath = folderPath;

	std::string fileName = filepath.substr(slashIdx+1, filepath.length() - slashIdx);


	mesh->_materials.resize(materialCnt);
	
	//マテリアルが使うテクスチャの準備
	D3DX11CreateShaderResourceViewFromFile(dev.Device(), "models/nulltex.png", nullptr, nullptr, &_nulltexture, &result);
	D3DX11CreateShaderResourceViewFromFile(dev.Device(), "models/nulltexb.png", nullptr, nullptr, &_nulltextureb, &result);
	int offset = 0;
	for (int i = 0; i < (int)materials.size(); i++)
	{
		mesh->_materials[i].diffuse.x = materials[i].diffuse.x;
		mesh->_materials[i].diffuse.y = materials[i].diffuse.y;
		mesh->_materials[i].diffuse.z = materials[i].diffuse.z;
		mesh->_materials[i].alpha = materials[i].diffuse.w;

		mesh->_materials[i].ambient = materials[i].ambient;
		mesh->_materials[i].specularColor = materials[i].specularColor;
		mesh->_materials[i].specularity = materials[i].specularity;
		mesh->_materials[i].indicesNum = materials[i].indicesNum;
		mesh->_materials[i].offset = offset;
		offset += materials[i].indicesNum;
		mesh->_materials[i].texture = nullptr;
		mesh->_materials[i].sph = nullptr;
		mesh->_materials[i].spa = nullptr;
		//[*]があったときなかったときで処理変える
		//無ければスフィアなし
		//スフィアありならtextureとsphにそれぞれTextureLoadする
		std::string str = materials[i].textureName;
		int astariskIdx = str.find('*');
		if (astariskIdx == -1)
		{
			LoadTexture(str, folderPath, mesh->_materials[i]);
		}
		else
		{
			std::string str1 = str.substr(0, astariskIdx);
			std::string str2 = str.substr(astariskIdx + 1, str.length() - astariskIdx);

			LoadTexture(str1, folderPath, mesh->_materials[i]);
			LoadTexture(str2, folderPath, mesh->_materials[i]);
		}
		//D3DX11CreateShaderResourceViewFromFile(dev.Device(), str.c_str(), nullptr, nullptr, &textures[i], &result);
		//if (textures[i] == nullptr) textures[i] = defaultTex;
	}
	

	//ボーン情報読み込み
	unsigned short boneCnt;
	fread(&boneCnt, sizeof(boneCnt), 1, fp);

	std::vector<BONE> bones(boneCnt);
	for (int i = 0; i < (int)bones.size(); i++)
	{
		//構造体の定義のときにpragma pack(1)してあるので構造体アライメントは起きない
		fread(&bones[i], sizeof(BONE), 1, fp);
		mesh->_boneMap[bones[i].boneName] = i;
	}
	mesh->_bones = bones;

	mesh->_boneVerts.resize(bones.size());
	for (int i = 0; i < (int)bones.size(); i++)
	{
		if (bones[i].tailPosBoneIdx != 0)
		{
			mesh->_boneVerts[i].name = bones[i].boneName;//01/24
			mesh->_boneVerts[i].head.pos = bones[i].bonePos;
			mesh->_boneVerts[i].head.boneId = i;
			mesh->_boneVerts[i].tail.pos = bones[bones[i].tailPosBoneIdx].bonePos;
			mesh->_boneVerts[i].tail.boneId = i;
		}
		else
		{
			mesh->_boneVerts[i].name = bones[i].boneName;//01/24
			mesh->_boneVerts[i].head.pos = bones[i].bonePos;
			mesh->_boneVerts[i].head.boneId = i;
			mesh->_boneVerts[i].tail.pos = bones[i].bonePos;
			mesh->_boneVerts[i].tail.boneId = i;
		}
	}

	std::vector<BoneVertForBuff> test(bones.size() * 2);
	for (int i = 0; i < (int)bones.size(); )
	{
		test[i*2] = mesh->_boneVerts[i].head;
		test[i*2+1] = mesh->_boneVerts[i].tail;
		i += 1;
	}

	D3D11_BUFFER_DESC boneVertBuffDesc = {};
	boneVertBuffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	int hh = test.size();
	boneVertBuffDesc.ByteWidth = test.size()*sizeof(BoneVertForBuff);
	boneVertBuffDesc.CPUAccessFlags = 0;
	boneVertBuffDesc.Usage = D3D11_USAGE_DEFAULT;
	boneVertBuffDesc.StructureByteStride = sizeof(BoneVertForBuff);
	boneVertBuffDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA boneVertData;
	boneVertData.pSysMem = &test[0];

	result = dev.Device()->CreateBuffer(&boneVertBuffDesc, &boneVertData, &mesh->_boneVertBuffer);

	mesh->_vertexSize = vertexCnt;
	mesh->_indexSize = indexCnt;
	mesh->_boneSize = boneCnt;
	mesh->_materialSize = materialCnt;


	mesh->_boneMatrixies.resize(512);
	for (auto& bone : mesh->_boneMatrixies)
	{
		bone = XMMatrixIdentity();
	}
	D3D11_BUFFER_DESC boneMatrixBuffDesc = {};
	boneMatrixBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	boneMatrixBuffDesc.ByteWidth = sizeof(XMMATRIX) * 512;
	boneMatrixBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	boneMatrixBuffDesc.MiscFlags = 0;
	boneMatrixBuffDesc.Usage = D3D11_USAGE_DYNAMIC;
	boneMatrixBuffDesc.StructureByteStride = sizeof(XMMATRIX);

	D3D11_SUBRESOURCE_DATA boneMatData;
	boneMatData.pSysMem = &mesh->_boneMatrixies[0];

	dev.Device()->CreateBuffer(&boneMatrixBuffDesc, &boneMatData, &mesh->_boneMatrixBuff);


	unsigned short ikCount;
	fread(&ikCount, sizeof(unsigned short), 1, fp);
	
	for (int i = 0; i < ikCount; ++i)
	{
		unsigned short ikboneIdx=-1;
		unsigned short targetboneIdx=-1;
		unsigned int ikchainLength=0;
		unsigned short iterations = 0;
		float controlWeight = 0.0f;

		fread(&ikboneIdx, sizeof(unsigned short), 1, fp);
		//ボーン配列からikボーンの名前を取ってくる
		std::string ikboneName = bones[ikboneIdx].boneName;

		fread(&targetboneIdx, sizeof(unsigned short), 1, fp);
		fread(&ikchainLength, sizeof(unsigned char), 1, fp);
		fread(&iterations, sizeof(unsigned short), 1, fp);
		fread(&controlWeight, sizeof(float), 1, fp);


		PMDMesh::IKList iklist = {};
		iklist.ikboneIndices.resize(ikchainLength);
		iklist.ikchainLen = ikchainLength;
		for (int i = 0; i < ikchainLength; ++i)
		{
			unsigned int idx = 0;
			fread(&idx, sizeof(unsigned short), 1, fp);
			iklist.ikboneIndices[i] = idx;
		}

		iklist.iterations = iterations;
		iklist.limitangle = controlWeight;

		//今↑で作ったIKListのコピーをmeshの連想配列に入れる
		mesh->_ikMap[ikboneName] = iklist;
		
	}



	return mesh;
}

PMDMesh*
PMDLoader::Load(std::string filepath)
{
	if (_map.find(filepath) == _map.end())
	{
		_map[filepath] = LoadPMD(filepath);
	}
	else
	{
		return _map[filepath];
	}
	return _map[filepath];
}


void
PMDLoader::CalculateTangentBinormal(TempVertex* v0,TempVertex* v1, TempVertex* v2,
									std::vector<PMDVertForBuff>& vertsForBuff,const std::vector<unsigned short>& indices,int idx)
{
	//後でVector3 Vector2のdouble型を作って試す
	//ついでに平均も取ってみる

	Vector3 vec01, vec02;//頂点0から頂点1、頂点0から頂点2のベクトル
	Vector2 uvVec01, uvVec02;

	vec01 = v1->pos - v0->pos;
	vec02 = v2->pos - v0->pos;

	uvVec01 = v1->uv - v0->uv;
	uvVec02 = v2->uv - v0->uv;

	double den = 1.0f / (uvVec01.x*uvVec02.y - uvVec02.x*uvVec01.y);

	Vector3 tangent;
	Vector3 binormal;

	tangent.x = (uvVec02.y*vec01.x - uvVec01.y*vec02.x)*den;
	tangent.y = (uvVec02.y*vec01.y - uvVec01.y*vec02.y)*den;
	tangent.z = (uvVec02.y*vec01.z - uvVec01.y*vec02.z)*den;
	//tangent = vec01*uvVec02.y - vec02*uvVec01.y;//↑3はこっちでもいい

	binormal.x = (uvVec01.x*vec02.x - uvVec02.x*vec01.x)*den;
	binormal.y = (uvVec01.x*vec02.y - uvVec02.x*vec01.y)*den;
	binormal.z = (uvVec01.x*vec02.z - uvVec02.x*vec01.z)*den;
	//binormal = vec02*uvVec01.x - vec01*uvVec02.x;//↑3はこっちでもいい


	tangent = tangent.Normalize();
	binormal = binormal.Normalize();

	//↓Normalize関数作ったので要らない
	/*float length;
	length = sqrt((tangent.x*tangent.x) + (tangent.y * tangent.y) + (tangent.z*tangent.z));
	tangent.x = tangent.x / length;
	tangent.y = tangent.y / length;
	tangent.z = tangent.z / length;
	length = sqrt((binormal.x*binormal.x) + (binormal.y*binormal.y) + (binormal.z*binormal.z));
	binormal.x = binormal.x / length;
	binormal.y = binormal.y / length;
	binormal.z = binormal.z / length;*/

	vertsForBuff[indices[idx]].tangent = tangent;
	vertsForBuff[indices[idx]].binormal = binormal;
	vertsForBuff[indices[idx+1]].tangent = tangent;
	vertsForBuff[indices[idx+1]].binormal = binormal;
	vertsForBuff[indices[idx+2]].tangent = tangent;
	vertsForBuff[indices[idx+2]].binormal = binormal;


	int a = 0;

}

void
PMDLoader::CalculateTangent(TempVertex * v0, TempVertex* v1, TempVertex* v2,
std::vector<PMDVertForBuff>& vertsForBuff, const std::vector<unsigned short>& indices, int idx)
{
	Vector3 tangent;
	Vector3 binormal;

	Vector3 cp0[3] = {
		Vector3(v0->pos.x, v0->uv.x, v0->uv.y),
		Vector3(v0->pos.y, v0->uv.x, v0->uv.y),
		Vector3(v0->pos.z, v0->uv.x, v0->uv.y)
	};
	Vector3 cp1[3] = {
		Vector3(v1->pos.x, v1->uv.x, v1->uv.y),
		Vector3(v1->pos.y, v1->uv.x, v1->uv.y),
		Vector3(v1->pos.z, v1->uv.x, v1->uv.y)
	};
	Vector3 cp2[3] = {
		Vector3(v2->pos.x, v2->uv.x, v2->uv.y),
		Vector3(v2->pos.y, v2->uv.x, v2->uv.y),
		Vector3(v2->pos.z, v2->uv.x, v2->uv.y)
	};

	//平面パラメータからUV軸座標算出
	float u[3], v[3];
	for (int i = 0; i < 3; i++)
	{
		Vector3 vec1 = cp1[i] - cp0[i];
		Vector3 vec2 = cp2[i] - cp1[i];
		Vector3 abc;
		abc = vec1.Cross(vec2);

		if (abc.x == 0.0f)
		{
			return;
		}
		u[i] = -abc.y / abc.x;
		v[i] = -abc.z / abc.x;

	}
	tangent.x = u[0];
	tangent.y = u[1];
	tangent.z = u[2];
	binormal.x = v[0];
	binormal.y = v[1];
	binormal.z = v[2];

	tangent=tangent.Normalize();
	binormal = binormal.Normalize();

	vertsForBuff[indices[idx]].tangent = tangent;
	vertsForBuff[indices[idx]].binormal = binormal;
	vertsForBuff[indices[idx + 1]].tangent = tangent;
	vertsForBuff[indices[idx + 1]].binormal = binormal;
	vertsForBuff[indices[idx + 2]].tangent = tangent;
	vertsForBuff[indices[idx + 2]].binormal = binormal;
	int bg = 0;
	return;
}

void 
PMDLoader::LoadTexture(std::string filename, std::string folderName, PMDMaterial& material)
{
	DeviceDx11& dev = DeviceDx11::Instance();
	HRESULT result;

	int extIdx = filename.rfind('.');//拡張子の前の「.」
	std::string ext = filename.substr(extIdx + 1, filename.length() - extIdx);
	std::string filePath = folderName + filename;

	//spa sphならmeshのもつmaterialsのspa,sphに読み込む
	//違ったらtextureに読み込む
	if (ext == "sph"){
		D3DX11CreateShaderResourceViewFromFile(dev.Device(), filePath.c_str(), nullptr, nullptr, &material.sph, &result);
	}
	else if (ext == "spa")
	{
		D3DX11CreateShaderResourceViewFromFile(dev.Device(), filePath.c_str(), nullptr, nullptr, &material.spa, &result);
	}
	else D3DX11CreateShaderResourceViewFromFile(dev.Device(), filePath.c_str(), nullptr, nullptr, &material.texture, &result);

	if (material.spa == nullptr)material.spa = _nulltextureb;
	if (material.sph == nullptr)material.sph = _nulltexture;
	if (material.texture == nullptr)material.texture = _nulltexture;

	
}
