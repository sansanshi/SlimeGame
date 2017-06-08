#pragma once
#include<D3D11.h>
#include<D3DX11.h>
#include"DeviceDx11.h"
#include<vector>
#include<map>
#include<string>
#include"PMDMesh.h"
#include"PMDLoader.h"
#include"VMDData.h"
#include"VMDLoader.h"
#include"Define.h"
#include"Camera.h"


class PMDMesh;


//ボーンの木の節
struct BoneNode
{
	unsigned short child;
	unsigned short sibling;
};

class Player
{
	//malloc free　メモリを確保する
	//コンストラクタ走らない
	//new　delete　メモリを○○の型で確保する
	//コンストラクタ走る

private:
	std::string _folderPath;
	PMDMesh* _mesh;
	DeviceDx11& dev;
	ID3D11InputLayout* _vertInputLayout;
	ID3D11InputLayout* _boneInputLayout;

	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;

	ID3D11VertexShader* _boneVertexShader;
	ID3D11PixelShader* _bonePixelShader;

	ID3D11VertexShader* _depthVS;
	ID3D11InputLayout* _depthViewInputLayout;
	ID3D11PixelShader* _depthPS;

	std::map<std::string, std::vector<VMDData::MotionData>> _keyframes;

	XMFLOAT3 eyePoint;
	XMFLOAT3 gazePoint;
	XMFLOAT3 upVec;

	Camera& _cameraRef;
	//MATRIXIES _matrixMVP;
	WorldAndCamera _worldAndCamera;
	XMFLOAT3 _pos;
	XMMATRIX _transMatrix;
	XMMATRIX _rotMatrix;
	XMMATRIX _scaleMatrix;

	ID3D11Buffer* _matrixBuffer;
	D3D11_MAPPED_SUBRESOURCE _mem;//マップして取ってくるメモリの塊


	ShaderGlobals _shaderGlobals;
	ID3D11Buffer* _globalBuffer;
	D3D11_MAPPED_SUBRESOURCE _mappedGlobals;


	Material _material;
	ID3D11Buffer* _materialBuffer;
	D3D11_MAPPED_SUBRESOURCE _mappedMaterial;

	//↓2つはPMDMeshに持たせる
	//XMMATRIX _boneMatrixies[512];
	//ID3D11Buffer* _boneMatrixBuff;
	D3D11_MAPPED_SUBRESOURCE _mappedBones;

	std::vector<BoneNode> _boneTree;

	float rotAngle;

	BYTE _key[256];
	BYTE _oldkey[256];
	bool _isBoneView = false;

	VMDData* _vmd;

	std::map<std::string, VMDData*> _animations;

	unsigned int _frameNo;

	ID3D11ShaderResourceView* _normalTex;

	ID3D11ShaderResourceView* _decalTex;

	VMDData* _currentVMD;

	//IKボーンオフセットの確認用変数
	XMFLOAT3 _ikpos;
	XMFLOAT3 _ikOffset;



public:
	Player(Camera&);
	~Player();
	PMDMesh* GetMesh(){ return _mesh; };

	void RegisterAnim(std::string, VMDData*);

	void Init();

	void Update();

	void Draw();
	void DrawBone();

	void DrawLightView();

	//カメラからの深度描画
	void DrawCameraDepth();

	void SetAnimation(const char*);

	XMMATRIX& GetModelMatrix(){
		return _worldAndCamera.world;
	};

	const XMFLOAT3& IKPos(){ return _ikpos; };

	void CcdIkSolve(PMDMesh& mesh,const std::string& ikName,XMFLOAT3& offset);

	//アニメーションをブレンドする　tは0.0~1.0
	void BlendAnimation(const char* animName,float t);

	const XMFLOAT3& GetPos() { return _pos; };
};

