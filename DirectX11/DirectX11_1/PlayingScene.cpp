#include "PlayingScene.h"
#include"ShaderGenerator.h"



#pragma pack(1)
struct HUDVertex{
	XMFLOAT3 pos;
	XMFLOAT2 uv;
	/*HUDVertex(XMFLOAT3 p, XMFLOAT2 texcoord) :pos(p), uv(texcoord){}*/
};
#pragma pack()

//ビルボードの頂点バッファ
//@param ビルボード中心X座標(3D座標系
//@param　ビルボード中心Y座標
//@param ビルボード幅（3D座標系
//@param ビルボード高さ(3D座標系
//@param note 幅と高さは中心からの広がり 左上はx-width/2,y+height/2
ID3D11Buffer* CreateBillBoardVertexBuffer(float width, float height)
{
	DeviceDx11& dev = DeviceDx11::Instance();

	std::vector<HUDVertex> vertices(4);
	{
		vertices[0] = { XMFLOAT3(-width/2, height/2, 0), XMFLOAT2(0.f, 0.f) };//左上
		vertices[1] = { XMFLOAT3(width/2 , height/2 , 0), XMFLOAT2(1.0f, 0.0f) };//右上
		vertices[2] = { XMFLOAT3(-width/2, -height/2, 0), XMFLOAT2(0.f, 1.f) };//左下
		vertices[3] = { XMFLOAT3(width/2, -height/2, 0), XMFLOAT2(1.f, 1.f) };//右下
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
		vertices[0] = { XMFLOAT3(left, top, 0.1f), XMFLOAT2(0.f, 0.f) };//左上
		vertices[1] = { XMFLOAT3(left + width, top, 0.1f), XMFLOAT2(1.0f, 0.0f) };//右上
		vertices[2] = { XMFLOAT3(left, top + height, 0.1f), XMFLOAT2(0.f, 1.f) };//左下
		vertices[3] = { XMFLOAT3(left + width, top + height, 0.1f), XMFLOAT2(1.f, 1.f) };//右下
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
//DirectX11初期化関数

HRESULT
PlayingScene::InitDirect3D(HWND hwnd)
{
	DeviceDx11& dev = DeviceDx11::Instance();
	HRESULT result = dev.Init(hwnd);

	//バックバッファのレンダーターゲットビューを作成
	ID3D11Texture2D *pBack;
	//バックバッファのサーフェイスをしぇーだリソース（テクスチャ）として抜き出す
	dev.SwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBack);
	//そのテクスチャをレンダーターゲットとするようなレンダーターゲットビューを作成
	dev.Device()->CreateRenderTargetView(pBack, nullptr, &g_pRTV);
	pBack->Release();

	//デプスステンシルビュー（DSV）を作成
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

	//レンダーターゲットビューとデプスステンシルビューをセット
	dev.Context()->OMSetRenderTargets(1, &g_pRTV, g_pDSV);


	//アルファ有効の為の設定
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

	
	////ノーマルマップ用テクスチャ
	//ID3D11ShaderResourceView* normalTex;
	//result = D3DX11CreateShaderResourceViewFromFile(dev.Device(), "normal.png", nullptr, nullptr, &normalTex, &result);
	//dev.Context()->PSSetShaderResources(5, 1, &normalTex);

	////視差マッピング用テクスチャ
	//ID3D11ShaderResourceView* heightMap;
	//result = D3DX11CreateShaderResourceViewFromFile(dev.Device(), "height.png", nullptr, nullptr, &heightMap, &result);
	//dev.Context()->PSSetShaderResources(6, 1, &heightMap);


	//ID3D11ShaderResourceView* displaysmentMap;
	//result = D3DX11CreateShaderResourceViewFromFile(dev.Device(), "disp.png", nullptr, nullptr, &displaysmentMap, &result);
	//dev.Context()->VSSetShaderResources(7, 1, &displaysmentMap);

	//float col[4] = { 0.5, 0.5, 0.5, 1.0 };
	//clearColor = col;
}

//平面、シリンダも初期化子で初期化してるのは後でどうにかする
//GameMainからウィンドウハンドルを受け取っている
//最初にDirect3Dの初期化をするために_result作って初期化子でInitDirecct3Dを呼ぶ
PlayingScene::PlayingScene(HWND hwnd) 
	:_hwnd(hwnd),_result(InitDirect3D(_hwnd)),dev(DeviceDx11::Instance()),
	_player(_camera), _plane(120, 120, Vector3(0, 1, 0), _camera), 
	_cylinder(4, 20, 20,_camera),
	_sphere(100,5,_camera),
	_tessPlane(300,300,Vector3(0,1,0),_camera),
	_decalBox(1,1,1,&_camera)
{
	//InitDirect3D(_hwnd);//初期化子で既に呼んでいる
	Init();
	_renderer.Init();//レンダラー初期化
	_player.Init();

	_skySphere = new SkySphere(100, 200, &_camera);

	_soundManager.Init();//サウンドマネージャ初期化

	HRESULT result;

	//デカールファクトリ
	_decalFac = new DecalFactory(&_camera);


	_effect.Emit();
	_effectMov = { 0, 0, 0 };
	_effect.SetCamera(_camera.CameraView(), _camera.CameraProjection());

	//ビュー行列は回転と平行移動しかない（拡大はない
	//回転行列は転置すると逆行列の性質を示す
	//ビュー行列の回転と平行移動は互いに干渉しないので、転置行列にすると逆行列になる
	//Inverse関数を使わなくていいので単純に転置させた方が軽い


	//HUD用シェーダ
	//HUD用頂点（ピクセル単位（2D座標系で指定）
	//HUD用頂点返還Matrix
	//ポイントは2Dピクセル指定をどうするのか
	//→HUD用行列で2D用の行列にできないか
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

	//マーカー頂点バッファ作成
	_makerBuffer = CreateHUDVertexBuffer(-8, -8, 16, 16);
	D3DX11CreateShaderResourceViewFromFile(dev.Device(), "texture/marker.dds", nullptr, nullptr, &_makerSRV, &result);

	//ビルボードテスト
	billBoardBuffer = nullptr;
	billBoardBuffer = CreateBillBoardVertexBuffer(10, 10);
	billBoardSRV = nullptr;
	D3DX11CreateShaderResourceViewFromFile(dev.Device(), "texture/uvCheck_transparent.png", nullptr, nullptr, &billBoardSRV, &result);
	billBoardVS=nullptr;
	billBoardPS=nullptr;
	billBoardInputLayout=nullptr;
	CreateBillBoardShader(billBoardVS, billBoardInputLayout, billBoardPS);

	
	//mvp行列用のバッファ作る HUD用に作っただけなので後でクラス化したら消す
	D3D11_BUFFER_DESC matBuffDesc = {};
	matBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matBuffDesc.ByteWidth = sizeof(WorldAndCamera);
	matBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;//バッファの中身はCPUで書き換える
	matBuffDesc.Usage = D3D11_USAGE_DYNAMIC;//CPUによる書き込み、GPUによる読み込みが行われるという意味
	//matBuffDesc.ByteWidth = sizeof(XMMATRIX);
	//matBuffDesc.StructureByteStride = sizeof(XMMATRIX);


	_worldAndCamera.world = _hudMatrix;
	_worldAndCamera.cameraView = _camera.CameraView();
	_worldAndCamera.cameraProj = _camera.CameraProjection();
	D3D11_SUBRESOURCE_DATA d;
	d.pSysMem = &_worldAndCamera;

	result = dev.Device()->CreateBuffer(&matBuffDesc, &d, &_matrixBuffer);

	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
	//*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違
	dev.Context()->Unmap(_matrixBuffer, 0);

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);


	//シェーダにフレーム単位で渡す定数バッファ
	unsigned int globalTimer = 0;
	_shaderGlobals.eyePos = _camera.GetEyePos();//XMFLOAT4(eyePoint.x, eyePoint.y, eyePoint.z, 1);//XMFLOAT3(gazePoint.x - eyePoint.x, gazePoint.y - eyePoint.y, gazePoint.z - eyePoint.z);//Vector3(gazePoint.x - eyePoint.x, gazePoint.y - eyePoint.y, gazePoint.z - eyePoint.z).Normalize();
	_shaderGlobals.lightPos = _camera.GetLightPos();//w要素は1ならポイントライト、0ならディレクショナルライト？
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
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mappedGlobals.pData, (void*)(&_shaderGlobals), sizeof(ShaderGlobals));
	//*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違えるとメモリがぐちゃぐちゃになる
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

