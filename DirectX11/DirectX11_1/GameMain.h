#pragma once
#include"D3D11.h"

class Scene;


class GameMain
{
private:
	GameMain();
	GameMain(const GameMain&);
	GameMain& operator =(const GameMain&);

	Scene* _scene = nullptr;
	HWND _hwnd;
public:
	~GameMain();

	static GameMain& Instance(){
		static GameMain game;
		return game;
	}

	void GameLoop();

	void ChangeScene(Scene* scene);

	

	ID3D11Texture2D* texDepth;

};

