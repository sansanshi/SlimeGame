//コンスタントバッファはメモリの塊として送られてくるので
//float3で送られてきた物をfloat4で受け取ったりするとfloat(4バイト)分ズレる
//頂点シェーダの引数はfloat3で送られてきたデータをfloat4で受け取っても
//勝手に第4要素に1を入れてくれるっぽい（？）

cbuffer global:register(b0){
	matrix _world;
	matrix _cameraView;
	matrix _cameraProj;
	matrix _lightView;
	matrix _lightProj;

};

cbuffer material:register(b1){

	float3 _diffuse:COLOR;
	float _alpha;
	float3 _specularColor;
	float _specularity;
	float3 _ambient;
};


cbuffer boneMatrix:register(b3)
{
	matrix _boneMatrix[512];
};

cbuffer Global2:register(b5){
	float4 lightPos;
	float4 eyePos;
	int timer;

};


struct Output{
	float4 pos:SV_POSITION;
	float4 normal:NORMAL;
	float2 uv:TEXCOORD0;
	float4 diffuse:COLOR0;
	float3 specular:COLOR1;//セマンティクスを設定しないとエラー吐く（？）
	float3 ambient:COLOR2;

	float4 lightVec:TEXCOORD1;
	float4 eyeVec:TEXCOORD2;

	float4x4 tangentMatrix:TANGENTMATRIX;
	float4x4 noTransMatrix:NOTRANS;
	float4 tangent:TEXCOORD3;
	float4 binormal:TEXCOORD4;

	float4 postest:POSITION1;
	float4 shadowpos:POSITION2;
	float2 texcoord:TEXCOORD5;//シャドウマップ（テスト用

	int timer : TIMER;

};

struct depthVS_Out{
	float4 pos:SV_POSITION;
};

Texture2D _tex:register(t0);
Texture2D _sph:register(t1);
Texture2D _spa:register(t2);

Texture2D _normalTex:register(t5);
Texture2D _heightMap:register(t6);
Texture2D _dispMap:register(t7);
Texture2D _dispMask:register(t8);
Texture2D _decalMap:register(t9);//デカール
Texture2D _shadowTex:register(t10);//ライトからのレンダリング結果をテクスチャとして受け取る
Texture2D _lightViewTex:register(t11);
SamplerState _samplerState:register(s0);
SamplerState _samplerStateDisp:register(s1);
SamplerState _samplerState_clamp:register(s2);


matrix TangentMatrix(float4 tangent, float4 binormal, float4 normal)
{
	matrix mat = {
		float4(normalize(tangent)),
		float4(normalize(binormal)),
		float4(normalize(normal)),
		float4(0, 0, 0, 1) };
	matrix test = {
		float4(1, 0, 0, 0),
		float4(0, 1, 0, 0),
		float4(0, 0, 1, 0),
		float4(0, 0, 0, 1)
	};
	return mat;

}

matrix InvTangentMatrix(float4 tangent, float4 binormal, float4 normal)
{
	matrix r = TangentMatrix(tangent, binormal, normal);
	return transpose(r);
}


matrix DisableTranslation(matrix mat)
{
	matrix m = mat;
	m._m03 = m._m13 = m._m23 = 0;
	return m;
}

float GetRandomNumber(float2 texCoord, int Seed)
{
	return frac(sin(dot(texCoord.xy, float2(12.9898, 78.233)) + Seed) * 43758.5453);
}

