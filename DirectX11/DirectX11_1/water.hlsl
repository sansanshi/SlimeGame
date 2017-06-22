#include"ShaderInclude.hlsli"

cbuffer global:register(b0) {
	matrix _world;
	matrix _cameraView;
	matrix _cameraProj;
	matrix _lightView;
	matrix _lightProj;

};

cbuffer material:register(b1) {

	float3 _diffuse:COLOR;
	float _alpha;
	float3 _specularColor;
	float _specularity;
	float3 _ambient;
};


struct Output {
	float4 pos:SV_POSITION;
	float4 normal:NORMAL;
	float2 uv:TEXCOORD0;
	float4 diffuse:COLOR0;
	float3 specular:COLOR1;
	float3 ambient:COLOR2;

	float4 lightVec:TEXCOORD1;
	float4 eyeVec:TEXCOORD2;

	float4x4 tangentMatrix:TANGENTMATRIX;
	float4x4 noTransMatrix:NOTRANS;
	float4 tangent:TEXCOORD3;
	float4 binormal:TEXCOORD4;

	float4 postest:POSITION1;
	float4 shadowposVS:POSITION2;//ライト視点の位置(ViewSpace)
	float4 shadowposCS:POSITION3;//ライト視点の位置（ClippingSpace)
	float2 texcoord:TEXCOORD5;//シャドウマップ用uv座標（テスト用

	int timer : TIMER;
	float nearZ : NEAR;
	float farZ : FAR;

	float fog : TEXCOORD6;
	float4 fogColor:COLOR3;

	float2 windowSize:TEXCOORD7;
};






Output WaterVS(float4 pos:POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD,
	float4 tangent : TANGENT, float4 binormal : BINORMAL)
{
	Output o;
	matrix _camera = mul(_cameraProj, _cameraView);
	matrix m = mul(_camera, _world);
	o.pos = mul(m, pos);
	//法線も回転する（平行移動成分は消す
	matrix model_noTrans = DisableTranslation(_world);
	o.normal = normalize(mul(model_noTrans, normal));
	o.uv = uv;

	float3 posWorld = mul(_world, pos);
	o.lightVec = float4(normalize(lightPos.xyz - posWorld), 0);
	o.eyeVec = float4(normalize(eyePos.xyz - posWorld), 1);

	matrix invTang = InvTangentMatrix(float4(1, 0, 0, 0), float4(0, 1, 0, 0), float4(0, 0, 1, 0));
	o.lightVec = mul(invTang, o.lightVec);


	o.postest = mul(pos, m);
	matrix _lightVP = mul(_lightProj, _lightView);
	matrix lightview = mul(_lightView, _world);//_lightView
	o.shadowposCS = mul(mul(_lightVP, _world), pos);
	o.shadowposVS = mul(_world, pos);
	o.shadowposVS = mul(_lightView, o.shadowposVS);
	o.texcoord = (float2(1, 1) + (o.pos.xy / o.pos.w)*float2(1, -1))*0.5f;

	o.farZ = farZ;
	o.nearZ = nearZ;

	float dist = length(mul(_world, pos).xyz - eyePos.xyz);
	o.fog = fogCoord.x + dist*fogCoord.y;
	o.fogColor = fogColor;

	o.windowSize = windowSize;
	return o;
}
float4 WaterPS(Output o) :SV_Target
{

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

	bright = saturate(dot(o.lightVec, normalVec));//saturate(dot(-o.lightVec, normalVec));
	o.shadowposCS = float4(o.shadowposCS.xyz / o.shadowposCS.w, 1.0f);
	float2 shadowUV = (float2(1, 1) + (o.shadowposCS.xy)*float2(1, -1))*0.5f;
	shadowUV += float2(0.5f / o.windowSize.x, 0.5f / o.windowSize.y);
	float lightviewDepth = _shadowTex.Sample(_samplerState_clamp, shadowUV).r;

	float ld = o.shadowposVS.z / o.farZ;
	float2 satUV = saturate(shadowUV);
	float shadowWeight = 1.0f;
	if (shadowUV.x == satUV.x&&shadowUV.y == satUV.y&&ld > lightviewDepth + 0.01f) {
		shadowWeight = 0.1f;
		//return float4(lightviewDepth, 0, 0, 1);
	}
	//return float4(normalVec, 1);
	return float4(bright, bright, bright, 1);

	bright = min(bright, shadowWeight);
	//return float4(bright, bright, bright, 1);
	//return _shadowTex.Sample(_samplerState_clamp, o.uv);
	float4 col = float4((ambient.rgb + bright)/**texColor.rgb*/,1);//ambient+bright;
																   //フォグをかける
	col.a = 0.5f;
	//col = lerp(o.fogColor, col, o.fog);
	col = _tex.Sample(_samplerState, o.uv);
	float4 subCol = _subTex.Sample(_samplerState, o.uv);
	col=col*subCol;
	return col;
}