#pragma region 入力（仮
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

	if (keystate[VK_LBUTTON]&0x80)//マウス左
	{
		//レイを計算する　far - near  XMVectorUnProj~~
		//d カメラのポジションと平面の法線の内積をとる
		//t レイと平面の法線の内積をとる
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

		//デカールボックスの位置セット
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

	XMFLOAT3 camRot = {0.0f,0.0f,0.0f};//それぞれx,y,x軸基準の回転
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

	//シェーダに渡すタイマーの更新
	_shaderGlobals.timer++;
	_shaderGlobals.eyePos = _camera.GetEyePos();//XMFLOAT4(eyePoint.x, eyePoint.y, eyePoint.z, 1);//XMFLOAT3(gazePoint.x - eyePoint.x, gazePoint.y - eyePoint.y, gazePoint.z - eyePoint.z);//Vector3(gazePoint.x - eyePoint.x, gazePoint.y - eyePoint.y, gazePoint.z - eyePoint.z).Normalize();
	_shaderGlobals.lightPos = _camera.GetLightPos();//w要素は1ならポイントライト、0ならディレクショナルライト？
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


#pragma region ライトビュー描画
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

#pragma region カメラデプス描画
	_renderer.ChangeRT_CameraDepth();
	_player.DrawCameraDepth();
	_plane.DrawCameraDepth();
	_cylinder.DrawCameraDepth();
	_sphere.DrawCameraDepth();

#pragma endregion
	

#pragma region カメラビュー描画
	_renderer.ChangeRT_Camera();
	_skySphere->Draw();
	//ライトビューからのレンダリング結果をテクスチャとしてGPUに渡す
	//SetRenderTargetした後でSetShaderResourcesしないと渡らない
	ID3D11ShaderResourceView* resource = _renderer.LightDepthShaderResource();
	dev.Context()->PSSetShaderResources(10, 1, &resource);

	//ライトからのレンダリング結果（カラー
	resource = _renderer.LightViewShaderResource();
	dev.Context()->PSSetShaderResources(11, 1, &resource);

	_renderer.ChangePTForPMD();
	_player.Draw();//プレイヤーのメイン描画

	_renderer.ChangePTForPrimitive();
	//_plane.Draw();//床
	_cylinder.Draw();//柱
	_renderer.ChangePTForPMD();
	dev.Context()->DSSetConstantBuffers(5, 1, &_globalBuffer);
	_tessPlane.Draw();//テッセレーション平面

	//デカールボックスが頂点シェーダで深度バッファテクスチャ使うので渡す
	resource = _renderer.CameraDepthShaderResource();
	dev.Context()->VSSetShaderResources(12, 1, &resource);
	_renderer.DepthDisable();
	_renderer.ZWriteOff();


	_renderer.CullNone();//両面描画させる
	_sphere.Draw();//半透明スライム
	_renderer.CullBack();

	//カメラからの深度バッファテクスチャ
	resource = _renderer.CameraDepthShaderResource();
	dev.Context()->PSSetShaderResources(12, 1, &resource);
	//_renderer.CullNone();
	_decalBox.DebugDraw();//デカールボックス
	_decalFac->Draw();
	_renderer.ZWriteOn();
	//_renderer.CullBack();
	

#pragma endregion

	{
		//ビルボード表示

		_renderer.ChangePTForPrimitive();

		dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);
		//world書き換え（hudMatrixに）
		_worldAndCamera.world = XMMatrixTranslation(-10, 15, 10);

		XMMATRIX w = _worldAndCamera.world;
		XMMATRIX view = _camera.CameraView();
		XMFLOAT3 trans = { view._41, view._42, view._43 };
		view._41 = view._42 = view._43 = 0;
		XMMATRIX invView = XMMatrixTranspose(view);
		//invView._41 = trans.x; invView._42 = trans.y; invView._43 = trans.z;
		XMMATRIX proj = _camera.CameraProjection();
		//そのままカメラ回転逆行列を掛けるとワールド平行移動が影響を受けてしまうため
		//一時的にワールドの平行移動を無効にして、カメラ逆回転を掛けた後で平行移動成分を元に戻す
		//Wr * [Wt * Vr^-1] * Vr * Vt * proj
		//　　　　　↑ここが問題　ワールド平行移動の後に回転行列が掛けられるので
		//						　　平行移動後の座標を基準に回転されてしまう
		XMFLOAT3 worldTrans = { w._41, w._42, w._43 };
		w._41 = w._42 = w._43 = 0.0f;
		XMMATRIX temp = XMMatrixMultiply(w, invView);
		temp._41 = worldTrans.x; temp._42 = worldTrans.y; temp._43 = worldTrans.z;
		_worldAndCamera.world = temp;
		_worldAndCamera.cameraView = _camera.CameraView();
		_worldAndCamera.cameraProj = _camera.CameraProjection();

		dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
		//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
		memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
		//*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違
		dev.Context()->Unmap(_matrixBuffer, 0);

		dev.Context()->VSSetShader(billBoardVS, nullptr, 0);
		dev.Context()->PSSetShader(billBoardPS, nullptr, 0);
		dev.Context()->IASetInputLayout(billBoardInputLayout);
		//ライトからのレンダリング結果（カラー
		resource = billBoardSRV;
		dev.Context()->PSSetShaderResources(0, 1, &resource);
		unsigned int hudstride = sizeof(HUDVertex);
		unsigned int hudoffset = 0;
		dev.Context()->IASetVertexBuffers(0, 1, &billBoardBuffer, &hudstride, &hudoffset);

		dev.Context()->Draw(4, 0);
	}

	
	_effect.SetCamera(_camera.CameraView(), _camera.CameraProjection());
	_effect.Update();

