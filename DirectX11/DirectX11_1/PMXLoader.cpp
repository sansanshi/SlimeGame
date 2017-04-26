#include "PMXLoader.h"
#include<string>
#include<iostream>
#include"DeviceDx11.h"
#include<D3DX11.h>


PMXLoader::PMXLoader()
{
}


PMXLoader::~PMXLoader()
{
	
}

void
PMXLoader::LoadHeader(PMXMesh* mesh)
{
	//fread(&_header, sizeof(PMXHeader), 1, _fp);
	fread(&_header.sigunature, sizeof(_header.sigunature), 1, _fp);
	fread(&_header.version, sizeof(_header.version), 1, _fp);
	fread(&_header.byteSize, sizeof(_header.byteSize), 1, _fp);
	_header.byteInfo.resize(_header.byteSize);
	fread(&_header.byteInfo[0], sizeof(BYTE), _header.byteSize, _fp);
	return;
}
void
PMXLoader::LoadModelInfo(PMXMesh* mesh)
{

	fread(&_modelInfo.modelNameByteNum, sizeof(int), 1, _fp);//モデル名のバイト数読み込み
	_modelInfo.modelName.resize(_modelInfo.modelNameByteNum);
	for (int i = 0; i < _modelInfo.modelNameByteNum; i++)
	{
		fread(&_modelInfo.modelName[i], sizeof(BYTE), 1, _fp);
	}
	//fread(&_modelInfo.modelName, sizeof(BYTE)*_modelInfo.modelNameByteNum, 1, _fp);

	fread(&_modelInfo.modelName_eng_ByteNum, sizeof(int), 1, _fp);//モデル名（英名）のバイト数読み込み
	_modelInfo.modelName_eng.resize(_modelInfo.modelName_eng_ByteNum);
	for (int i = 0; i < _modelInfo.modelName_eng_ByteNum; i++)
	{
		fread(&_modelInfo.modelName_eng[i], sizeof(BYTE), 1, _fp);
	}

	fread(&_modelInfo.commentByteNum, sizeof(int), 1, _fp);//コメント数読み込み
	_modelInfo.comment.resize(_modelInfo.commentByteNum);
	for (int i = 0; i < (int)_modelInfo.comment.size(); i++)
	{
		fread(&_modelInfo.comment[i], sizeof(BYTE), 1, _fp);
	}

	fread(&_modelInfo.commentByteNum_eng, sizeof(int), 1, _fp);//コメント数（英語）読み込み
	_modelInfo.comment_eng.resize(_modelInfo.commentByteNum_eng);
	for (int i = 0; i < (int)_modelInfo.comment_eng.size(); i++)
	{
		fread(&_modelInfo.comment_eng[i], sizeof(BYTE), 1, _fp);
	}

	

	int b = 0;
	
	std::string str(_modelInfo.modelName.begin(), _modelInfo.modelName.end());
	
	//wchar_t name[11];
	//char c[22];
	//for (int i = 0; i < 22; i++)
	//{
	//	c[i] = str[i];
	//}
	//setlocale(LC_ALL, "Japanese");
	//size_t length = strlen(c);
	//wchar_t wc[32];
	//length=mbstowcs(wc, c, strlen(c) + 1);
	//
	//wchar_t temp = wc[2];
	return;
}

