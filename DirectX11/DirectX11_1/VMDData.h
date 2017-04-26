#pragma once
#include<D3D11.h>
#include<xnamath.h>
#include<vector>
#include<string>
#include<map>
#include<set>

class VMDLoader;


class VMDData
{
	friend VMDLoader;
public:
	VMDData();
	~VMDData();
#pragma pack(1)
	struct MotionData
	{
		//std::string boneName;//キーフレーム管理でボーン名とMotionDataの連想配列作るのでこっちは消してフレームナンバーに変更
		unsigned int frameNo;
		XMVECTOR quaternion;

		bool operator<(const VMDData::MotionData& rv)
		{
			return this->frameNo < rv.frameNo;
		}
		/*bool operator<(const VMDData::MotionData& lv, const VMDData::MotionData& rv)
		{
			return lv.frameNo < rv.frameNo;
		}*/
	};
#pragma pack()


	typedef std::map<std::string, std::vector<MotionData>> Keyframes_t;

	//std::vector<MotionData>& GetMotionData(){ return _motionDatas; };
	Keyframes_t& GetKeyframes(){ return _keyframes; };

	unsigned int Duration(){ return _duration; };
	bool IsRepeat(){ return _isRepeat; };
	
private:
	//std::vector<MotionData> _motionDatas;
	Keyframes_t _keyframes;

	//モーションの長さ
	unsigned int _duration;
	bool _isRepeat;

};

