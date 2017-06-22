#pragma once
#include"D3D11.h"
#include"xnamath.h"
#include"Define.h"

class Player;
class Camera;
class DecalBox;
class DecalFactory;
class InputManager
{
public:
	InputManager();
	~InputManager();

	void Update();
private:
	BYTE _keystate[256];
	BYTE _lastKeystate[256];
	
};