void
PMXLoader::LoadVertex(PMXMesh* mesh)
{
	unsigned int vertexCnt;
	fread(&vertexCnt, sizeof(unsigned int), 1, _fp);
	std::vector<PMXVertex> verts(vertexCnt);
	//頂点情報読み込み
	for (int i = 0; i < (int)vertexCnt; i++)
	{
		fread(&verts[i].pos, sizeof(XMFLOAT3), 1, _fp);
		fread(&verts[i].normal, sizeof(XMFLOAT3), 1, _fp);
		fread(&verts[i].uv, sizeof(XMFLOAT2), 1, _fp);
		//int型に変換するためにchar型に変換
		char c = _header.byteInfo[1];
		if (atoi(&c) > 0)
		{
			std::vector<XMFLOAT4> adduv(_header.byteInfo[1]);
			fread(&adduv[0], sizeof(XMFLOAT4), adduv.size(), _fp);
		}
		BYTE weightType = 0;
		fread(&weightType, sizeof(BYTE), 1, _fp);

		WORD boneIndex[4];
		float boneWeight[4];
		XMFLOAT3 sdef_c;
		XMFLOAT3 sdef_R0;
		XMFLOAT3 sdef_R1;
		switch (weightType)
		{
		case 0://BRDF1
			fread(&boneIndex[0], sizeof(WORD), 1, _fp);
			break;
		case 1://BRDF2
			fread(&boneIndex[0], sizeof(WORD), 2, _fp);
			fread(&boneWeight[0], sizeof(float), 1, _fp);
			break;
		case 2://BRDF3
			fread(&boneIndex[0], sizeof(WORD), 4, _fp);
			fread(&boneWeight[0], sizeof(float), 4, _fp);
			break;
		case 3://SDEF
			fread(&boneIndex[0], sizeof(WORD), 2, _fp);
			fread(&boneWeight[0], sizeof(float), 1, _fp);
			fread(&sdef_c, sizeof(XMFLOAT3), 1, _fp);
			fread(&sdef_R0, sizeof(XMFLOAT3), 1, _fp);
			fread(&sdef_R1, sizeof(XMFLOAT3), 1, _fp);
			break;
		default:
			break;
		}
		float edgeScale = 1.0f;
		fread(&edgeScale, sizeof(float), 1, _fp);

	}
	int a = 0;
}
void
PMXLoader::LoadIndices(PMXMesh* mesh)
{
	unsigned int indicesCnt = 0;
	fread(&indicesCnt, sizeof(unsigned int), 1, _fp);

	std::vector<unsigned short> indices(indicesCnt);
	for (int i = 0; i < (int)indicesCnt; i++)
	{
		fread(&indices[i], sizeof(unsigned short), 1, _fp);
	}
	int hh = 0;
}

void
PMXLoader::LoadTextures(PMXMesh* mesh)
{
	HRESULT result;
	DeviceDx11& dev = DeviceDx11::Instance();
	unsigned int textureCnt = 0;
	fread(&textureCnt, sizeof(unsigned int), 1, _fp);
	mesh->_textures.resize(textureCnt);

	//std::vector<wchar_t> textureName;
	std::wstring textureFileName;
	for (int i = 0; i < (int)textureCnt; i++)
	{
		unsigned int byteNum = 0;
		fread(&byteNum, sizeof(unsigned int), 1, _fp);
		textureFileName.resize(byteNum / 2);
		fread(&textureFileName[0], sizeof(wchar_t), byteNum/2, _fp);

		std::wstring filePath = _folderPath + textureFileName;
		D3DX11CreateShaderResourceViewFromFileW(dev.Device(), filePath.c_str(), nullptr, nullptr, &mesh->_textures[i], &result);
		//D3DX11CreateShaderResourceViewFromFile(dev.Device(), "models/tdamiku/face_MikuAp.tga", nullptr, nullptr, &_textures[i], &result);

		//toonとsphereのテクスチャは通常のテクスチャとテーブルが違うかもしれないので後で確認する
		int k = 0;
	}

	int hh = 0;
}

