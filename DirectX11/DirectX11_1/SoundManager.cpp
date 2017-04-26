#include "SoundManager.h"


SoundManager::SoundManager()
{
	acbFilePath = "sound/Basic.acb";
	awbFilePath = "sound/Basic.awb";
	acfFilePath = "sound/ADX2_samples.acf";

}


SoundManager::~SoundManager()
{
}


static void CriErrorCallbackFunction(const CriChar8 *errid,
	CriUint32 p1, CriUint32 p2,
	CriUint32 *parray)
{
	const CriChar8 *errmsg;
	//debug�E�B���h�E�ɃG���[�o��
	errmsg = criErr_ConvertIdToMessage(errid, p1, p2);
	//::OutputDebugStringA(errmsg);
}

void* CriAllocatorFunction(void *obj, CriUint32 size)
{
	void *ptr = malloc(size);
	return ptr;
}

void CriFreeFunc(void *obj, void *ptr)
{
	free(ptr);
}

void
SoundManager::Init()
{
	//�G���[���̃R�[���o�b�N�֐��o�^
	criErr_SetCallback(CriErrorCallbackFunction);

	//�������A���P�[�^�̓o�^
	criAtomEx_SetUserAllocator(CriAllocatorFunction, CriFreeFunc, nullptr);

	//���C�u����������
	criAtomEx_Initialize_PC(nullptr, nullptr, 0);

	//�X�g���[�~���O�p�o�b�t�@�̍쐬
	dbas = criAtomDbas_Create(nullptr, nullptr, 0);

	//�S�̐ݒ�t�@�C���̓o�^
	criAtomEx_RegisterAcfFile(nullptr, acfFilePath,nullptr,0);

	//DSP�o�X�ݒ�̓o�^
	criAtomEx_AttachDspBusSetting("DspBufSetting_0", nullptr, 0);

	//�{�C�X�v�[���̍쐬
	CriAtomExStandardVoicePoolConfig vpconfig;
	criAtomExVoicePool_SetDefaultConfigForStandardVoicePool(&vpconfig);
	vpconfig.player_config.streaming_flag = CRI_TRUE;
	voicePool = criAtomExVoicePool_AllocateStandardVoicePool(&vpconfig, nullptr, 0);

	//ACB�t�@�C���̃��[�h
	acbHandle = criAtomExAcb_LoadAcbFile(nullptr, acbFilePath, nullptr, awbFilePath, nullptr, 0);
		
	//�v���C���[�̍쐬
	player = criAtomExPlayer_Create(nullptr, nullptr, 0);

	//�Ƃ肠����BGM�炷
	criAtomExPlayer_SetCueId(player, acbHandle,0);
	criAtomExPlayer_Start(player);
}

void
SoundManager::Update()
{
	criAtomEx_ExecuteMain();
}

void
SoundManager::Terminate()
{
	//��n��
	criAtomExPlayer_Destroy(player);
	criAtomExAcb_Release(acbHandle);

	criAtomExVoicePool_Free(voicePool);
	criAtomEx_UnregisterAcf();
	criAtomEx_Finalize_PC();
}

void CriFreeCallBack()
{

}

void
SoundManager::Play(int cueId)
{
	criAtomExPlayer_SetCueId(player, acbHandle, cueId);
	criAtomExPlayer_Start(player);
}