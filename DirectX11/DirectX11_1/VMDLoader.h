#pragma once
#include<string>

#include<map>
#include"VMDData.h"
//.vmd����f�[�^��ǂݎ��VMDData��Ԃ�

#pragma pack(1)
struct VMDHeader//54�o�C�g
{
	char motionName[30];
	char modelName[20];
	unsigned int motionCnt;//���[�V������
};
#pragma pack()


class VMDLoader
{
private:
	FILE* _fp;
	VMDHeader _header;

public:
	VMDLoader();
	~VMDLoader();

	VMDData* Load(std::string filepath,bool repeatFlag);
	void LoadVMD();
};

