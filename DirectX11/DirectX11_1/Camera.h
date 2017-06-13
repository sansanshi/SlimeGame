#pragma once
#include"D3D11.h"
#include"xnamath.h"
#include"Define.h"
class Camera
{
public:
	//struct CameraMatrixies{
	//	XMMATRIX cameraview;//カメラビュー＊プロジェクション
	//	XMMATRIX lightview;//ライトビュー＊プロジェクション
	//	CameraMatrixies() 
	//		:cameraview(XMMatrixIdentity()),lightview(XMMatrixIdentity()){};
	//};

	Camera();
	~Camera();
	//CameraMatrixies& GetMatrixies(){ return _camera; };
	void Update();
	void SetEyeGazeUp(XMFLOAT3&, XMFLOAT3&, XMFLOAT3&);
	XMFLOAT4 GetEyePos(){ return XMFLOAT4(eyePoint.x,eyePoint.y,eyePoint.z,1); };
	XMFLOAT4 GetLightPos(){ return XMFLOAT4(lightPoint.x,lightPoint.y,lightPoint.z,1); };

	const XMMATRIX& CameraProjection(){ return _cameraProjection; };
	const XMMATRIX& CameraView(){ return _view; };
	const XMMATRIX& LightView() { return _light; };
	const XMMATRIX& LightProjection() { return _lightProjection; };
	const XMFLOAT3& GetPos(){ return eyePoint; };
	const XMFLOAT3& GetRotation() { return XMFLOAT3(_pitch, _yaw, _roll); };
	Vector3 EyeVec();

	XMVECTOR CalculateCursorVector(float x, float y);

	//@param pitch X軸回転
	//@param yaw Y軸回転
	//@param roll Z軸回転
	void Rotate(float pitch, float yaw, float roll);

	void Rotate(const XMFLOAT3& rot);

	//ワールド基準で動く
	void Move(float x, float y, float z);
	//向いている方向を基準に動く
	void MoveTPS(float front,float right);

private:
	//CameraMatrixies _camera;
	void Init();


	float _rotAngle;

	XMFLOAT3 eyePoint;
	XMFLOAT3 gazePoint;
	XMFLOAT3 upVec;

	XMFLOAT3 lightPoint;
	XMMATRIX _view;
	XMMATRIX _projection;
	XMMATRIX _cameraProjection;
	XMMATRIX _lightProjection;
	XMMATRIX _light;

	XMFLOAT3 _originEyeVec;

	float _pitch;
	float _yaw;
	float _roll;

};

