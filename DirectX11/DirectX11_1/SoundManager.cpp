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
	//debugウィンドウにエラー出力
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
	//エラー時のコールバック関数登録
	criErr_SetCallback(CriErrorCallbackFunction);

	//メモリアロケータの登録
	criAtomEx_SetUserAllocator(CriAllocatorFunction, CriFreeFunc, nullptr);

	//ライブラリ初期化
	criAtomEx_Initialize_PC(nullptr, nullptr, 0);

	//ストリーミング用バッファの作成
	dbas = criAtomDbas_Create(nullptr, nullptr, 0);

	//全体設定ファイルの登録
	criAtomEx_RegisterAcfFile(nullptr, acfFilePath,nullptr,0);

	//DSPバス設定の登録
	criAtomEx_AttachDspBusSetting("DspBufSetting_0", nullptr, 0);

	//ボイスプールの作成
	CriAtomExStandardVoicePoolConfig vpconfig;
	criAtomExVoicePool_SetDefaultConfigForStandardVoicePool(&vpconfig);
	vpconfig.player_config.streaming_flag = CRI_TRUE;
	voicePool = criAtomExVoicePool_AllocateStandardVoicePool(&vpconfig, nullptr, 0);

	//ACBファイルのロード
	acbHandle = criAtomExAcb_LoadAcbFile(nullptr, acbFilePath, nullptr, awbFilePath, nullptr, 0);
		
	//プレイヤーの作成
	player = criAtomExPlayer_Create(nullptr, nullptr, 0);

	//とりあえずBGM鳴らす
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
	//後始末
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