#include "GameMain.h"
#include"Scene.h"
#include<d3d11.h>
#include<d3dx11.h>
#include"DeviceDx11.h"
#include"Define.h"
#include<vector>
#include"PlayingScene.h"

static int g_mouseWheel = 0;
static int g_zDelta = 0;

//ウィンドウプロシージャ
LRESULT CALLBACK WindowProcedure(
	HWND hwnd,
	UINT msg,
	WPARAM wpal,
	LPARAM lpal)
{
	g_mouseWheel = 0;
	g_zDelta = 0;
	if (msg == WM_DESTROY){
		PostQuitMessage(0);
		return 0;
	}
	if (msg == WM_ACTIVATE)
	{
		return 0;
	}

	if (msg == WM_MOUSEWHEEL)
	{
		unsigned short fwKeys = GET_KEYSTATE_WPARAM(wpal);
		int zDelta = GET_WHEEL_DELTA_WPARAM(wpal);
		g_zDelta = zDelta;

		g_mouseWheel = zDelta / WHEEL_DELTA;
		
		if (g_mouseWheel != 0)
		{
			g_mouseWheel = g_mouseWheel > 0 ? 1 : -1;
		}

	}
	
	return DefWindowProc(hwnd, msg, wpal, lpal);
}




HWND InitWindow()
{
	//ウィンドウクラスの登録
	WNDCLASS w = WNDCLASS();
	w.lpfnWndProc = (WNDPROC)WindowProcedure;
	w.lpszClassName = "ClassName";
	w.hInstance = GetModuleHandle(0);
	RegisterClass(&w);//ウィンドウクラス登録
	//ウィンドウサイズ設定
	RECT wrc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
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
	float color[] = { 0.0f, 0.0f, 0.0f, 0.0f };

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

		_scene->Update(g_zDelta);

	}

}

void
GameMain::ChangeScene(Scene* scene)
{
	delete _scene;
	_scene = scene;
}
