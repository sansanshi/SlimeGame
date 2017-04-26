#include<Windows.h>
#include"GameMain.h"
#include"Define.h"

//なんかのクラスのメンバ変数or newされた変数は16バイトアライメントが保証されない
//XMMATRIXとかXMFLOATとかは16バイトアライメントが保証されなければ正常に動作しない
//一度ローカル変数に代入する等で回避する




//void MatrixTransmission(int idx, XMMATRIX& parent, std::vector<XMMATRIX>& boneMatrixies, std::vector<BoneNode>& boneTree);


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{

	GameMain& gm = GameMain::Instance();

	gm.GameLoop();
	

	return 0;
}