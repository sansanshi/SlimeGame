#include "PlayingScene.h"
#include"ShaderDefine.h"
#include"Player.h"
#include"DecalBox.h"
#include"Plane.h"
#include"TessPlane.h"
#include"Cylinder.h"
#include"Sphere.h"
#include"SkySphere.h"
#include"DecalFactory.h"
#include"Camera.h"
#include"Renderer.h"
#include"Billboard.h"
#include"HUD.h"
#include"InputManager.h"
#include"ResourceManager.h"


//DirectX11�������֐�

HRESULT
PlayingScene::InitDirect3D(HWND hwnd)
{
	DeviceDx11& dev = DeviceDx11::Instance();
	HRESULT result = dev.Init(hwnd);

	//�o�b�N�o�b�t�@�̃����_�[�^�[�Q�b�g�r���[���쐬
	ID3D11Texture2D *pBack;
	//�o�b�N�o�b�t�@�̃T�[�t�F�C�X�������[�����\�[�X�i�e�N�X�`���j�Ƃ��Ĕ����o��
	dev.SwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBack);
	//���̃e�N�X�`���������_�[�^�[�Q�b�g�Ƃ���悤�ȃ����_�[�^�[�Q�b�g�r���[���쐬
	dev.Device()->CreateRenderTargetView(pBack, nullptr, &g_pRTV);
	pBack->Release();

	//�f�v�X�X�e���V���r���[�iDSV�j���쐬
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = (unsigned int)WINDOW_WIDTH;
	descDepth.Height = (unsigned int)WINDOW_HEIGHT;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	dev.Device()->CreateTexture2D(&descDepth, nullptr, &g_pDS);
	dev.Device()->CreateDepthStencilView(g_pDS, nullptr, &g_pDSV);

	//D3D11_VIEWPORT vp;
	_viewport.Width = WINDOW_WIDTH;
	_viewport.Height = WINDOW_HEIGHT;
	_viewport.MinDepth = 0.0f;
	_viewport.MaxDepth = 1.0f;
	_viewport.TopLeftX = 0;
	_viewport.TopLeftY = 0;
	dev.Context()->RSSetViewports(1, &_viewport);

	//�����_�[�^�[�Q�b�g�r���[�ƃf�v�X�X�e���V���r���[���Z�b�g
	dev.Context()->OMSetRenderTargets(1, &g_pRTV, g_pDSV);


	//�A���t�@�L���ׂ̈̐ݒ�
	ID3D11BlendState* blendState = nullptr;
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	D3D11_RENDER_TARGET_BLEND_DESC& blrtdesc = blendDesc.RenderTarget[0];
	blrtdesc.BlendEnable = true;
	blrtdesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blrtdesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blrtdesc.BlendOp = D3D11_BLEND_OP_ADD;
	blrtdesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	blrtdesc.DestBlendAlpha = D3D11_BLEND_ZERO;
	blrtdesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blrtdesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	dev.Device()->CreateBlendState(&blendDesc, &blendState);
	dev.Context()->OMSetBlendState(blendState, blendFactor, 0xffffffff);

	return S_OK;
}

void
PlayingScene::Init()
{
	HRESULT result;
	DeviceDx11& dev = DeviceDx11::Instance();

}

