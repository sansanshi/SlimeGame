#include<Windows.h>
#include"GameMain.h"
#include"Define.h"

//�Ȃ񂩂̃N���X�̃����o�ϐ�or new���ꂽ�ϐ���16�o�C�g�A���C�����g���ۏ؂���Ȃ�
//XMMATRIX�Ƃ�XMFLOAT�Ƃ���16�o�C�g�A���C�����g���ۏ؂���Ȃ���ΐ���ɓ��삵�Ȃ�
//��x���[�J���ϐ��ɑ�����铙�ŉ������




//void MatrixTransmission(int idx, XMMATRIX& parent, std::vector<XMMATRIX>& boneMatrixies, std::vector<BoneNode>& boneTree);


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{

	GameMain& gm = GameMain::Instance();

	gm.GameLoop();
	

	return 0;
}