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
	::EffekseerRendererDX11::Renderer* _renderer;//�����_���[
	::Effekseer::Manager* _manager;//�G�t�F�N�g�Ǘ�
	::Effekseer::Effect*_effect;//���ݍĐ����̃G�t�F�N�g

	float _windowWidth;
	float _windowHeight;

	int _currentPlayingHandle;

public:
	Effect();
	~Effect();

	//Effekseer�ɂ��G�t�F�N�g�̍X�V�ƕ`����s��
	void Update();
	//Effekseer�G�t�F�N�g�𔭐�������
	void Emit();
	void Emit(XMFLOAT3 pos);

	void Move(XMFLOAT3&);

	void SetCamera(const XMMATRIX& cam, const XMMATRIX& proj);
};

