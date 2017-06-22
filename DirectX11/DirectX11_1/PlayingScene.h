#pragma once
#include "Scene.h"
#include"Define.h"
#include"Camera.h"
#include"Renderer.h"
#include"Effect.h"
#include"SoundManager.h"
#include"DeviceDx11.h"
#include<memory>

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

class PlayingScene :
	public Scene
{
private:
	HWND _hwnd;
	DeviceDx11& dev;
	HRESULT _result;

	ID3D11Buffer* _globalBuffer;
	std::unique_ptr<Renderer> _renderer;
	Camera _camera;
	
	void Init();
	//Player* _player;
	std::unique_ptr<Player> _player;
	std::unique_ptr<Plane> _plane;
	Cylinder* _cylinder;
	Sphere* _sphere;
	TessPlane* _tessPlane;
	SkySphere* _skySphere;

	DecalBox* _decalBox;
	DecalFactory* _decalFac;
	float _decalBoxPitch;

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

	//マーカー頂点バッファ（IKボーン位置表示
	ID3D11Buffer* _makerBuffer;
	ID3D11ShaderResourceView* _makerSRV;

	//HUD
	ID3D11Buffer* _hudBuffer;
	ID3D11VertexShader* _hudVS;
	ID3D11PixelShader* _hudPS;
	ID3D11InputLayout* _hudInputLayout;
	XMMATRIX _hudMatrix;
	bool debugToggle;


	//ビルボードテスト
	ID3D11Buffer* billBoardBuffer;
	ID3D11ShaderResourceView* billBoardSRV;
	ID3D11VertexShader* billBoardVS;
	ID3D11PixelShader* billBoardPS;
	ID3D11InputLayout* billBoardInputLayout;

	Billboard* _billBoard;
	HUD* _debugHUD;
	HUD* _makerHUD;



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

	void Update();

	//float* clearColor;

	HRESULT InitDirect3D(HWND hwnd);

	ID3D11RenderTargetView* g_pRTV;
	ID3D11Texture2D* g_pDS;
	ID3D11DepthStencilView* g_pDSV;

	
};