//���ʁA�V�����_���������q�ŏ��������Ă�̂͌�łǂ��ɂ�����
//GameMain����E�B���h�E�n���h�����󂯎���Ă���
//�ŏ���Direct3D�̏����������邽�߂�_result����ď������q��InitDirecct3D���Ă�
PlayingScene::PlayingScene(HWND hwnd) 
	:_hwnd(hwnd),_result(InitDirect3D(_hwnd)),dev(DeviceDx11::Instance())
{
	//InitDirect3D(_hwnd);//�������q�Ŋ��ɌĂ�ł���
	Init();
	_camera = std::make_shared<Camera>();
	_renderer = std::make_unique<Renderer>();
	_renderer->Init();//�����_���[������

	_player = std::make_unique<Player>(_camera);
	_player->Init();

	_plane = std::make_unique<Plane>(600, 600, Vector3(0, 1, 0), _camera);
	//_plane = new Plane(300, 300, Vector3(0, 1, 0), &_camera);
	_cylinder = std::make_unique<Cylinder>(4, 20, 20, _camera);// new Cylinder(4, 20, 20, _camera);

	_cylinder2 = std::make_unique<Cylinder>(4, 20, 20, _camera);
	_cylinder3 = std::make_unique<Cylinder>(4, 20, 20, _camera);
	_cylinder4 = std::make_unique<Cylinder>(4, 20, 20, _camera);

	_sphere = std::make_unique<Sphere>(100, 5, _camera);//new Sphere(100, 5, _camera);
	_tessPlane = std::make_unique<TessPlane>(600, 600, Vector3(0, 1, 0), _camera);//new TessPlane(400, 400, Vector3(0, 1, 0), _camera);
	_decalBox = std::make_unique<DecalBox>(1, 1, 1, _camera);//new DecalBox(1, 1, 1, _camera);

	_skySphere = std::make_unique<SkySphere>(100, SKYSPHERE_RADIUS, _camera);//new SkySphere(100, SKYSPHERE_RADIUS, _camera);

	_soundManager.Init();//�T�E���h�}�l�[�W��������

	HRESULT result;

	//�f�J�[���t�@�N�g��
	_decalFac = std::make_unique<DecalFactory>(_camera);//new DecalFactory(_camera);
	_decalBoxPitch = 45.0f*XM_PI / 180.0f;
	_oldPitch = _decalBoxPitch;

	_isLockCursor = true;
	_cursorPoint = { 0 };
	_oldCursorPoint = { 0 };

	_billBoard = std::make_unique<Billboard>(_camera, 10, 10);//new Billboard(_camera,10,10);
	_debugHUD = std::make_unique<HUD>(_camera, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);//new HUD(_camera,0,0,320,240);
	_makerHUD = std::make_unique<HUD>(_camera, -8, -8, 16, 16);// new HUD(_camera, -8, -8, 16, 16);


	_effect.Emit();
	_effectMov = { 0, 0, 0 };
	_effect.SetCamera(_camera->CameraView(), _camera->CameraProjection());

	//�r���[�s��͉�]�ƕ��s�ړ������Ȃ��i�g��͂Ȃ�
	//��]�s��͓]�u����Ƌt�s��̐���������
	//�r���[�s��̉�]�ƕ��s�ړ��݂͌��Ɋ����Ȃ��̂ŁA�]�u�s��ɂ���Ƌt�s��ɂȂ�
	//Inverse�֐����g��Ȃ��Ă����̂ŒP���ɓ]�u�����������y��

	ResourceManager& resourceMgr = ResourceManager::Instance();
	_makerSRV = resourceMgr.LoadSRV("Maker_main", "marker.dds");
	//D3DX11CreateShaderResourceViewFromFile(dev.Device(), "marker.dds", nullptr, nullptr, &_makerSRV, &result);


	
	
	//�V�F�[�_�Ƀt���[���P�ʂœn���萔�o�b�t�@
	unsigned int globalTimer = 0;
	_shaderGlobals.eyePos = _camera->GetEyePos();//XMFLOAT4(eyePoint.x, eyePoint.y, eyePoint.z, 1);//XMFLOAT3(gazePoint.x - eyePoint.x, gazePoint.y - eyePoint.y, gazePoint.z - eyePoint.z);//Vector3(gazePoint.x - eyePoint.x, gazePoint.y - eyePoint.y, gazePoint.z - eyePoint.z).Normalize();
	_shaderGlobals.lightPos = _camera->GetLightPos();//w�v�f��1�Ȃ�|�C���g���C�g�A0�Ȃ�f�B���N�V���i�����C�g�H
	_shaderGlobals.timer = 0;
	_shaderGlobals.nearZ = NEAR_Z;
	_shaderGlobals.farZ = FAR_Z;

	_shaderGlobals.fog
		= XMFLOAT2(FOG_END / (FOG_END - FOG_START), -1.0f / (FOG_END - FOG_START));
	_shaderGlobals.fogColor
		= FOG_COLOR;
	
	_shaderGlobals.windowSize.x = WINDOW_WIDTH;
	_shaderGlobals.windowSize.y = WINDOW_HEIGHT;

	_shaderGlobals.directionalLightVec = DIRECTIONAL_LIGHT_VECTOR;


	D3D11_BUFFER_DESC globalBuffDesc;
	globalBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	globalBuffDesc.ByteWidth = sizeof(ShaderGlobals) + (16 - sizeof(ShaderGlobals) % 16);
	int gh = sizeof(ShaderGlobals) + (16 - sizeof(ShaderGlobals) % 16);
	globalBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	globalBuffDesc.Usage = D3D11_USAGE_DYNAMIC;
	globalBuffDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA globalData;
	globalData.pSysMem = &_shaderGlobals;

	result = dev.Device()->CreateBuffer(&globalBuffDesc, &globalData, &_globalBuffer);


	dev.Context()->Map(_globalBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mappedGlobals);
	//�����ł��̃������̉�ɁA�}�g���b�N�X�̒l���R�s�[���Ă��
	memcpy(_mappedGlobals.pData, (void*)(&_shaderGlobals), sizeof(ShaderGlobals));
	
	dev.Context()->Unmap(_globalBuffer, 0);

	dev.Context()->VSSetConstantBuffers(5, 1, &_globalBuffer);

	debugToggle = true;
}


