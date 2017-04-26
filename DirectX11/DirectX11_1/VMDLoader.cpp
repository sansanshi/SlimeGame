#include "VMDLoader.h"
#include<D3D11.h>
#include<xnamath.h>
#include<algorithm>

VMDLoader::VMDLoader()
{
}


VMDLoader::~VMDLoader()
{
}

VMDData*
VMDLoader::Load(std::string filepath,bool isRepeat)
{
	if ((_fp = fopen(filepath.c_str(), "rb"))==nullptr)
	{
		return nullptr;
	};
	fread(&_header, 54, 1, _fp);
	int a = sizeof(VMDHeader);

	unsigned int maxFrame=0;

	VMDData* vmdData = new VMDData();

	//外では使わない（ロード時にしか使わないような）構造体は関数内で宣言していい
#pragma pack(1)
	struct VmdMotion
	{
		char boneName[15];
		unsigned int frameNo;//フレーム番号
		XMFLOAT3 translation;//移動量
		XMFLOAT4 quaternion;//クォータニオン
		BYTE interporation[64];//bezier[4][4][4];//ベジェ

	};
#pragma pack()

	std::vector<VmdMotion> datas(_header.motionCnt);
	fread(&datas[0], sizeof(VmdMotion), _header.motionCnt, _fp);

	/*vmdData->_motionDatas.resize(_header.motionCnt);
	for (int i = 0; i < datas.size(); i++)
	{
		vmdData->_motionDatas[i].boneName = datas[i].boneName;
		vmdData->_motionDatas[i].quaternion = XMLoadFloat4(&datas[i].quaternion);
	}*/
	for (auto& m : datas)
	{
		VMDData::MotionData md = { m.frameNo, XMLoadFloat4(&m.quaternion) };
		maxFrame = max(maxFrame, m.frameNo);
		vmdData->GetKeyframes()[m.boneName].push_back(md);//.push_back(md);
	}

	/*XMFLOAT4 q = {0,0,0,1};
	VMDData::MotionData md = { 20, XMLoadFloat4(&q) };
	vmdData->GetKeyframes()["センター"].push_back(md);

	XMFLOAT4 q2 = { 0, 0, 0, 1 };
	VMDData::MotionData md2 = { 10, XMLoadFloat4(&q2) };
	vmdData->GetKeyframes()["センター"].push_back(md2);*/


	//ソートする
	for (auto& bone : vmdData->_keyframes)
	{
		auto& keyframes = bone.second;
		std::sort(keyframes.begin(), keyframes.end(),
			[](const VMDData::MotionData& lv, const VMDData::MotionData& rv)
		{return lv.frameNo < rv.frameNo; });
	}

	vmdData->_duration = maxFrame;
	vmdData->_isRepeat=isRepeat;

	return vmdData;

}

void
VMDLoader::LoadVMD()
{
	
}
