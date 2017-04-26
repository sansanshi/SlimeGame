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

	BYTE byteSize;//�㑱����f�[�^��̃o�C�g�T�C�Y
	std::vector<BYTE> byteInfo;//����byteSize���̔z��@PMX2.0��8�ŌŒ�
	//���f�����

};
#pragma pack()


struct TextBuf{
	unsigned int byteNum;//������̃o�C�g��

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
	//BYTE weightMethod;//�E�F�C�g�ό`�����@0:BDEF1 1:BDEF2 2:BDEF4 3:SDEF
	//���ό`�����ɂ��p���f�[�^���قȂ�^�Q��Index��-1:��Q�Ƃ̏ꍇ������̂Œ���
	/*
	BDEF1 : int 		| 4   | �{�[���̂�
	BDEF2 : int,int,float 	| 4*3 | �{�[��2�ƁA�{�[��1�̃E�F�C�g�l(PMD����)
	BDEF4 : int*4, float*4	| 4*8 | �{�[��4�ƁA���ꂼ��̃E�F�C�g�l�B�E�F�C�g���v��1.0�ł���ۏ�͂��Ȃ�
	SDEF  : int,int,float, float3*3
	| 4*12 | BDEF2�ɉ����ASDEF�p��float3(Vector3)��3�B���ۂ̌v�Z�ł͂���ɕ␳�l�̎Z�o���K�v(�ꉞ���̂܂�BDEF2�Ƃ��Ă��g�p�\)
	�ƉσT�C�Y�ɂȂ�̂Œ��ӁB
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

