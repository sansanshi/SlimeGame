#include "Camera.h"
#include"Define.h"
#include"DeviceDx11.h"
#include"Geometry.h"

const float CAMERA_MOVE_SPD = 0.5f;

Camera::Camera()
{
	Init();
}


Camera::~Camera()
{
}

void 
Camera::Init()
{
	_pitch = 0.0f;
	_yaw = 0.0f;
	_roll = 0.0f;

	_rotAngle = 0.0f;
	_eyePoint = { 0, 20, -25 };//{ 15.3f, 25.3f, -14.3f };
	_gazePoint = { 0, 10, 0 };//{ 0.f, 8.f, 0.f };
	_upVec = { 0.f, 1.0f, 0.f };

	_lightPoint = { -35.3f, 60.3f, -30.3f };

	XMMATRIX view = XMMatrixLookAtLH(
		XMLoadFloat3(&_eyePoint),
		XMLoadFloat3(&_gazePoint),
		XMLoadFloat3(&_upVec));
	_view = view;

	 XMMATRIX cameraProjection = XMMatrixPerspectiveFovLH(
		(45.0f*(XM_PI / 180)),
		WINDOW_WIDTH / WINDOW_HEIGHT,
		NEAR_Z,
		FAR_Z);
	 _cameraProjection = cameraProjection;//構造体アライメント対策

	 float te = WINDOW_WIDTH / WINDOW_HEIGHT;

	 XMMATRIX lightProjection = XMMatrixPerspectiveFovLH(
		 (45.0f*(XM_PI / 180)),
		 WINDOW_WIDTH / WINDOW_HEIGHT,
		 NEAR_Z,
		 FAR_Z);
	 /*XMMATRIX lightProjection = XMMatrixOrthographicLH(
		 50, 50, 1.10f, 100.0f);*/
	 _lightProjection = lightProjection;//構造体アライメント対策

	XMMATRIX lightView = XMMatrixLookAtLH(
		XMLoadFloat3(&_lightPoint),
		XMLoadFloat3(&_gazePoint),
		XMLoadFloat3(&_upVec));
	_light = lightView;
	//_camera->cameraview = XMMatrixMultiply(view, cameraProjection);
	//_camera->cameraview = camera;
	//_camera->lightview = XMMatrixMultiply(lightView, lightProjection);
	//_camera->lightview = light;

	_originEyeVec = { _gazePoint.x - _eyePoint.x, _gazePoint.y - _eyePoint.y, _gazePoint.z - _eyePoint.z };
}

void
Camera::Update()
{
	_rotAngle += 1 * XM_PI / 180;
	//lightPoint.x = 25.3f*cos(_rotAngle);
	//lightPoint.z = 25.3f*sin(_rotAngle);

	//lightPoint.x = 35.3f*cos(_rotAngle);
	//lightPoint.z = 35.3*sin(_rotAngle);


	XMMATRIX view = XMMatrixLookAtLH(
		XMLoadFloat3(&_eyePoint),
		XMLoadFloat3(&_gazePoint),
		XMLoadFloat3(&_upVec));
	_view = view;
	XMMATRIX light = XMMatrixLookAtLH(
		XMLoadFloat3(&_lightPoint),
		XMLoadFloat3(&XMFLOAT3(0,5,0)),
		XMLoadFloat3(&_upVec));
	_light = light;
	int jjk = 0;

}

Vector3
Camera::EyeVec()
{
	//プレイヤの進む方向を決めるために使うのでとりあえず高さは合わせておく
	Vector3 eye,gaze;
	eye = XMFLOAT3(_eyePoint.x, _eyePoint.y, _eyePoint.z);
	gaze = XMFLOAT3(_gazePoint.x, _eyePoint.y, _gazePoint.z);

	Vector3 vec = (gaze - eye).Normalize();
	return vec;
	
}

void
Camera::SetEyeGazeUp(XMFLOAT3& eye, XMFLOAT3& gaze, XMFLOAT3& up)
{
	_eyePoint = eye;
	_gazePoint = gaze;
	_upVec = up;
}