PlayingScene::~PlayingScene()
{

	_soundManager.Terminate();
}


void
PlayingScene::Update(int mouseWheelDelta)
{
	_soundManager.Update();

	

#pragma region ���́i��
	std::copy(keystate, keystate + sizeof(keystate), lastkeystate);
	GetKeyboardState(keystate);


	//�}�E�X�J�[�\���ʒu�Z�o
	/*_oldCursorPoint = _cursorPoint;
	GetCursorPos(&_cursorPoint);
	ScreenToClient(_hwnd, &_cursorPoint);
	POINT center = { WINDOW_WIDTH / 2,WINDOW_HEIGHT / 2 };
	ClientToScreen(_hwnd, &center);
	
	SetCursorPos(center.x,center.y);*/

	

	if (keystate['M']&0x80)
	{
		if (!(lastkeystate['M']&0x80))
		{
			debugToggle = !debugToggle;
		}
	}

	if (keystate['L'] & 0x80)
	{
		if (!(lastkeystate['L'] & 0x80))
		{
			_effect.Emit();
		}
	}
	if (keystate[VK_SPACE] & 0x80)
	{
		_camera->Move(0.0f, SPEED_RISING, 0.0f);
	}
	if (keystate[VK_SHIFT] & 0x80)
	{
		_camera->Move(0.0f, -SPEED_RISING, 0.0f);
	}

#pragma region �f�J�[���{�b�N�X��]

	int mouseWheel = mouseWheelDelta / WHEEL_DELTA;
	if (keystate['R'] & 0x80)
	{
		if (mouseWheel != 0)
		{
			_decalBoxPitch += mouseWheel*5*XM_PI / 180.0f;
			int jk = 0;
			
		}
	}
	if (keystate['X'] & 0x80)
	{
		if (mouseWheel != 0)
		{
			_decalBox->ChangeScale(mouseWheel*0.5f);

		}
	}
	/*if (keystate[VK_ADD] & 0x80)
	{
		_decalBoxPitch += 1 * XM_PI / 180.0f;
	}
	if (keystate[VK_SUBTRACT] & 0x80)
	{
		_decalBoxPitch -= 1.0f*XM_PI / 180.0f;
	}*/
	
	XMFLOAT3 test = _camera->GetRotation();
	_decalBox->SetPYR(XMFLOAT3(_decalBoxPitch, _camera->GetRotation().y, 0));
	if (_decalBoxPitch - _oldPitch > 0.000f)
	{
		int jj = 0;
	}

#pragma endregion

	if (keystate[VK_LBUTTON]&0x80)//�}�E�X��
	{
		//���C���v�Z����@far - near  XMVectorUnProj~~
		//d �J�����̃|�W�V�����ƕ��ʂ̖@���̓��ς��Ƃ�
		//t ���C�ƕ��ʂ̖@���̓��ς��Ƃ�
		//
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(_hwnd, &p);


		XMVECTOR ray = _camera->CalculateCursorVector(p.x, p.y);
		ray = XMVector3Normalize(ray);

		XMVECTOR camerapos = XMLoadFloat3(&_camera->GetPos());

		float d;
		XMStoreFloat(&d, XMVector3Dot(camerapos, _plane->Normal()));
		float t;
		XMStoreFloat(&t, XMVector3Dot(-ray, _plane->Normal()));

		ray = XMVectorScale(ray, d / t);

		XMFLOAT3 pos;
		XMStoreFloat3(&pos, camerapos + ray);
		//pos.y += 5;
		
		//_effect.Emit(pos);

		//�f�J�[���{�b�N�X�̈ʒu�Z�b�g
		_decalBox->SetPos(pos);
		int j = 0;
		
		if (keystate[VK_RBUTTON] & 0x80)
		{
			if (!(lastkeystate[VK_RBUTTON] & 0x80)) 
			{
				_decalFac->CreateDecalBox(_decalBox->GetWorldMatrix());
				//_decalFac->CreateDecalBox(_decalBox.GetPos(), _decalBox.GetRotation(), _decalBox.GetScale());
			}
		}

	}

	float moveFront = 0.0f, moveRight = 0.0f;
	if (keystate['W'] & 0x80)
	{
		moveFront += 1.0f;//_camera->MoveTPS(0.5f, 0);
	}
	if (keystate['S'] & 0x80)
	{
		moveFront -= 1.0f;//_camera->MoveTPS(-0.5f, 0);
	}
	if (keystate['A'] & 0x80)
	{
		moveRight -= 1.0f;//_camera->MoveTPS(0, -0.5f);
	}
	if (keystate['D'] & 0x80)
	{
		moveRight += 1.0f;//_camera->MoveTPS(0, 0.5f);
	}
	_camera->MoveTPS(moveFront, moveRight);

	XMFLOAT3 camRot = {0.0f,0.0f,0.0f};//���ꂼ��x,y,x����̉�]
	if (keystate[VK_NUMPAD6]&0x80/*||keystate['X']&0x80*/)
	{
		camRot.y += 1;
	}
	if (keystate[VK_NUMPAD4] & 0x80/*||keystate['Z']&0x80*/)
	{
		camRot.y -= 1;
	}
	if (keystate[VK_NUMPAD2] & 0x80)
	{
		camRot.x += 1;
	}
	if (keystate[VK_NUMPAD8] & 0x80)
	{
		camRot.x -= 1;
	}
	float calcRadian= XM_PI / 180.0f;
	camRot.x *= calcRadian;
	camRot.y *= calcRadian;
	if (camRot.x != 0 || camRot.y != 0 || camRot.z != 0)
	{
		_camera->Rotate(camRot);
	}


#pragma endregion

	//�V�F�[�_�ɓn���^�C�}�[�̍X�V
	_shaderGlobals.timer++;
	_shaderGlobals.eyePos = _camera->GetEyePos();//XMFLOAT4(eyePoint.x, eyePoint.y, eyePoint.z, 1);//XMFLOAT3(gazePoint.x - eyePoint.x, gazePoint.y - eyePoint.y, gazePoint.z - eyePoint.z);//Vector3(gazePoint.x - eyePoint.x, gazePoint.y - eyePoint.y, gazePoint.z - eyePoint.z).Normalize();
	_shaderGlobals.lightPos = _camera->GetLightPos();//w�v�f��1�Ȃ�|�C���g���C�g�A0�Ȃ�f�B���N�V���i�����C�g�H
	//shaderGlobals.dummy0++;
	dev.Context()->Map(_globalBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mappedGlobals);
	memcpy(_mappedGlobals.pData, (void*)(&_shaderGlobals), sizeof(ShaderGlobals));
	dev.Context()->Unmap(_globalBuffer, 0);
	dev.Context()->VSSetConstantBuffers(5, 1, &_globalBuffer);


	_camera->Update();

	_player->Update();
	_plane->Update();

	_cylinder2->SetPos(XMFLOAT3(10,0,0));
	_cylinder3->SetPos(XMFLOAT3(10, 0, 10));
	_cylinder4->SetPos(XMFLOAT3(-10, 0, 10));
	_cylinder2->SetScale(XMFLOAT3(10, 1, 3));

	_cylinder->Update();
	_cylinder2->Update();
	_cylinder3->Update();
	_cylinder4->Update();
	_tessPlane->Update();
	_sphere->Update();
	_decalBox->Update();
	_decalFac->Update();
	_skySphere->SetPos(_camera->GetPos());
	_skySphere->Update();
	_billBoard->Update();
	_debugHUD->Update();
	_makerHUD->Update();

	float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };


	_renderer->ChangePTForPMD();
	_renderer->CullBack();


