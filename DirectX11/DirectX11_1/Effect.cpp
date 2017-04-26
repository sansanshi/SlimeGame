#include "Effect.h"
#include<Effekseer.h>
#include<EffekseerRendererDX11.h>
#include<cassert>
#include"DeviceDx11.h"
#include<D3D11.h>
#include<xnamath.h>
#include"Define.h"

#if _DEBUG
#pragma comment(lib,"VS2015/Debug/Effekseer.lib")
#pragma comment(lib,"VS2015/Debug/EffekseerRendererDX11.lib")
#else
#pragma comment(lib,"VS2013/Release/Effekseer.lib")
#pragma comment(lib,"VS2013/Release/EffekseerRendererDX11.lib")
#endif

Effect::Effect()
{
	DeviceDx11& dev = DeviceDx11::Instance();
	const int effect_instance_max = 2000;

	// 描画管理インスタンスの生成
	_manager = ::Effekseer::Manager::Create(effect_instance_max);
	assert(_manager);

	_renderer = ::EffekseerRendererDX11::Renderer::Create(dev.Device(), dev.Context(), effect_instance_max);
	assert(_renderer);


	// 描画方法の指定、独自に拡張しない限り定形文です。
	_manager->SetSpriteRenderer(_renderer->CreateSpriteRenderer());
	_manager->SetRibbonRenderer(_renderer->CreateRibbonRenderer());
	_manager->SetRingRenderer(_renderer->CreateRingRenderer());
	// テクスチャ画像の読込方法の指定(パッケージ等から読み込む場合拡張する必要があります。)
	_manager->SetTextureLoader(_renderer->CreateTextureLoader());
	// 座標系の指定(RHで右手系、LHで左手系)
	_manager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);

	Effekseer::Vector3D();
	_renderer->SetProjectionMatrix(
		Effekseer::Matrix44().PerspectiveFovLH(XM_PIDIV2,
		static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT), 0.1f, 100.0f));

	//カメラ行列
	_renderer->SetCameraMatrix(
		Effekseer::Matrix44().LookAtLH(Effekseer::Vector3D(0,15,-25), Effekseer::Vector3D(0.0f, 10.0f, 0.0f),
		Effekseer::Vector3D(0.0f, 1.0f, 0.0f)));

	_effect=Effekseer::Effect::Create(_manager, (const EFK_CHAR*)(L"sample.efk"), 1.0f, 0);
	assert(_manager);
	
	int test = 0;

}


Effect::~Effect()
{
}

void 
Effect::Update()
{
	_manager->Update();



	_renderer->BeginRendering();
	_manager->Draw();
	_renderer->EndRendering();

}
void 
Effect::Emit()
{
	_currentPlayingHandle = _manager->Play(_effect, 0, 0, -1);
}
void
Effect::Emit(XMFLOAT3 pos)
{
	_currentPlayingHandle = _manager->Play(_effect, pos.x, pos.y, pos.z);
}

void
Effect::Move(XMFLOAT3& vec)
{
	Effekseer::Vector3D pos = _manager->GetLocation(_currentPlayingHandle);
	pos.X += vec.x;
	pos.Y += vec.y;
	pos.Z += vec.z;
	_manager->SetLocation(_currentPlayingHandle, pos);
}

void
Effect::SetCamera(const XMMATRIX& cam, const XMMATRIX& proj)
{
	Effekseer::Matrix44 effproj, effview;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			effproj.Values[j][i] = proj.m[j][i];
			effview.Values[j][i] = cam.m[j][i];
		}
	}
	_renderer->SetCameraMatrix(effview);
	_renderer->SetProjectionMatrix(effproj);
}