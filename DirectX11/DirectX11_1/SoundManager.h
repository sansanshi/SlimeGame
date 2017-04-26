#pragma once
#include<cri_adx2le.h>
#include<string>

class SoundManager
{

private:
	CriChar8* acfFilePath;
	CriChar8* acbFilePath;
	CriChar8* awbFilePath;
	CriAtomExAcbHn acbHandle;//ACBハンドル
	CriAtomExVoicePoolHn voicePool;//ボイスプールハンドル
	CriAtomDbasId dbas;//D-BASハンドル
	CriAtomExPlayerHn player;//プレイヤハンドル

	/*static void CriErrorCallbackFunction(const CriChar8 *errid,
		CriUint32 p1, CriUint32 p2,
		CriUint32 *parray);*/
public:
	SoundManager();
	~SoundManager();

	void Init();
	void Update();
	void Load();
	void Play(int cueId);//鳴らす
	void Terminate();//明示的に終わらせる
};

