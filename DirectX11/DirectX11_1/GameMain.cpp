#include "GameMain.h"
#include"Scene.h"
#include<d3d11.h>
#include<d3dx11.h>
#include"DeviceDx11.h"
#include"Define.h"
#include<vector>
#include"PlayingScene.h"


//ウィンドウプロシージャ
LRESULT CALLBACK WindowProcedure(
	HWND hwnd,
	UINT msg,
	WPARAM wpal,
	LPARAM lpal)
{
	if (msg == WM_DESTROY){
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wpal, lpal);
}



//void MatrixTransmission(int idx, XMMATRIX& parent, std::vector<XMMATRIX>& boneMatrixies, std::vector<BoneNode>& boneTree);





HWND InitWindow()
{
	//ウィンドウクラスの登録
	WNDCLASS w = WNDCLASS();
	w.lpfnWndProc = (WNDPROC)WindowProcedure;
	w.lpszClassName = "ClassName";
	w.hInstance = GetModuleHandle(0);
	RegisterClass(&w);//ウィンドウクラス登録
	//ウィンドウサイズ設定
	RECT wrc = { 0, 0, 640, 480 };
	::AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//ウィンドウの生成
	HWND hwnd = CreateWindow(w.lpszClassName,
		"1401349澁谷紘輝",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr);

	return hwnd;
}

GameMain::GameMain()
{
	_hwnd = InitWindow();

	ChangeScene(new PlayingScene(_hwnd));
}


GameMain::~GameMain()
{
}

void
GameMain::GameLoop()
{
	DeviceDx11& dev = DeviceDx11::Instance();

	MSG msg;
	float color[] = { 0.5, 0.5, 0.5, 1.0 };

	ShowWindow(_hwnd, SW_SHOW);

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)){
			if (msg.message == WM_QUIT){
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		_scene->Update();

	}

}

void
GameMain::ChangeScene(Scene* scene)
{
	delete _scene;
	_scene = scene;
}
