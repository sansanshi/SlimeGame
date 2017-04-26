#pragma once
#include<string>

#include<map>
#include"VMDData.h"
//.vmdからデータを読み取りVMDDataを返す

#pragma pack(1)
struct VMDHeader//54バイト
{
	char motionName[30];
	char modelName[20];
	unsigned int motionCnt;//モーション数
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

