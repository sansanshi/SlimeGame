#pragma once
#include<cri_adx2le.h>
#include<string>

class SoundManager
{

private:
	CriChar8* acfFilePath;
	CriChar8* acbFilePath;
	CriChar8* awbFilePath;
	CriAtomExAcbHn acbHandle;//ACB�n���h��
	CriAtomExVoicePoolHn voicePool;//�{�C�X�v�[���n���h��
	CriAtomDbasId dbas;//D-BAS�n���h��
	CriAtomExPlayerHn player;//�v���C���n���h��

	/*static void CriErrorCallbackFunction(const CriChar8 *errid,
		CriUint32 p1, CriUint32 p2,
		CriUint32 *parray);*/
public:
	SoundManager();
	~SoundManager();

	void Init();
	void Update();
	void Load();
	void Play(int cueId);//�炷
	void Terminate();//�����I�ɏI��点��
};

