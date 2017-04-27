cbuffer global:register(b0){
	matrix _world;
	matrix _view;//カメラビュー
	matrix _proj;//カメラプロジェクション
	matrix _invWorld;
	matrix _invView;
	matrix _invProj;

	matrix _invWVP;
	matrix _wvp;

};
cbuffer Global2:register(b5){
	float4 lightPos;
	float4 eyePos;
	int timer;

};

//行列の成分取り出しが列優先順になってる
//3行目4列目　なら._43 ._m32
//4行目3列目　なら._34 ._m23 で取り出す
struct Output{
	float4 pos:SV_POSITION;
	float4 postest:POSITION;
	float4 viewPos:POSITION2;
	float2 texcoord:TEXCOORD;
	float3 posWorld:POSITION1;
	float4x4 invWorld:MATRIX0;
	float4x4 invView:MATRIX4;
	float4x4 invWVP:MATRIX8;

	float4x4 proj:MATRIX12;
	float4x4 invProj:MATRIX16;

	float4 posi:TEXCOORD1;

	float4 color:COLOR;

};

matrix MatrixIdentity()
{
	matrix ret = {
		float4(1, 0, 0, 0),
		float4(0, 1, 0, 0),
		float4(0, 0, 1, 0),
		float4(0, 0, 0, 1)
	};
	return ret;
}

Texture2D _tex:register(t0);
Texture2D _sph:register(t1);
Texture2D _spa:register(t2);

Texture2D _normalTex:register(t5);
Texture2D _heightMap:register(t6);
Texture2D _dispMap:register(t7);
Texture2D _dispMask:register(t8);
Texture2D _decalTex:register(t9);//デカール
Texture2D _shadowTex:register(t10);//ライトからのレンダリング結果をテクスチャとして受け取る
Texture2D _lightViewTex:register(t11);
Texture2D _cameraDepthTex:register(t12);

SamplerState _samplerState:register(s0);
SamplerState _samplerStateDisp:register(s1);
SamplerState _samplerState_clamp:register(s2);


matrix DisableTranslation(matrix mat)
{
	matrix m = mat;
	m._m03 = m._m13 = m._m23 = 0;
	return m;
}

matrix InverseTranslation(matrix mat)
{
	matrix m = mat;
	m._m03 = -m._m03;
	m._m13 = -m._m13;
	m._m23 = -m._m23;
	return m;
}

Output DecalBoxVS(float4 pos:POSITION)
{
	Output o;
	matrix wvp = mul(_proj, mul(_view, _world));
	o.pos = mul(wvp,pos);

	o.postest = mul(_world, pos);
	o.postest = mul(_view, o.postest);
	o.postest = mul(_proj, o.postest);

	o.texcoord = (float2(1, 1) + (o.postest.xy / o.postest.w)*float2(1, -1))*0.5f;

	float sampleDepth = _cameraDepthTex.SampleLevel(_samplerState, o.texcoord, 0).r;

	float3 positionVS;//ViewSpace Position
	matrix wv = mul(_view, _world);

	positionVS = mul(wv, pos);
	o.viewPos = mul(wv, pos);
	//positionVS = mul(float4(positionVS, 1), _invView).xyz;

	

	o.pos = mul(_world, pos);
	o.pos = mul(_view, o.pos);
	o.viewPos = o.pos;
	o.pos = mul(_proj, (float4x1)o.pos);

	//o.pos = mul(_wvp, pos);

	//directXが右手系なのでpositionVS.zにマイナスは要らない？
	float3 viewRay = float3(positionVS.xy, 100.0f);//positionVS.xyz*(100.0f / positionVS.z);//(-positionVS.z / 100.0f);//

		float3 viewPosition = float3(viewRay.xy,viewRay.z*sampleDepth);
		//o.pos = mul( _proj,float4(viewPosition, 1));

		float3 wpos = mul(float4(viewPosition, 1), _invView).xyz;
		o.posWorld = wpos;
	o.posWorld = mul(_world, pos);

	wpos = mul(float4(positionVS, 1), _invView).xyz;
	float3 objpos = mul(float4(wpos, 1), _invWorld).xyz;
		o.posWorld = wpos;
	//float3 objpos = mul(float4(wpos, 1), _invWorld).xyz;

	//o.pos = mul( wvp,float4(objpos,1));

	/*
	float3 objPos = mul(float4(wpos, 1), _invWorld).xyz;
		o.pos = mul(wvp,float4(objPos, 1));*/

	//o.pos = mul(_proj, float4(viewPosition,1));
	//o.pos = wpos;
	o.invWorld = InverseTranslation(_world);
	o.invView = _invView;
	o.invWVP = _invWVP;
	o.proj = _proj;
	o.invProj = _invProj;

	matrix invTest = mul(_invWorld,_world);


	float4 vpos = mul(_invProj, o.postest);
		float4 posw = mul(_invView, vpos);
		float4 lpos = mul(_invWorld, posw);
		//lpos = mul(_invWVP, o.pos);
		if (_invWorld._14 == -5.0f)
		{
			o.color = float4(0, 1, 1, 1);
		}

	return o;
}

