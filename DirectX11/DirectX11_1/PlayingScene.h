#pragma once
#include "Scene.h"
#include"Define.h"
#include"Effect.h"
#include"SoundManager.h"
#include"DeviceDx11.h"
#include<memory>

class Camera;
class Renderer;
class Player;
class Plane;
class Cylinder;
class Sphere;
class TessPlane;
class DecalBox;
class SkySphere;
class DecalFactory;
class Billboard;
class HUD;
class PMDMesh;
class InputManager;

class PlayingScene :
	public Scene
{
private:
	HWND _hwnd;
	DeviceDx11& dev;
	HRESULT _result;

	ID3D11Buffer* _globalBuffer;
	std::unique_ptr<Renderer> _renderer;
	std::shared_ptr<Camera> _camera;
	
	void Init();
	//Player* _player;
	std::unique_ptr<Player> _player;
	std::unique_ptr<Plane> _plane;
	std::unique_ptr<Cylinder> _cylinder;
	std::unique_ptr<Cylinder> _cylinder2;
	std::unique_ptr<Cylinder> _cylinder3;
	std::unique_ptr<Cylinder> _cylinder4;
	std::unique_ptr<Sphere> _sphere;
	std::unique_ptr<TessPlane> _tessPlane;
	std::unique_ptr<SkySphere> _skySphere;

	std::unique_ptr<DecalBox> _decalBox;
	std::unique_ptr<DecalFactory> _decalFac;
	float _decalBoxPitch;
	float _oldPitch;

	std::unique_ptr<InputManager> _inputManager;

	SoundManager _soundManager;

	//マウスカーソル位置保存用変数
	POINT _cursorPoint;
	POINT _oldCursorPoint;
	bool _isLockCursor;
	void LockCursorToggle();
	void UnlockCursor();


	ID3D11RenderTargetView* _rtvlight;
	ID3D11DepthStencilView* _lightDSV;
	ID3D11ShaderResourceView* _shaderResourceViewForShadow;//レンダリング結果をテクスチャとして扱う

	std::weak_ptr<ID3D11ShaderResourceView*> _makerSRV;

	//HUD表示フラグ
	bool debugToggle;


	//ビルボードテスト
	std::unique_ptr<Billboard> _billBoard;
	std::unique_ptr<HUD> _debugHUD;
	std::unique_ptr<HUD> _makerHUD;



	BYTE keystate[256];
	BYTE lastkeystate[256];

	WorldAndCamera _worldAndCamera;
	ID3D11Buffer* _matrixBuffer;
	D3D11_MAPPED_SUBRESOURCE _mem;

	Effect _effect;
	XMFLOAT3 _effectMov;

	D3D11_VIEWPORT _viewport;

	ShaderGlobals _shaderGlobals;
	D3D11_MAPPED_SUBRESOURCE _mappedGlobals;

public:
	PlayingScene(HWND);
	~PlayingScene();

	void Update(int);

	//float* clearColor;

	HRESULT InitDirect3D(HWND hwnd);

	ID3D11RenderTargetView* g_pRTV;
	ID3D11Texture2D* g_pDS;
	ID3D11DepthStencilView* g_pDSV;

	
};

