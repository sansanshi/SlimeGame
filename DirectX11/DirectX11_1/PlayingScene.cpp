#include "PlayingScene.h"
#include"ShaderGenerator.h"



#pragma pack(1)
struct HUDVertex{
	XMFLOAT3 pos;
	XMFLOAT2 uv;
	/*HUDVertex(XMFLOAT3 p, XMFLOAT2 texcoord) :pos(p), uv(texcoord){}*/
};
#pragma pack()

//�r���{�[�h�̒��_�o�b�t�@
//@param �r���{�[�h���SX���W(3D���W�n
//@param�@�r���{�[�h���SY���W
//@param �r���{�[�h���i3D���W�n
//@param �r���{�[�h����(3D���W�n
//@param note ���ƍ����͒��S����̍L���� �����x-width/2,y+height/2
ID3D11Buffer* CreateBillBoardVertexBuffer(float width, float height)
{
	DeviceDx11& dev = DeviceDx11::Instance();

	std::vector<HUDVertex> vertices(4);
	{
		vertices[0] = { XMFLOAT3(-width/2, height/2, 0), XMFLOAT2(0.f, 0.f) };//����
		vertices[1] = { XMFLOAT3(width/2 , height/2 , 0), XMFLOAT2(1.0f, 0.0f) };//�E��
		vertices[2] = { XMFLOAT3(-width/2, -height/2, 0), XMFLOAT2(0.f, 1.f) };//����
		vertices[3] = { XMFLOAT3(width/2, -height/2, 0), XMFLOAT2(1.f, 1.f) };//�E��
	};
	int idxX;
	int idxY;
	idxX / 10.0f;
	idxX + 1 / 10.0f;
	idxY / 3.0f;
	idxY + 1 / 3.0f;
	//idxX = (frame / 4) % 10;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &vertices[0];

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = sizeof(HUDVertex) * 4;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = sizeof(HUDVertex);

	HRESULT result = S_OK;
	ID3D11Buffer* vertexBuffer = nullptr;
	result = dev.Device()->CreateBuffer(&desc, &data, &vertexBuffer);


	XMFLOAT3 pa[4] = {
		vertices[0].pos,
		vertices[1].pos,
		vertices[2].pos,
		vertices[3].pos
	};


	return vertexBuffer;

}
XMMATRIX CreateHUDMatrix(float width, float height,float offsetx=0,float offsety=0)
{
	XMMATRIX HUDMat=XMMatrixIdentity();
	XMMATRIX m0 = XMMatrixScaling(2.0f / width, -2.0f / height, 1);
	XMMATRIX m1 = XMMatrixTranslation(-1, 1, 0);
	HUDMat = XMMatrixMultiply(m0, m1);

	HUDMat._41 = -1.0f + offsetx*2.0f / WINDOW_WIDTH;
	HUDMat._42 = 1.0f + offsety*2.0f / WINDOW_HEIGHT;

	/*HUDMat._11 = 2.0f / (float)width;
	HUDMat._22 = 2.0f / (float)height;
	HUDMat._41 = -1;
	HUDMat._42 = 1;*/
	return HUDMat;
}

ID3D11Buffer* CreateHUDVertexBuffer(float top, float left, float width, float height)
{
	DeviceDx11& dev = DeviceDx11::Instance();
	
	std::vector<HUDVertex> vertices(4);
	{
		vertices[0] = { XMFLOAT3(left, top, 0.1f), XMFLOAT2(0.f, 0.f) };//����
		vertices[1] = { XMFLOAT3(left + width, top, 0.1f), XMFLOAT2(1.0f, 0.0f) };//�E��
		vertices[2] = { XMFLOAT3(left, top + height, 0.1f), XMFLOAT2(0.f, 1.f) };//����
		vertices[3] = { XMFLOAT3(left + width, top + height, 0.1f), XMFLOAT2(1.f, 1.f) };//�E��
	};
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &vertices[0];

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = sizeof(HUDVertex)*4;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = sizeof(HUDVertex);

	HRESULT result = S_OK;
	ID3D11Buffer* vertexBuffer = nullptr;
	result = dev.Device()->CreateBuffer(&desc, &data, &vertexBuffer);

	XMMATRIX hudMatrix = CreateHUDMatrix(WINDOW_WIDTH, WINDOW_HEIGHT);

	XMFLOAT3 pa[4] = {
		vertices[0].pos,
		vertices[1].pos,
		vertices[2].pos,
		vertices[3].pos
	};

	for (auto& p : pa)
	{
		XMVECTOR pos = XMLoadFloat3(&p);
		XMMATRIX mat = hudMatrix;
		XMVECTOR v = XMVector3Transform(pos, mat);
		XMFLOAT3 r;
		XMStoreFloat3(&r, v);
		int b = 0;
	}

	return vertexBuffer;

}