float4 DecalBoxPS(Output o):SV_Target
{
	float4 col;

	float2 screenpos = o.postest.xy / o.postest.w;

	float2 coord = float2(
	(1.0f + screenpos.x) / 2.0f + (0.5f / 640.0f),
	(1.0f - screenpos.y) / 2.0f + (0.5f / 480.0f));
	//								↑半ピクセルずらす
	//ピクセルは実際にはドット（点）であり、ピクセル（正方形）の中心にある
	//(1.0 + screenpos.x)/2.0f,(1.0f - screenpos.y)/2.0f
	//↑で出てくるのはピクセルの左上の座標であり、
	//解像度を基に半ピクセルずらすと正確なテクセルが求められる
	//テクセル　：テクスチャエレメント　テクスチャを構成する画素
	//半ピクセルずらさない場合実際に描画するとテクスチャが若干歪む


	float d = _cameraDepthTex.Sample(_samplerState, coord);

	
	float2 Deproject = float2(o.proj._m00, o.proj._m11);
		float dep = _cameraDepthTex.Sample(_samplerState,coord);
	float4 scenePosView = float4(o.postest.xy*d / (Deproject.xy*o.postest.w),
	dep, 1);

	//o.invView = DisableTranslation(o.invView);
	float4 scenePosWorld = mul(o.invView, scenePosView);
		float4 scenePosLocal = mul(o.invWorld,scenePosWorld);

	//matrix invWV = mul(o.invWorld, o.invView);
	//scenePosLocal = mul(invWV,scenePosView);
	//clip(4.0f - scenePosLocal.xyz);

		

	//float3 positionVS = mul(o.invProj, o.postest);//o.viewPos;
	//	float3 viewRay = float3(positionVS.xy*(100.0f / positionVS.z),100.0f);
	//	//viewRay = normalize(positionVS.xyz)*100.0f;

	//	float3 viewPosition = viewRay*d;
	//	float3 positionW = mul(o.invView,float4(viewPosition, 1)).xyz;
	//	float4 positionL = mul(o.invWorld,float4(positionW, 1));

	float4 positionVS = mul(o.invProj, o.postest);//o.viewPos;
	//positionVS = float4(positionVS.xyz / positionVS.w, 1.0f);
		
		float3 viewRay = float3(positionVS.xy*(100.0f / positionVS.z),100.0f);
		//viewRay = normalize(positionVS.xyz)*100.0f;

		float3 viewPosition = viewRay*d;
		float3 positionW = mul(o.invView,float4(viewPosition, 1)).xyz;
		float4 positionL = mul(o.invWorld,float4(positionW, 1));

		//clip(8.0f - abs(positionL.xyz));




	float depth = _cameraDepthTex.Sample(_samplerState, coord);
	//x=0~1 y=0~1のテクスチャ座標（？）から
	//x=-1~1 y=-1~1のクリッピング空間の座標にする
	//zの値はサンプリングした深度値を使う
	//深度値はカメラ座標からFarPlaneまでの距離を1.0として正規化したもの
	//正規化された座標が出てくる
	float4 screenPos = float4(coord*2.0f - 1.0f, depth, 1.0f);

		//WVP行列の逆行列をかけていってオブジェクト空間の座標にする
		//float4 localPos = mul(o.invWVP, screenPos);
		float4 localPos = mul(o.invProj, screenPos);
	//↑射影行列の逆行列かけるとwの値は
	//クリッピング空間で正規化されたｚ値が0.0なら1.0、ｚが1.0なら0.009999(0.01)になる
	//視錘台の拡大率（頂点にWVPかけた時のｗ）の逆数？
	//このｗで
	//「クリッピング空間で正規化された座標に対し射影行列の逆行列を掛けて出てきたｘｙｚ座標」
	//を割ると拡大率を掛けた事になる
	//			＝ビュー空間での座標が求められる
	localPos = float4(localPos.xyz / localPos.w, 1);
	localPos = mul(o.invView, localPos);
	localPos = mul(o.invWorld, localPos);
	//localPos = float4(localPos.xyz / localPos.w, localPos.w);

	//clip(8.0f-abs(localPos.xz));



	float4 posv = mul(o.invProj, screenPos);
		posv = float4(posv.xyz / posv.w, 1);
	float4	posw = mul(o.invView, posv);
	//posw = float4((posw.xyz / posw.w), posw.w);
	float4 posl = mul(o.invWorld, posw);
		//clip(4.0f - abs(posl.xz));


		//col = float4(0, 0, 0, 1);

	float2 uv = localPos.xz / float2(16.0f, -16.0f) + 0.5f;
	col = _decalTex.Sample(_samplerState, uv);//_decalTex.Sample(_samplerState, uv);
	if (localPos.x > 0.0f)
	{
		col.g = 1.0f;
	}

	//return float4(1, 0, 0, 0.7f);
	if (uv.x<1.0f&&uv.x>0.0f
		&&uv.y<1.0f&&uv.y>0.0f)
	{
		col.b = 1.0f;
	}
		return col;

	return float4(0, 0, 0, 1);
	//return (1, 1, 1, 1);
}