#pragma region HUD描画
	if (debugToggle)
	{
		_renderer.ChangePTForPrimitive();

		dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);
		//world書き換え（hudMatrixに）
		_worldAndCamera.world = _hudMatrix;// XMMatrixIdentity();
		_worldAndCamera.cameraView = _camera.CameraView();
		_worldAndCamera.cameraProj = _camera.CameraProjection();

		dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
		//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
		memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
		//*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違
		dev.Context()->Unmap(_matrixBuffer, 0);

		dev.Context()->VSSetShader(_hudVS, nullptr, 0);
		dev.Context()->PSSetShader(_hudPS, nullptr, 0);
		dev.Context()->IASetInputLayout(_hudInputLayout);
		//ライトからのレンダリング結果（カラー
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
		//ikボーンの場所取ってくる
		int ikboneIdx = _player.GetMesh()->GetBoneMap()["右足ＩＫ"];
		XMFLOAT3 ikpos = _player.GetMesh()->
			GetBoneVertices()[ikboneIdx].head.pos;
		ikpos = _player.IKPos();
		//↑の座標に対しｗ（ボーン行列含む）、ｖ、ｐ掛けてｗで割ればクリッピング空間上の位置posCSが求まる
		//posCSはウィンドウの解像度Sw,Shを基にそれぞれx=-1~1→0~Sw,h=-1~1→0~Shにできる
		//出てきたx,yの値分オフセットさせる行列をマーカーの頂点にかけてやる
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

		//後でCreateHudMatrixの引数にoffsetX,offsestY追加して作り直す
		XMMATRIX screenOfsMatrix = XMMatrixTranslation(ikScreenPos.x, ikScreenPos.y, 0);

		XMMATRIX h = _hudMatrix;
		XMMATRIX m = XMMatrixMultiply(screenOfsMatrix, h);



		_renderer.ChangePTForPrimitive();

		dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);
		//world書き換え（hudMatrixに）
		_worldAndCamera.world = m;// XMMatrixIdentity();
		_worldAndCamera.cameraView = _camera.CameraView();
		_worldAndCamera.cameraProj = _camera.CameraProjection();

		dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
		//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
		memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
		//*(XMMATRIX*)mem.pData = matrix;//川野先生の書き方　memcpyで数値を間違
		dev.Context()->Unmap(_matrixBuffer, 0);

		dev.Context()->VSSetShader(_hudVS, nullptr, 0);
		dev.Context()->PSSetShader(_hudPS, nullptr, 0);
		dev.Context()->IASetInputLayout(_hudInputLayout);
		//ライトからのレンダリング結果（カラー
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