HRESULT CreateHUDShader(
	ID3D11VertexShader*& vs,
	ID3D11InputLayout*& layout,
	ID3D11PixelShader*& ps)
{
	HRESULT result;
	D3D11_INPUT_ELEMENT_DESC inputElementDescs[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	int hh = sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	result = ShaderGenerator::CreateVertexShader(
		"BaseShader.hlsl",
		"HUDVS",
		"vs_5_0",
		vs,
		inputElementDescs,
		sizeof(inputElementDescs)/sizeof(D3D11_INPUT_ELEMENT_DESC),
		layout);

	result = ShaderGenerator::CreatePixelShader(
		"BaseShader.hlsl",
		"HUDPS",
		"ps_5_0",
		ps);

	return result;
}
HRESULT CreateBillBoardShader(
	ID3D11VertexShader*& vs,
	ID3D11InputLayout*& layout,
	ID3D11PixelShader*& ps)
{
	HRESULT result;
	D3D11_INPUT_ELEMENT_DESC inputElementDescs[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	int hh = sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	result = ShaderGenerator::CreateVertexShader(
		"BaseShader.hlsl",
		"BillBoardVS",
		"vs_5_0",
		vs,
		inputElementDescs,
		sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		layout);

	result = ShaderGenerator::CreatePixelShader(
		"BaseShader.hlsl",
		"BillBoardPS",
		"ps_5_0",
		ps);

	return result;
}
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

	
	////�m�[�}���}�b�v�p�e�N�X�`��
	//ID3D11ShaderResourceView* normalTex;
	//result = D3DX11CreateShaderResourceViewFromFile(dev.Device(), "normal.png", nullptr, nullptr, &normalTex, &result);
	//dev.Context()->PSSetShaderResources(5, 1, &normalTex);

	////�����}�b�s���O�p�e�N�X�`��
	//ID3D11ShaderResourceView* heightMap;
	//result = D3DX11CreateShaderResourceViewFromFile(dev.Device(), "height.png", nullptr, nullptr, &heightMap, &result);
	//dev.Context()->PSSetShaderResources(6, 1, &heightMap);


	//ID3D11ShaderResourceView* displaysmentMap;
	//result = D3DX11CreateShaderResourceViewFromFile(dev.Device(), "disp.png", nullptr, nullptr, &displaysmentMap, &result);
	//dev.Context()->VSSetShaderResources(7, 1, &displaysmentMap);

	//float col[4] = { 0.5, 0.5, 0.5, 1.0 };
	//clearColor = col;
}

//���ʁA�V�����_���������q�ŏ��������Ă�̂͌�łǂ��ɂ�����
//GameMain����E�B���h�E�n���h�����󂯎���Ă���
//�ŏ���Direct3D�̏����������邽�߂�_result����ď������q��InitDirecct3D���Ă�
PlayingScene::PlayingScene(HWND hwnd) 
	:_hwnd(hwnd),_result(InitDirect3D(_hwnd)),dev(DeviceDx11::Instance()),
	_player(_camera), _plane(120, 120, Vector3(0, 1, 0), _camera), 
	_cylinder(4, 20, 20,_camera),
	_sphere(100,5,_camera),
	_tessPlane(300,300,Vector3(0,1,0),_camera),
	_decalBox(1,1,1,&_camera)
{
	//InitDirect3D(_hwnd);//�������q�Ŋ��ɌĂ�ł���
	Init();
	_renderer.Init();//�����_���[������
	_player.Init();

	_skySphere = new SkySphere(100, 200, &_camera);

	_soundManager.Init();//�T�E���h�}�l�[�W��������

	HRESULT result;

	//�f�J�[���t�@�N�g��
	_decalFac = new DecalFactory(&_camera);


	_effect.Emit();
	_effectMov = { 0, 0, 0 };
	_effect.SetCamera(_camera.CameraView(), _camera.CameraProjection());

	//�r���[�s��͉�]�ƕ��s�ړ������Ȃ��i�g��͂Ȃ�
	//��]�s��͓]�u����Ƌt�s��̐���������
	//�r���[�s��̉�]�ƕ��s�ړ��݂͌��Ɋ����Ȃ��̂ŁA�]�u�s��ɂ���Ƌt�s��ɂȂ�
	//Inverse�֐����g��Ȃ��Ă����̂ŒP���ɓ]�u�����������y��


	//HUD�p�V�F�[�_
	//HUD�p���_�i�s�N�Z���P�ʁi2D���W�n�Ŏw��j
	//HUD�p���_�Ԋ�Matrix
	//�|�C���g��2D�s�N�Z���w����ǂ�����̂�
	//��HUD�p�s���2D�p�̍s��ɂł��Ȃ���
	_hudMatrix = CreateHUDMatrix(WINDOW_WIDTH, WINDOW_HEIGHT);

	XMFLOAT3 pa[2] = {
		{ 0, 0, 0 },
		{ (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, 0 }
	};

	for (auto& p : pa)
	{
		XMVECTOR pos = XMLoadFloat3(&p);
		XMMATRIX mat = _hudMatrix;
		XMVECTOR v = XMVector3Transform(pos, mat);
		XMFLOAT3 r;
		XMStoreFloat3(&r, v);
		int b = 0;
	}
	_hudVS = nullptr;
	_hudInputLayout = nullptr;
	_hudPS = nullptr;
	CreateHUDShader(_hudVS, _hudInputLayout, _hudPS);


	_hudBuffer = CreateHUDVertexBuffer(0,0,320,240);

	unsigned int hudstride = sizeof(HUDVertex);
	unsigned int hudoffset = 0;

	//�}�[�J�[���_�o�b�t�@�쐬
	_makerBuffer = CreateHUDVertexBuffer(-8, -8, 16, 16);
	D3DX11CreateShaderResourceViewFromFile(dev.Device(), "texture/marker.dds", nullptr, nullptr, &_makerSRV, &result);

	//�r���{�[�h�e�X�g
	billBoardBuffer = nullptr;
	billBoardBuffer = CreateBillBoardVertexBuffer(10, 10);
	billBoardSRV = nullptr;
	D3DX11CreateShaderResourceViewFromFile(dev.Device(), "texture/uvCheck_transparent.png", nullptr, nullptr, &billBoardSRV, &result);
	billBoardVS=nullptr;
	billBoardPS=nullptr;
	billBoardInputLayout=nullptr;
	CreateBillBoardShader(billBoardVS, billBoardInputLayout, billBoardPS);

	
	//mvp�s��p�̃o�b�t�@��� HUD�p�ɍ���������Ȃ̂Ō�ŃN���X�����������
	D3D11_BUFFER_DESC matBuffDesc = {};
	matBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matBuffDesc.ByteWidth = sizeof(WorldAndCamera);
	matBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;//�o�b�t�@�̒��g��CPU�ŏ���������
	matBuffDesc.Usage = D3D11_USAGE_DYNAMIC;//CPU�ɂ�鏑�����݁AGPU�ɂ��ǂݍ��݂��s����Ƃ����Ӗ�
	//matBuffDesc.ByteWidth = sizeof(XMMATRIX);
	//matBuffDesc.StructureByteStride = sizeof(XMMATRIX);


	_worldAndCamera.world = _hudMatrix;
	_worldAndCamera.cameraView = _camera.CameraView();
	_worldAndCamera.cameraProj = _camera.CameraProjection();
	D3D11_SUBRESOURCE_DATA d;
	d.pSysMem = &_worldAndCamera;

	result = dev.Device()->CreateBuffer(&matBuffDesc, &d, &_matrixBuffer);

	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
	//�����ł��̃������̉�ɁA�}�g���b�N�X�̒l���R�s�[���Ă��
	memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
	//*(XMMATRIX*)mem.pData = matrix;//���搶�̏������@memcpy�Ő��l���Ԉ�
	dev.Context()->Unmap(_matrixBuffer, 0);

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);


	//�V�F�[�_�Ƀt���[���P�ʂœn���萔�o�b�t�@
	unsigned int globalTimer = 0;
	_shaderGlobals.eyePos = _camera.GetEyePos();//XMFLOAT4(eyePoint.x, eyePoint.y, eyePoint.z, 1);//XMFLOAT3(gazePoint.x - eyePoint.x, gazePoint.y - eyePoint.y, gazePoint.z - eyePoint.z);//Vector3(gazePoint.x - eyePoint.x, gazePoint.y - eyePoint.y, gazePoint.z - eyePoint.z).Normalize();
	_shaderGlobals.lightPos = _camera.GetLightPos();//w�v�f��1�Ȃ�|�C���g���C�g�A0�Ȃ�f�B���N�V���i�����C�g�H
	_shaderGlobals.timer = 0;
	_shaderGlobals.nearZ = NEAR_Z;
	_shaderGlobals.farZ = FAR_Z;

	_shaderGlobals.fog
		= XMFLOAT2(FAR_Z / (FAR_Z - NEAR_Z), -1.0f / (FAR_Z - NEAR_Z));
	_shaderGlobals.fogColor
		= XMFLOAT4(0.5f,0.9f,0.9f,1);
	


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
	//*(XMMATRIX*)mem.pData = matrix;//���搶�̏������@memcpy�Ő��l���ԈႦ��ƃ������������Ⴎ����ɂȂ�
	dev.Context()->Unmap(_globalBuffer, 0);

	dev.Context()->VSSetConstantBuffers(5, 1, &_globalBuffer);

	debugToggle = true;
}


PlayingScene::~PlayingScene()
{
	delete(_decalFac);
	delete(_skySphere);

	_soundManager.Terminate();
}


void
PlayingScene::Update()
{
	_soundManager.Update();

#pragma region ���́i��
	std::copy(keystate, keystate + sizeof(keystate), lastkeystate);
	GetKeyboardState(keystate);
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
	if (keystate[VK_UP] & 0x80)
	{
		if (!(lastkeystate[VK_UP] & 0x80))
		{
			_effectMov.y += 1;
			_effect.Move(_effectMov);
		}
	}

	if (keystate[VK_LBUTTON]&0x80)//�}�E�X��
	{
		//���C���v�Z����@far - near  XMVectorUnProj~~
		//d �J�����̃|�W�V�����ƕ��ʂ̖@���̓��ς��Ƃ�
		//t ���C�ƕ��ʂ̖@���̓��ς��Ƃ�
		//
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(_hwnd, &p);


		XMVECTOR ray = _camera.CalculateCursorVector(p.x, p.y);
		ray = XMVector3Normalize(ray);

		XMVECTOR camerapos = XMLoadFloat3(&_camera.GetPos());

		float d;
		XMStoreFloat(&d, XMVector3Dot(camerapos, _plane.Normal()));
		float t;
		XMStoreFloat(&t, XMVector3Dot(-ray, _plane.Normal()));

		ray = XMVectorScale(ray, d / t);

		XMFLOAT3 pos;
		XMStoreFloat3(&pos, camerapos + ray);
		//pos.y += 5;
		
		//_effect.Emit(pos);

		//�f�J�[���{�b�N�X�̈ʒu�Z�b�g
		_decalBox.SetPos(pos);
		int j = 0;
		bool f = keystate[VK_SPACE] & 0x80;
		if (f) {
			int j = 0;
		}
		if (keystate[VK_SPACE] & 0x80 == true)
		{
			_decalFac->CreateDecalBox(pos, XMFLOAT3(45.0f, 0.0f, 0.0f), XMFLOAT3(16.0f, 16.0f, 16.0f));
		}
		/*UINT num = 1;
		dev.Context()->RSGetViewports(&num, &vp);
		XMVECTOR retVec;
		retVec = XMVector3Unproject(nullptr, 0, 0,
			vp.Width, vp.Height, vp.MinDepth, vp.MaxDepth,
			_camera.CameraProjection(), _camera.CameraView(), XMMatrixIdentity());*/

	}

	float moveFront = 0.0f, moveRight = 0.0f;
	if (keystate['W'] & 0x80)
	{
		moveFront += 1.0f;//_camera.MoveTPS(0.5f, 0);
	}
	if (keystate['S'] & 0x80)
	{
		moveFront -= 1.0f;//_camera.MoveTPS(-0.5f, 0);
	}
	if (keystate['A'] & 0x80)
	{
		moveRight -= 1.0f;//_camera.MoveTPS(0, -0.5f);
	}
	if (keystate['D'] & 0x80)
	{
		moveRight += 1.0f;//_camera.MoveTPS(0, 0.5f);
	}
	_camera.MoveTPS(moveFront, moveRight);

	XMFLOAT3 camRot = {0.0f,0.0f,0.0f};//���ꂼ��x,y,x����̉�]
	if (keystate[VK_NUMPAD6]&0x80)
	{
		camRot.y += 1;
	}
	if (keystate[VK_NUMPAD4] & 0x80)
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
	_camera.Rotate(camRot);


	
	/*POINT p;
	GetCursorPos(&p);
	ScreenToClient(_hwnd, &p);*/

#pragma endregion

	//�V�F�[�_�ɓn���^�C�}�[�̍X�V
	_shaderGlobals.timer++;
	_shaderGlobals.eyePos = _camera.GetEyePos();//XMFLOAT4(eyePoint.x, eyePoint.y, eyePoint.z, 1);//XMFLOAT3(gazePoint.x - eyePoint.x, gazePoint.y - eyePoint.y, gazePoint.z - eyePoint.z);//Vector3(gazePoint.x - eyePoint.x, gazePoint.y - eyePoint.y, gazePoint.z - eyePoint.z).Normalize();
	_shaderGlobals.lightPos = _camera.GetLightPos();//w�v�f��1�Ȃ�|�C���g���C�g�A0�Ȃ�f�B���N�V���i�����C�g�H
	//shaderGlobals.dummy0++;
	dev.Context()->Map(_globalBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mappedGlobals);
	memcpy(_mappedGlobals.pData, (void*)(&_shaderGlobals), sizeof(ShaderGlobals));
	dev.Context()->Unmap(_globalBuffer, 0);
	dev.Context()->VSSetConstantBuffers(5, 1, &_globalBuffer);


	_player.Update();
	_plane.Update();
	_cylinder.Update();
	_tessPlane.Update();
	_sphere.Update();
	_decalBox.Update();
	_decalFac->Update();
	_skySphere->SetPos(_camera.GetPos());
	_skySphere->Update();

	_camera.Update();


	float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	//float white[4] = { 1.0, 1.0, 1.0, 1.0 };


	_renderer.ChangePTForPMD();
	_renderer.CullBack();
	_renderer.ChangeRT_LightColor();
	_sphere.DrawLightView_color();


#pragma region ���C�g�r���[�`��
	_renderer.ChangeRT_Light();

	//_renderer.ZWriteOff();
	_renderer.ChangePTForPrimitive();
	_plane.DrawLightView();
	_cylinder.DrawLightView();

	_renderer.ChangePTForPMD();
	_player.DrawLightView();
	_sphere.DrawLightView();
	//_renderer.ZWriteOn();

#pragma endregion

#pragma region �J�����f�v�X�`��
	_renderer.ChangeRT_CameraDepth();
	_player.DrawCameraDepth();
	_plane.DrawCameraDepth();
	_cylinder.DrawCameraDepth();
	_sphere.DrawCameraDepth();

#pragma endregion
	

#pragma region �J�����r���[�`��
	_renderer.ChangeRT_Camera();
	_skySphere->Draw();
	//���C�g�r���[����̃����_�����O���ʂ��e�N�X�`���Ƃ���GPU�ɓn��
	//SetRenderTarget�������SetShaderResources���Ȃ��Ɠn��Ȃ�
	ID3D11ShaderResourceView* resource = _renderer.LightDepthShaderResource();
	dev.Context()->PSSetShaderResources(10, 1, &resource);

	//���C�g����̃����_�����O���ʁi�J���[
	resource = _renderer.LightViewShaderResource();
	dev.Context()->PSSetShaderResources(11, 1, &resource);

	_renderer.ChangePTForPMD();
	_player.Draw();//�v���C���[�̃��C���`��

	_renderer.ChangePTForPrimitive();
	//_plane.Draw();//��
	_cylinder.Draw();//��
	_renderer.ChangePTForPMD();
	dev.Context()->DSSetConstantBuffers(5, 1, &_globalBuffer);
	_tessPlane.Draw();//�e�b�Z���[�V��������

	//�f�J�[���{�b�N�X�����_�V�F�[�_�Ő[�x�o�b�t�@�e�N�X�`���g���̂œn��
	resource = _renderer.CameraDepthShaderResource();
	dev.Context()->VSSetShaderResources(12, 1, &resource);
	_renderer.DepthDisable();
	_renderer.ZWriteOff();


	_renderer.CullNone();//���ʕ`�悳����
	_sphere.Draw();//�������X���C��
	_renderer.CullBack();

	//�J��������̐[�x�o�b�t�@�e�N�X�`��
	resource = _renderer.CameraDepthShaderResource();
	dev.Context()->PSSetShaderResources(12, 1, &resource);
	//_renderer.CullNone();
	_decalBox.DebugDraw();//�f�J�[���{�b�N�X
	_decalFac->Draw();
	_renderer.ZWriteOn();
	//_renderer.CullBack();
	

#pragma endregion

	{
		//�r���{�[�h�\��

		_renderer.ChangePTForPrimitive();

		dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);
		//world���������ihudMatrix�Ɂj
		_worldAndCamera.world = XMMatrixTranslation(-10, 15, 10);

		XMMATRIX w = _worldAndCamera.world;
		XMMATRIX view = _camera.CameraView();
		XMFLOAT3 trans = { view._41, view._42, view._43 };
		view._41 = view._42 = view._43 = 0;
		XMMATRIX invView = XMMatrixTranspose(view);
		//invView._41 = trans.x; invView._42 = trans.y; invView._43 = trans.z;
		XMMATRIX proj = _camera.CameraProjection();
		//���̂܂܃J������]�t�s����|����ƃ��[���h���s�ړ����e�����󂯂Ă��܂�����
		//�ꎞ�I�Ƀ��[���h�̕��s�ړ��𖳌��ɂ��āA�J�����t��]���|������ŕ��s�ړ����������ɖ߂�
		//Wr * [Wt * Vr^-1] * Vr * Vt * proj
		//�@�@�@�@�@�����������@���[���h���s�ړ��̌�ɉ�]�s�񂪊|������̂�
		//						�@�@���s�ړ���̍��W����ɉ�]����Ă��܂�
		XMFLOAT3 worldTrans = { w._41, w._42, w._43 };
		w._41 = w._42 = w._43 = 0.0f;
		XMMATRIX temp = XMMatrixMultiply(w, invView);
		temp._41 = worldTrans.x; temp._42 = worldTrans.y; temp._43 = worldTrans.z;
		_worldAndCamera.world = temp;
		_worldAndCamera.cameraView = _camera.CameraView();
		_worldAndCamera.cameraProj = _camera.CameraProjection();

		dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
		//�����ł��̃������̉�ɁA�}�g���b�N�X�̒l���R�s�[���Ă��
		memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
		//*(XMMATRIX*)mem.pData = matrix;//���搶�̏������@memcpy�Ő��l���Ԉ�
		dev.Context()->Unmap(_matrixBuffer, 0);

		dev.Context()->VSSetShader(billBoardVS, nullptr, 0);
		dev.Context()->PSSetShader(billBoardPS, nullptr, 0);
		dev.Context()->IASetInputLayout(billBoardInputLayout);
		//���C�g����̃����_�����O���ʁi�J���[
		resource = billBoardSRV;
		dev.Context()->PSSetShaderResources(0, 1, &resource);
		unsigned int hudstride = sizeof(HUDVertex);
		unsigned int hudoffset = 0;
		dev.Context()->IASetVertexBuffers(0, 1, &billBoardBuffer, &hudstride, &hudoffset);

		dev.Context()->Draw(4, 0);
	}

	
	_effect.SetCamera(_camera.CameraView(), _camera.CameraProjection());
	_effect.Update();

#pragma region HUD�`��
	if (debugToggle)
	{
		_renderer.ChangePTForPrimitive();

		dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);
		//world���������ihudMatrix�Ɂj
		_worldAndCamera.world = _hudMatrix;// XMMatrixIdentity();
		_worldAndCamera.cameraView = _camera.CameraView();
		_worldAndCamera.cameraProj = _camera.CameraProjection();

		dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
		//�����ł��̃������̉�ɁA�}�g���b�N�X�̒l���R�s�[���Ă��
		memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
		//*(XMMATRIX*)mem.pData = matrix;//���搶�̏������@memcpy�Ő��l���Ԉ�
		dev.Context()->Unmap(_matrixBuffer, 0);

		dev.Context()->VSSetShader(_hudVS, nullptr, 0);
		dev.Context()->PSSetShader(_hudPS, nullptr, 0);
		dev.Context()->IASetInputLayout(_hudInputLayout);
		//���C�g����̃����_�����O���ʁi�J���[
		resource = _renderer.LightDepthShaderResource();
		//resource = _renderer.CameraDepthShaderResource();
		dev.Context()->PSSetShaderResources(10, 1, &resource);
		unsigned int hudstride = sizeof(HUDVertex);
		unsigned int hudoffset = 0;
		dev.Context()->IASetVertexBuffers(0, 1, &_hudBuffer, &hudstride, &hudoffset);
		
		dev.Context()->Draw(4, 0);
	}