#pragma region ���C�g�r���[�`��
	_renderer->ChangeRT_Light();

	//_renderer->ZWriteOff();
	_renderer->ChangePTForPrimitive();
	//_plane.DrawLightView();
	_tessPlane->DrawLightView();
	_cylinder->DrawLightView();
	_cylinder2->DrawLightView();
	_cylinder3->DrawLightView();
	_cylinder4->DrawLightView();

	_renderer->ChangePTForPMD();
	_player->DrawLightView();
	_sphere->DrawLightView();
	//_renderer->ZWriteOn();

#pragma endregion

#pragma region �J�����f�v�X�`��
	_renderer->ChangeRT_CameraDepth();
	_player->DrawCameraDepth();
	//_plane.DrawCameraDepth();
	_tessPlane->DrawCameraDepth();
	_cylinder->DrawCameraDepth();
	_cylinder2->DrawCameraDepth();
	_cylinder3->DrawCameraDepth();
	_cylinder4->DrawCameraDepth();
	_sphere->DrawCameraDepth();

#pragma endregion
	

#pragma region �J�����r���[�`��
	//_renderer->ChangeRT_Camera();
	_renderer->ChangeRT_PostEffect();
	_renderer->ZWriteOn();
	_skySphere->Draw();
	//���C�g�r���[����̃����_�����O���ʂ��e�N�X�`���Ƃ���GPU�ɓn��
	//SetRenderTarget�������SetShaderResources���Ȃ��Ɠn��Ȃ�
	ID3D11ShaderResourceView* resource = _renderer->LightDepthShaderResource();
	dev.Context()->PSSetShaderResources(TEXTURE_LIGHT_DEPTH, 1, &resource);


	_renderer->ChangePTForPMD();
	_player->Draw();//�v���C���[�̃��C���`��

	_renderer->ChangePTForPrimitive();
	_cylinder->Draw();//��
	_cylinder2->Draw();
	_cylinder3->Draw();
	_cylinder4->Draw();

	
	_renderer->ChangePTForPMD();
	dev.Context()->DSSetConstantBuffers(5, 1, &_globalBuffer);
	_tessPlane->Draw();//�e�b�Z���[�V��������

	//�f�J�[���{�b�N�X�����_�V�F�[�_�Ő[�x�o�b�t�@�e�N�X�`���g���̂œn��
	resource = _renderer->LightDepthShaderResource();
	dev.Context()->VSSetShaderResources(TEXTURE_CAMERA_DEPTH, 1, &resource);


	_renderer->CullNone();//���ʕ`�悳����
	_sphere->Draw();//�������X���C��

	//�J��������̐[�x�o�b�t�@�e�N�X�`��
	resource = _renderer->CameraDepthShaderResource();
	dev.Context()->PSSetShaderResources(TEXTURE_CAMERA_DEPTH, 1, &resource);
	_renderer->ZWriteOff();
	_decalFac->Draw();//�f�J�[���{�b�N�X
	_decalBox->DebugDraw();//�f�o�b�O�p�f�J�[���{�b�N�X
	_renderer->ZWriteOn();
	_renderer->CullBack();
	_plane->Draw();//��
	_billBoard->Draw();