Output DecalPlaneVS(float4 pos:POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD)
{
	Output o;
	matrix wvp = mul(_proj, mul(_view, _world));
	o.pos = mul( wvp,pos);
	o.postest = o.pos;

	o.invWorld = _invWorld;
	o.invView = _invView;
	o.invProj = _invProj;

	o.texcoord = uv;

	return o;
}

float4 DecalPlanePS(Output o):SV_Target
{
	float2 screenpos = o.postest.xy / o.postest.w;

	float2 coord = float2(
		(1.0f + screenpos.x) / 2.0f + (0.5f / 640.0f),
		(1.0f - screenpos.y) / 2.0f + (0.5f / 480.0f));

	float depth = _cameraDepthTex.Sample(_samplerState, coord).r;

	float4 clipPos = float4(coord*2.0f - 1.0f, depth, 1.0f);

	float4 localPos = mul(o.invProj, clipPos);

	localPos = float4(localPos.xyz / localPos.w, 1);
	localPos = mul(o.invView, localPos);
	localPos = mul(o.invWorld, localPos);

	float2 uv = localPos.xz / float2(4.0f, 4.0f) + 0.5f;

	float4 col = _decalTex.Sample(_samplerState, uv);//_decalTex.Sample(_samplerState, o.texcoord);
	//col = lerp(col,float4(0.1f,0.1f,0.1f,1.0f), depth);
	//col.r = pow(col.r, 10);



	return col;
}