//こっち（シェーダ側）で定義済みのセマンティクスに対応するInputLayoutがないと
//CreateInputLayoutの時点でエラー？
//main側からfloat3をfloat4で受け取るとｗの要素には1.0が入っている
//「w」の要素に気を付けて計算する
Output BaseVS(float4 pos : POSITION, float2 uv : TEXCOORD,
	float4 normal : NORMAL, uint boneid : BONE_ID, uint boneWeight : BONE_WEIGHT,
	bool edgeFlag : EDGE, float4 binormal : BINORMAL, float4 tangent : TANGENT)
{
	Output o;


	matrix worldtemp = _world;
	float boneWgt1 = (float)boneWeight / 100.0f;
	float boneWgt2 = (100.0f - (float)boneWeight) / 100.0f;

	worldtemp = mul(worldtemp, _boneMatrix[boneid & 0x0000ffff] * boneWgt1) +
		mul(worldtemp, _boneMatrix[(boneid & 0xffff0000) / 0x10000] * boneWgt2);


	matrix worldtemp_noTr = DisableTranslation(worldtemp);
	o.noTransMatrix = worldtemp_noTr;

	//ボーン行列には平行移動成分が含まれてるので法線に適用するときは平行移動成分を消す
	o.normal = normalize(mul(worldtemp_noTr,normal));
	//o.normal.w = 1.0f;
	o.tangent = normalize(mul(worldtemp_noTr, tangent));
	o.binormal = normalize(mul(worldtemp_noTr, binormal));

	//o.tangentMatrix = TangentMatrix(normalize(o.tangent), normalize(o.binormal), normalize(o.normal));
	o.tangentMatrix = TangentMatrix(o.tangent, o.binormal, o.normal);

	float2 uvOffset = float2(0, (float)timer*0.01f);
		float displaysment = _dispMap.SampleLevel(_samplerStateDisp, uv + uvOffset, 0).r;//_dispMap.Sample(_samplerState, uv);
	//黒い部分は0、白い部分は1が入ってくる
	displaysment = displaysment * 2 - 1.0f;//-1.0～1.0の範囲に


	float f = (sin((float)timer*0.05f) + 1.0f) / 2;//0.0～1.0の範囲に
	f *= 0.2f;//0.0～0.3の範囲に
	displaysment *= f;

	

	float4 normalTemp = float4(normal.xyz * displaysment, 1.0);
		//normalTemp.w = 1.0f;
		float4 posTemp = pos;//posTemp = pos + normalTemp;←こうするとx,y,z,wの[w]の要素まで加算されてしまう
		//posTemp.xyz += normalTemp.xyz;//posTemp=float4(pos.xyz+normalTemp.xyz, 1.0)でも可

		matrix _camera = mul(_cameraProj, _cameraView);
	matrix tmp = mul(_camera, worldtemp);

	
	o.pos = mul(tmp, posTemp);//lightVecの後からズレてるっぽい？
	o.uv = uv;
	o.diffuse.rgb = _diffuse;
	o.diffuse.a = _alpha;
	o.specular = _specularColor*_specularity;
	o.ambient = _ambient;

	float4 posWorld = mul( worldtemp,pos);
		o.lightVec = float4(normalize(posWorld.xyz-lightPos.xyz ), 0);//float4(normalize(lightVec), 1.0);

	//eyeVec=頂点座標-視点
	o.eyeVec = float4(normalize(eyePos.xyz-posWorld.xyz),1);

	o.postest = mul(tmp, posTemp);
	matrix _lightVP = mul(_lightProj, _lightView);
	matrix lightview = mul(_lightVP, worldtemp);//_lightView
	o.shadowpos = mul(lightview, posTemp);

	o.timer = timer;

	return o;
}

