#pragma once
#include<D3D11.h>
#include<xnamath.h>

namespace Effekseer{
	class Manager;
	class Effect;
}
namespace EffekseerRendererDX11{
	class Renderer;
}


class Effect
{
private:
	::EffekseerRendererDX11::Renderer* _renderer;//レンダラー
	::Effekseer::Manager* _manager;//エフェクト管理
	::Effekseer::Effect*_effect;//現在再生中のエフェクト

	float _windowWidth;
	float _windowHeight;

	int _currentPlayingHandle;

public:
	Effect();
	~Effect();

	//Effekseerによるエフェクトの更新と描画を行う
	void Update();
	//Effekseerエフェクトを発生させる
	void Emit();
	void Emit(XMFLOAT3 pos);

	void Move(XMFLOAT3&);

	void SetCamera(const XMMATRIX& cam, const XMMATRIX& proj);
};

