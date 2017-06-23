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
	float3 posWorld:POSITION4;
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
	o.binormal = normalize(mul(model_noTrans, binormal));
	o.tangent = normalize(mul(model_noTrans, tangent));

	float3 posWorld = mul(_world, pos);
	//o.lightVec = float4(normalize(lightPos.xyz - posWorld), 0);
	o.eyeVec = float4(normalize(eyePos.xyz - posWorld), 1);

	//matrix invTang = InvTangentMatrix(float4(1, 0, 0, 0), float4(0, 1, 0, 0), float4(0, 0, 1, 0));
	//o.lightVec = mul(invTang, o.lightVec);
	o.lightVec = float4(lightPos.xyz, 1);
	o.tangentMatrix = TangentMatrix(o.tangent, o.binormal, o.normal);
	o.posWorld = posWorld;

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
	o.timer = timer;
	return o;
}
float4 WaterPS(Output o) :SV_Target
{
	float Phase = 0.15f;
float halfPhase = 0.15f / 2.0f;

//ノイズテクスチャ
float noise = _subTex.Sample(_samplerState, o.uv).r;
//フロートテクスチャ
float2 flowVector = _flowTex.Sample(_samplerState, o.uv).rg;
flowVector = flowVector*2.0f - 1.0f;

float flowOffs0 = fmod(o.timer / 60.0f + noise, Phase);
float flowOffs1 = fmod(o.timer / 60.0f + halfPhase+noise, Phase);

float phase0 =  flowOffs0;
float phase1 =  flowOffs1;

float3 norm0 = _normalTex.Sample(_samplerState, (o.uv * 4) + flowVector*phase0);
float3 norm1 = _normalTex.Sample(_samplerState, (o.uv * 2) + flowVector*phase1);

float f = (abs(halfPhase - flowOffs0) / halfPhase);

float3 normT = lerp(norm0, norm1, f);
normT = normalize(normT);
float3 lightVec = normalize(o.lightVec.xyz - o.posWorld);
normT = mul(o.tangentMatrix, normT);

float b = saturate(dot(lightVec, normT));
return float4(b, b, b, 1);
//return float4(normT, 1);

//float t = (float)o.timer / 60.0f + noise;
//float phase0 = fmod(t, Phase);
//float phase1 = fmod(t - halfPhase, Phase);
//
//float2 shiftUV;
//shiftUV.x = floor(t / Phase)*2.0f;
//shiftUV.y = phase0 > halfPhase ? shiftUV.x + 1.0f : shiftUV.x - 1.0f;
//shiftUV *= 0.125f;
//
//float alpha = abs(halfPhase - phase0) / halfPhase;
//
//
//float4 normal0 = _normalTex.Sample(_samplerState, flowVector*phase0 + shiftUV.x);
//float4 normal1 = _normalTex.Sample(_samplerState, flowVector*phase1 + shiftUV.y);
//float4 mix = lerp(normal0, normal1, alpha);
//mix = mix*2.0f - 1.0f;
//
//return float4(mix.rgb, 1);

	float time = (float)o.timer / 60.0f;
	float time2 = (float)o.timer / 60.0f + 30.0f;
	//float2 uvOffset = float2(sin(o.timer*0.002f),cos(o.timer*0.005f));



	float2 uvOffset = float2(time*0.05f, 0);
	float2 uvOffset2 = float2(time2*0.05f, 0);
	//法線テクスチャから色を取得
	float3 normalColor = _normalTex.Sample(_samplerState, o.uv+uvOffset);
	float3 normal2 = _normalTex.Sample(_samplerState, o.uv + uvOffset2);
	normalColor = lerp(normalColor, normal2, cos(time*2.0f));
	//ベクトルへ変換　↑で取得した時点では範囲が0~1なので-1~1になるように
	float3 normalVec = 2 * normalColor - 1.0f;
	normalVec = normalize(normalVec);

	float bright = saturate(dot(lightVec, normT));//saturate(dot(-o.lightVec, normalVec));
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
	//return float4(bright, bright, bright, 1);

	bright = min(bright, shadowWeight);
																   //フォグをかける
	//col.a = 0.5f;
	//col = lerp(o.fogColor, col, o.fog);
	float4 col = _tex.Sample(_samplerState, o.uv);
	float4 subCol = _subTex.Sample(_samplerState, o.uv);
	col.rgb=col.rgb*bright;
	return col;
}