float4 BasePS(Output o) :SV_Target
{

	/*float h = _heightMap.Sample(_samplerState, o.uv);

	float2 heightUV = o.uv + 0.03*h*o.eyeVec.xy;*/

	//法線テクスチャから色を取得
	float3 normalColor = _normalTex.Sample(_samplerState, o.uv).rgb;
	//ベクトルへ変換　↑で取得した時点では範囲が0~1なので-1~1になるように
	float3 normalVec = 2 * normalColor - 1.0f;
	normalVec = normalize(normalVec);
	//↓は頂点シェーダの方で処理させる（tangentMatrixを作ってこっちに渡す
	/*float3 ul = float3(normalVec.x*o.tangent.x, normalVec.x*o.tangent.y, normalVec.x*o.tangent.z);
	float3 vl = float3(normalVec.y*o.binormal.x, normalVec.y*o.binormal.y, normalVec.y*o.binormal.z);
	float3 zl = float3(normalVec.z*o.normal.x, normalVec.z*o.normal.y, normalVec.z*o.normal.z);
	float3 n_local = ul + vl + zl;*/
	//normalVec = n_local;
	float3 n_local = mul(normalVec, o.tangentMatrix);//これにモデルの回転とか適用されてないかも？
		//n_local = mul(o.noTransMatrix, n_local);
	//↑頂点シェーダの方で回転後のnormal、binormal、tangent使ってtangentMatrix作ったから要らない？
	float3 ref = reflect(-o.eyeVec, n_local);

	//saturate→0～1にクランプする関数
	//　※　光線ベクトルの「逆」ベクトルとの内積を取る
	float bright = saturate(dot(-o.lightVec, o.normal/*n_local*/));
	//return float4(bright, bright, bright, 1);
	//float4 col = v.diffuse*bright;//float4(v.diffuse*bright, v.diffuse*bright, v.diffuse*bright, 0);//_tex.Sample(_samplerState, v.uv);

	//v.diffuseにはRGBAが入っていて、brightには0~1の値が入っている
	//2つを乗算するとAlpha値にもbrightの0~1がかかってしまうので、本来暗くなる場所が透明になってしまう
	//float4 col = bright*(v.diffuse + ambient)*_tex.Sample(_samplerState, v.uv);//*sphcol;//brightは0~1の値を取っている
	//float3 sphcol = _sph.Sample(sample, v.normal.xy*float2(0.5f,-0.5f)+ / 2 + 0.5f);//sph適用
	//spa→nulltextureb sph→nulltexture
	//return float4(o.normal.x, o.normal.y, o.normal.z, 1.0f);

	float2 shadowUV = (float2(1, 1) + (o.shadowpos.xy / o.shadowpos.w)*float2(1, -1))*0.5f;
	float lightviewDepth = _shadowTex.Sample(_samplerState_clamp, shadowUV).r;

	float ld = o.shadowpos.z / o.shadowpos.w;
	float shadowWeight = 1.0f;
	if (ld > lightviewDepth+0.01f){
		shadowWeight = 0.1f;
	}

	//SSSのテスト　後で球体モデルにして試す
	float ed = o.postest.z / o.postest.w;
	float thickness = abs(ld - lightviewDepth)*100.0f;
	float4 transparent = float4(_tex.Sample(_samplerState, o.uv).rgb*(1.0f - saturate(thickness / 0.2f)), 1);
		transparent = _tex.Sample(_samplerState, o.uv)*(1.0f - saturate(thickness / 3.5f));
	float sss = 1.0f - saturate(thickness / 3.4f);
	//return transparent;
	//SSS

	//ジラジラ表現テスト
	float rand = GetRandomNumber(o.uv,(int)(o.timer));
	float _dot = saturate(dot(o.eyeVec.xyz, o.normal));
	rand = rand * _dot;

	float4 test = _tex.Sample(_samplerState, o.uv);
		return float4(test.r*rand, 0.2f, 0.2f, rand+0.4f);

	//
	


	bright = min(bright, shadowWeight);
	//return float4(bright, bright, bright, 1);
	float4 col = _tex.Sample(_samplerState, o.uv);
		float3 sphCol = _sph.Sample(_samplerState, o.normal.xy / 2 * float2(1, -1) + float2(0.5f, 0.5f));
		return float4((bright*o.diffuse.rgb + o.ambient.rgb)*col.rgb*sphCol
		/*+ 0.3f*pow(max(0, dot(ref, o.lightVec)), 8)*/, rand);//VSSetConstantBufferで渡ってきたデータは直接ピクセルシェーダでは使えないっぽい　全部の値が0になっている
	col.a = o.diffuse.a;
}

float4 BoneVS(float4 pos:POSITION, uint boneId : BONE_ID) :SV_POSITION
{
	matrix temp = mul(_world, _boneMatrix[boneId & 0x0000ffff]);
	matrix _lightVP = mul(_lightProj, _lightView);
	matrix mvp = mul(_lightVP, temp);//_lightView
	return  mul(mvp, pos);
}
float4 BonePS(float4 pos:SV_POSITION) :SV_Target
{
	return float4(0.0f, 0.0f, 0.0f, 1.0f);
}