XMVECTOR
Camera::CalculateCursorVector(float x, float y)
{
	DeviceDx11& dev = DeviceDx11::Instance();
	UINT num = 1;
	D3D11_VIEWPORT vp;
	dev.Context()->RSGetViewports(&num, &vp);

	XMMATRIX proj = _cameraProjection;
	XMMATRIX view = _view;

	XMVECTOR temp;

	XMFLOAT3 nearpos = XMFLOAT3(x, y, 1.00f);
	temp = XMLoadFloat3(&nearpos);
	XMVECTOR nearvec = XMVector3Unproject(temp, 0, 0, vp.Width, vp.Height,
		vp.MinDepth, vp.MaxDepth, proj, view, XMMatrixIdentity());
	XMFLOAT3 farpos = XMFLOAT3(x, y, 100.0f);
	temp = XMLoadFloat3(&farpos);
	XMVECTOR farvec = XMVector3Unproject(temp, 0, 0, vp.Width, vp.Height,
		vp.MinDepth, vp.MaxDepth, proj, view,XMMatrixIdentity());
	
	XMStoreFloat3(&nearpos, nearvec);
	XMStoreFloat3(&farpos, farvec);
	XMFLOAT3 ret = XMFLOAT3(farpos.x - nearpos.x, farpos.y - nearpos.y, farpos.z - nearpos.z);
	return XMLoadFloat3(&ret);

}

void
Camera::Move(float x, float y, float z)
{

	_eyePoint.x += x;
	_eyePoint.y += y;
	_eyePoint.z += z;

	_gazePoint.x += x;
	_gazePoint.y += y;
	_gazePoint.z += z;

	
}


void
Camera::Rotate(float pitch, float yaw, float roll)
{
	XMMATRIX xrot = XMMatrixRotationX(pitch);
	XMMATRIX yrot = XMMatrixRotationY(yaw);
	XMMATRIX zrot = XMMatrixRotationZ(roll);

	XMMATRIX rot = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	//視線ベクトルを作る
	XMFLOAT3 v = { _gazePoint.x - _eyePoint.x, _gazePoint.y - _eyePoint.y, _gazePoint.z - _eyePoint.z };
	XMVECTOR ray = XMLoadFloat3(&v); //XMLoadFloat3(&_originEyeVec);//{ _target.x - _pos.x, _target.y - _pos.y, _target.z - _pos.z };
	ray = XMVector3Transform(ray, rot);
	XMFLOAT3 r;
	XMStoreFloat3(&r, ray);

	_gazePoint = { _eyePoint.x + r.x, _eyePoint.y + r.y, _eyePoint.z + r.z };
}
void
Camera::Rotate(const XMFLOAT3& rotPYR)
{
	_pitch += rotPYR.x;
	_yaw += rotPYR.y;
	_roll += rotPYR.z;

	XMVECTOR ray = XMLoadFloat3(&_originEyeVec);
	XMMATRIX rot = XMMatrixRotationRollPitchYaw(_pitch, _yaw, _roll);
	ray = XMVector3TransformCoord(ray, rot);
	XMFLOAT3 r;
	XMStoreFloat3(&r, ray);


	_gazePoint = { _eyePoint.x + r.x, _eyePoint.y + r.y, _eyePoint.z + r.z };
}

void
Camera::MoveTPS(float front, float right)
{
	XMFLOAT3 frontVec = _gazePoint - _eyePoint;
	frontVec.y = 0.0f;
	frontVec = Normalize(frontVec);


	XMFLOAT3 frontMove = frontVec*front*CAMERA_MOVE_SPD;
	XMFLOAT3 rightVec = XMFLOAT3(frontVec.z, 0, -frontVec.x);
	XMFLOAT3 rightMove = rightVec*right*CAMERA_MOVE_SPD;

	XMFLOAT3 moveVec = frontMove + rightMove;
	
	Move(moveVec.x, moveVec.y, moveVec.z);
}