#pragma endregion

	{
		//ik�{�[���̏ꏊ����Ă���
		int ikboneIdx = _player.GetMesh()->GetBoneMap()["�E���h�j"];
		XMFLOAT3 ikpos = _player.GetMesh()->
			GetBoneVertices()[ikboneIdx].head.pos;
		ikpos = _player.IKPos();
		//���̍��W�ɑ΂����i�{�[���s��܂ށj�A���A���|���Ă��Ŋ���΃N���b�s���O��ԏ�̈ʒuposCS�����܂�
		//posCS�̓E�B���h�E�̉𑜓xSw,Sh����ɂ��ꂼ��x=-1~1��0~Sw,h=-1~1��0~Sh�ɂł���
		//�o�Ă���x,y�̒l���I�t�Z�b�g������s����}�[�J�[�̒��_�ɂ����Ă��
		//XMMATRIX bonemat = _player.GetMesh()->BoneMatrixies()[ikboneIdx];
		XMMATRIX world = _player.GetModelMatrix();
		//XMMATRIX wvp = XMMatrixMultiply(bonemat,world);
		XMMATRIX view = _camera.CameraView();
		XMMATRIX proj = _camera.CameraProjection();
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

		XMMATRIX h = _hudMatrix;
		XMMATRIX m = XMMatrixMultiply(screenOfsMatrix, h);



		_renderer.ChangePTForPrimitive();

		dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);
		//world���������ihudMatrix�Ɂj
		_worldAndCamera.world = m;// XMMatrixIdentity();
		_worldAndCamera.cameraView = _camera.CameraView();
		_worldAndCamera.cameraProj = _camera.CameraProjection();

		dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
		//�����ł��̃������̉�ɁA�}�g���b�N�X�̒l���R�s�[���Ă��
		memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
		//*(XMMATRIX*)mem.pData = matrix;//���搶�̏������@memcpy�Ő��l���Ԉ�
		dev.Context()->Unmap(_matrixBuffer, 0);

		dev.Context()->VSSetShader(_hudVS, nullptr, 0);
		dev.Context()->PSSetShader(_hudPS, nullptr, 0);
		dev.Context()->IASetInputLayout(_hudInputLayout);
		//���C�g����̃����_�����O���ʁi�J���[
		resource = _makerSRV;
		dev.Context()->PSSetShaderResources(10, 1, &resource);
		unsigned int hudstride = sizeof(HUDVertex);
		unsigned int hudoffset = 0;
		dev.Context()->IASetVertexBuffers(0, 1, &_makerBuffer, &hudstride, &hudoffset);

		dev.Context()->Draw(4, 0);

		int kkkk = 0;
	}

	dev.SwapChain()->Present(1, 0);
}