void
PMXLoader::LoadMaterial(PMXMesh* mesh)
{
	

	unsigned int materialCnt = 0;
	fread(&materialCnt, sizeof(unsigned int), 1, _fp);

	mesh->_materials.resize(materialCnt);
	unsigned int offset = 0;

	std::wstring materialName;
	std::wstring materialNameEng;
	for (int i = 0; i < (int)materialCnt; i++)
	{
		unsigned int byteNum_name = 0;//材質名のバイト数
		fread(&byteNum_name, sizeof(unsigned int), 1, _fp);
		materialName.resize(byteNum_name/2);
		fread(&materialName[0], sizeof(wchar_t), byteNum_name/2, _fp);

		unsigned int byteNum_nameEng = 0;
		fread(&byteNum_nameEng, sizeof(unsigned int), 1, _fp);
		materialNameEng.resize(byteNum_nameEng/2);
		fread(&materialNameEng[0], sizeof(wchar_t), byteNum_nameEng/2, _fp);

#pragma pack(1)
		struct Temp{
			XMFLOAT4 diffuse;
			XMFLOAT3 specular;
			float specularity;
			XMFLOAT3 ambient;

			BYTE bitFlag;

			XMFLOAT4 edgeColor;
			float edgeScale;
		};
#pragma pack()
		Temp temp;
		fread(&temp, sizeof(Temp), 1, _fp);

		/*char test[256];
		fread(&test[0], sizeof(char), 256, _fp);*/

		//int型にfreadで1バイト分読み込めなかった
		//とりあえずchar型で読み込んでおく
		//byteInfo[3]が2or4の場合多分ズレるので対処を考える
		// とりあえずswitchで1,2,4を分ける char ,unsigned short ,unsigned int
		char textureIdx;
		char size = _header.byteInfo[3];
		fread(&textureIdx,sizeof(BYTE)*_header.byteInfo[3],1,_fp);//char型のままでも数値として乗算に使える
		char sphereTexIdx;
		fread(&sphereTexIdx, sizeof(BYTE)*_header.byteInfo[3], 1, _fp);

		//スフィアモード 0:無効 1 : 乗算(sph) 2 : 加算(spa) 
		//3 : サブテクスチャ(追加UV1のx, yをUV参照して通常テクスチャ描画を行う)
		BYTE sphereMode;
		fread(&sphereMode, sizeof(BYTE), 1, _fp);
		BYTE toonFlag;
		fread(&toonFlag, sizeof(BYTE), 1, _fp);
		char toonTexIdx;
		if (toonFlag == 0)
		{
			fread(&toonTexIdx, sizeof(BYTE)*_header.byteInfo[3], 1, _fp);
		}
		else//toonフラグ1の時の処理
		{

		}
		unsigned int memoSize = 0;
		fread(&memoSize, sizeof(unsigned int), 1, _fp);
		std::wstring memo;
		memo.resize(memoSize/2);
		fread(&memo[0], sizeof(wchar_t), memo.size(), _fp);

		unsigned int indicesCnt = 0;
		fread(&indicesCnt, sizeof(unsigned int), 1, _fp);

		int b = (int)(textureIdx - '0');
		int k = (int)(toonTexIdx);

		mesh->_materials[i].diffuse = temp.diffuse;
		mesh->_materials[i].specular = temp.specular;
		mesh->_materials[i].specularity = temp.specularity;
		mesh->_materials[i].ambient = temp.ambient;
		mesh->_materials[i].bitFlag = temp.bitFlag;
		mesh->_materials[i].edgeColor = temp.edgeColor;
		mesh->_materials[i].edgeScale = temp.edgeScale;
		mesh->_materials[i].textureIdx = (int)(textureIdx);
		mesh->_materials[i].sphereTexIdx = (int)(sphereTexIdx);
		mesh->_materials[i].toonIdx = (int)(toonTexIdx);
		mesh->_materials[i].indicesCnt = indicesCnt;
		mesh->_materials[i].offset = offset;
		offset += indicesCnt;
		//そのマテリアルにsphereテクスチャ、toonテクスチャが無い場合の処理を後で考える

		int bb = 0;

	}

	int hh = 0;
	
}
void
PMXLoader::LoadBones(PMXMesh* mesh)
{
	unsigned int boneCnt;
	fread(&boneCnt, sizeof(unsigned int), 1, _fp);

	std::wstring boneName;
	std::wstring boneNameEng;
	for (int i = 0; i < (int)boneCnt; i++)
	{
		unsigned int boneNameByteSize = 0;
		fread(&boneNameByteSize, sizeof(unsigned int), 1, _fp);
		boneName.resize(boneNameByteSize / 2);
		fread(&boneName[0], sizeof(wchar_t)*boneName.size(), 1, _fp);


		unsigned int boneNameByteSize_eng = 0;
		fread(&boneNameByteSize_eng, sizeof(unsigned int), 1, _fp);
		boneNameEng.resize(boneNameByteSize_eng / 2);
		fread(&boneNameEng[0], sizeof(wchar_t)*boneNameEng.size(), 1, _fp);


		XMFLOAT3 pos;
		fread(&pos, sizeof(XMFLOAT3), 1, _fp);


		unsigned int parentBoneIdx = 0;
		int boneIdxSize = (int)_header.byteInfo[5];
		switch (boneIdxSize)
		{
		case 1:
			fread(&parentBoneIdx, boneIdxSize, 1, _fp);
			break;
		case 2:
			//親がいない＝ffffが入る

			fread(&parentBoneIdx, boneIdxSize, 1, _fp);
			break;
		case 4:
			fread(&parentBoneIdx, boneIdxSize, 1, _fp);
			break;
		default:
			break;
		}



		unsigned int deformHierarchy=0;
		fread(&deformHierarchy, sizeof(deformHierarchy), 1, _fp);


		unsigned short boneFlag=0;
		fread(&boneFlag, sizeof(unsigned short), 1, _fp);
		/*
		○ボーンフラグ
			0x0001  : 接続先(PMD子ボーン指定)表示方法 -> 0 : 座標オフセットで指定 1 : ボーンで指定

			0x0002 : 回転可能
			0x0004 : 移動可能
			0x0008 : 表示
			0x0010 : 操作可

			0x0020 : IK

			0x0080 : ローカル付与 | 付与対象 0 : ユーザー変形値／IKリンク／多重付与 1 : 親のローカル変形量
			0x0100 : 回転付与
			0x0200 : 移動付与

			0x0400 : 軸固定
			0x0800 : ローカル軸

			0x1000 : 物理後変形
			0x2000 : 外部親変形
		*/
		unsigned int connectBoneIdx=0;
		if (boneFlag & 0x0001)
		{

			fread(&connectBoneIdx, boneIdxSize, 1, _fp);
			int bb = 0;
		}
		else
		{
			XMFLOAT3 offset;
			fread(&offset, sizeof(XMFLOAT3), 1, _fp);
		}

		if (boneFlag & 0x0100 || boneFlag & 0x0200)
		{
			unsigned int additionParentBoneIdx = 0;//付与親ボーンインデックス
			fread(&additionParentBoneIdx, boneIdxSize, 1, _fp);
			float f = 0.f;//付与率
			fread(&f, sizeof(float), 1, _fp);
		}

		if (boneFlag & 0x0400)
		{
			XMFLOAT3 vec = {};
			fread(&vec, sizeof(XMFLOAT3), 1, _fp);
		}

		if (boneFlag & 0x0800)
		{
			XMFLOAT3 xvec = {};
			fread(&xvec, sizeof(XMFLOAT3), 1, _fp);
			XMFLOAT3 zvec = {};
			fread(&zvec, sizeof(XMFLOAT3), 1, _fp);

		}

		if (boneFlag & 0x2000)
		{
			int keyValue=0;
			fread(&keyValue, sizeof(int), 1, _fp);
		}

		if (boneFlag & 0x0020)
		{
			unsigned int IKBoneIdx = 0;
			fread(&IKBoneIdx, boneIdxSize, 1, _fp);
			int roopCnt = 0;//ループ回数
			fread(&roopCnt, sizeof(int), 1, _fp);
			float limitRad = 0.f;//制限角度
			fread(&limitRad, sizeof(float), 1, _fp);

			int IKLinkCnt=0;
			fread(&IKLinkCnt, sizeof(int), 1, _fp);
			for (int i = 0; i < IKLinkCnt; i++)
			{
				unsigned int linkBoneIdx=0;
				fread(&linkBoneIdx, boneIdxSize, 1, _fp);
				BYTE radLimitFlag=0;
				fread(&radLimitFlag, sizeof(BYTE), 1, _fp);
				if ((int)radLimitFlag)
				{
					XMFLOAT3 lowerLimit;//下限 (x,y,z) -> ラジアン角
					fread(&lowerLimit, sizeof(XMFLOAT3), 1, _fp);
					XMFLOAT3 upperLimit;
					fread(&upperLimit, sizeof(XMFLOAT3), 1, _fp);
				}
			}
		}

		int b = 0;
	}
	int k = 0;
}


PMXMesh*
PMXLoader::LoadPMX(std::wstring filepath)
{
	if ((_fp = _wfopen(filepath.c_str(), L"rb")) == nullptr)
	{
		return nullptr;
	}
	int slashIdx = filepath.rfind('/');
	_folderPath = filepath.substr(0, slashIdx + 1);

	PMXMesh* mesh = new PMXMesh();

	LoadHeader(mesh);

	LoadModelInfo(mesh);
	LoadVertex(mesh);
	LoadIndices(mesh);
	LoadTextures(mesh);
	LoadMaterial(mesh);
	LoadBones(mesh);
	return mesh;
}