Output PrimitiveVS(float4 pos:POSITION,float4 normal:NORMAL,float2 uv:TEXCOORD,
float4 tangent:TANGENT,float4 binormal:BINORMAL)
{
	Output o;
	matrix _camera = mul(_cameraProj, _cameraView);
	matrix m = mul(_camera, _world);
	o.pos = mul(m,pos);
	//法線も回転する（平行移動成分は消す
	matrix model_noTrans = DisableTranslation(_world);
	o.normal = normalize(mul(normal,model_noTrans));
	o.uv = uv;

	float3 posWorld = mul(pos, _world);
	o.lightVec = float4(normalize(lightPos.xyz-posWorld),0);
	o.eyeVec = float4(normalize(eyePos.xyz - posWorld),1);

	o.postest = mul(pos,m);
	matrix _lightVP = mul(_lightProj, _lightView);
	matrix lightview = mul(_lightVP, _world);//_lightView
	o.shadowpos = mul(lightview, pos);
	o.texcoord = (float2(1, 1) + (o.shadowpos.xy / o.shadowpos.w)*float2(1, -1))*0.5f;

	return o;
}
float4 PrimitivePS(Output o):SV_Target
{
	/*float h = _heightMap.Sample(_samplerState, o.uv);
	float2 heightUV = o.uv + 0.03*h*o.eyeVec.xy;*/

	float4 diffuse = float4(1.0, 1.0, 1.0, 1.0);
	float4 ambient = float4(0.4, 0.4, 0.4, 1);
	float bright = saturate(dot(o.lightVec, o.normal));
	//テクスチャから色取得
	float4 texColor = _shadowTex.Sample(_samplerState_clamp,o.uv);
	//法線テクスチャから色を取得
	float3 normalColor = _normalTex.Sample(_samplerState, o.uv);
		//ベクトルへ変換　↑で取得した時点では範囲が0~1なので-1~1になるように
		float3 normalVec = 2 * normalColor - 1.0f;
		normalVec = normalize(normalVec);

	bright = saturate(dot(o.lightVec, o.normal));//saturate(dot(-o.lightVec, normalVec));

	float2 shadowUV = (float2(1, 1) + (o.shadowpos.xy / o.shadowpos.w)*float2(1, -1))*0.5f;
		float lightviewDepth = _shadowTex.Sample(_samplerState_clamp, shadowUV).r;

	float ld = o.shadowpos.z / o.shadowpos.w;
	float2 satUV = saturate(shadowUV);
	float shadowWeight = 1.0f;
	if (shadowUV.x==satUV.x&&shadowUV.y==satUV.y&&ld > lightviewDepth+0.001f){
		shadowWeight = 0.1f;
	}

	//SSSのテスト　後で球体モデルにして試す
	float ed = o.postest.z / o.postest.w;//視点からの深度
	float thickness = abs(ld - lightviewDepth)*200.0f;
	float4 transparent = float4(diffuse.rgb*(1.0f - saturate(thickness / 0.2f)), 1);
		float sss = 1.0f - saturate(thickness / 2.4f);
	//return transparent;
	//SSS

	bright = min(bright, shadowWeight);
	//return float4(bright, bright, bright, 1);
	//return _shadowTex.Sample(_samplerState_clamp, o.uv);
	return float4((ambient.rgb+bright)/**texColor.rgb*/,1);//ambient+bright;
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

textureCUBE cubeTex;
samplerCUBE cubeSampler;

struct HUDOut
{
	float4 pos:SV_POSITION;
	float2 uv:TEXCOORD0;
};

//HUDシェーダ
//@param vs 
//@param layout
//@param ps
//@param result 成功ならS_OK 
HUDOut HUDVS(float4 pos:POSITION,float2 uv : TEXCOORD)
{
	HUDOut o;
	o.pos = mul(_world,pos);
	o.uv = uv;
	//o.pos = pos;
	return o;
}
//影のテクスチャを基に表示する
float4 HUDPS(HUDOut o):SV_Target
{
	//return (0,0,0,0);
	float r = _shadowTex.Sample(_samplerState_clamp, o.uv);
	//return r;
	r = pow(r, 50);
	return float4(r,0,0,1);
}

HUDOut BillBoardVS(float4 pos:POSITION, float2 uv:TEXCOORD)
{
	HUDOut o;
	matrix _camera = mul(_cameraProj, _cameraView);
	matrix wvp = mul(_camera, _world);
	o.pos = mul(wvp, pos);
	o.uv = uv;
	return o;
}
float4 BillBoardPS(HUDOut o):SV_Target
{
	float4 col = _tex.Sample(_samplerState, o.uv);
	return col;
	return float4(0, 0, 1, 1);
}




technique Render
{
	pass p0{
		VertexShader = compile vs_5_0 BaseVS();
		PixelShader = compile ps_5_0 BasePS();
	}
	pass p1{
		VertexShader = compile vs_5_0 BoneVS();
		PixelShader = compile ps_5_0 BonePS();
	}
};