#pragma endregion


	
	_effect.SetCamera(_camera->CameraView(), _camera->CameraProjection());
	_effect.Update();

#pragma region HUD�`��
	_renderer->ChangeRT_Camera();
	if (debugToggle)
	{
		resource = _renderer->TestShaderResource();//_renderer->CameraDepthShaderResource();
		dev.Context()->PSSetShaderResources(TEXTURE_LIGHT_DEPTH, 1, &resource);
		_debugHUD->Draw();

	}
#pragma endregion

	{
		//ik�{�[���̏ꏊ����Ă���
		int ikboneIdx = _player->GetMesh()->GetBoneMap()["�E���h�j"];
		XMFLOAT3 ikpos = _player->GetMesh()->
			GetBoneVertices()[ikboneIdx].head.pos;
		ikpos = _player->IKPos();
		//���̍��W�ɑ΂����i�{�[���s��܂ށj�A���A���|���Ă��Ŋ���΃N���b�s���O��ԏ�̈ʒuposCS�����܂�
		//posCS�̓E�B���h�E�̉𑜓xSw,Sh����ɂ��ꂼ��x=-1~1��0~Sw,h=-1~1��0~Sh�ɂł���
		//�o�Ă���x,y�̒l���I�t�Z�b�g������s����}�[�J�[�̒��_�ɂ����Ă��
		//XMMATRIX bonemat = _player.GetMesh()->BoneMatrixies()[ikboneIdx];
		XMMATRIX world = _player->GetModelMatrix();
		//XMMATRIX wvp = XMMatrixMultiply(bonemat,world);
		XMMATRIX view = _camera->CameraView();
		XMMATRIX proj = _camera->CameraProjection();
		XMMATRIX cam = XMMatrixMultiply(view, proj);
		XMMATRIX wvp = XMMatrixMultiply(world, cam);

		XMFLOAT4 ikposCS;
		XMVECTOR vec;
		vec = XMVector3Transform(XMLoadFloat3(&ikpos), wvp);
		XMStoreFloat4(&ikposCS, vec);
		ikposCS = XMFLOAT4(ikposCS.x / ikposCS.w, ikposCS.y / ikposCS.w, ikposCS.z / ikposCS.w, ikposCS.w / ikposCS.w);

		XMFLOAT2 ikScreenPos;
		ikScreenPos.x = (1.0f + ikposCS.x) / 2.0f*WINDOW_WIDTH;
		ikScreenPos.y = (1.0f - ikposCS.y) / 2.0f*WINDOW_HEIGHT;

		//���CreateHudMatrix�̈�����offsetX,offsestY�ǉ����č�蒼��
		XMMATRIX screenOfsMatrix = XMMatrixTranslation(ikScreenPos.x, ikScreenPos.y, 0);



		_renderer->ChangePTForPrimitive();
		dev.Context()->PSSetShaderResources(TEXTURE_LIGHT_DEPTH, 1, _makerSRV._Get());
		_makerHUD->Offset(ikScreenPos.x, ikScreenPos.y);
		_makerHUD->Draw();
	}

	dev.SwapChain()->Present(1, 0);
}

void
PlayingScene::LockCursorToggle()
{
	_isLockCursor = !_isLockCursor;
	if (_isLockCursor)
	{
		POINT center = { WINDOW_WIDTH / 2,WINDOW_HEIGHT / 2 };
		ClientToScreen(_hwnd, &center);

		_cursorPoint = center;
		_oldCursorPoint = center;
		SetCursorPos(center.x, center.y);
		ShowCursor(false);
	}
	else
	{
		ShowCursor(true);
	}
}
