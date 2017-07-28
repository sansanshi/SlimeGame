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

	float4 lightVec:TEXCOORD1;
	float4 eyeVec:TEXCOORD2;

	float4x4 tangentMatrix:TANGENTMATRIX;

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

	float3 posWorld = mul(_world, pos);
	//o.lightVec = float4(normalize(lightPos.xyz - posWorld), 0);
	o.eyeVec = float4(normalize(eyePos.xyz - posWorld), 1);

	o.lightVec =float4(lightPos.xyz-posWorld.xyz, 1);//float4( normalize(directionalLightVec.xyz),1);//
	
	float4 tang, binorm, norm;
	norm =  normalize(mul(model_noTrans, normal));
	binorm = normalize(mul(model_noTrans, binormal));
	tang = normalize(mul(model_noTrans, tangent));
	matrix invTang = InvTangentMatrix(tang, binorm, norm);
	o.lightVec = mul(invTang, o.lightVec);
	o.posWorld = posWorld;

	o.postest = mul(pos, m);
	matrix _lightVP = mul(_lightProj, _lightView);
	matrix lightWV = mul(_lightView, _world);//_lightView
	o.shadowposCS = mul(mul(_lightVP, _world), pos);
	o.shadowposVS = mul(lightWV, pos);

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
	float Phase = 2.5f;
	float halfPhase = 2.5f / 2.0f;

	//ノイズテクスチャ
	float noise = _subTex.Sample(_samplerState, o.uv*5).r;
	//フロートテクスチャ
	float2 flowVector = _flowTex.Sample(_samplerState, o.uv).rg;
	flowVector = flowVector*2.0f - 1.0f;
	flowVector.x *= -1.0f;
	float Time = o.timer / 60.0f + noise;

	float flowOffs0 = fmod(Time, Phase);
	float flowOffs1 = fmod(Time + halfPhase, Phase);

	float phase0 =  flowOffs0;
	float phase1 =  flowOffs1;

	
	float3 norm0 = _normalTex.Sample(_samplerState, (o.uv * 5) + flowVector*phase0);
	float3 norm1 = _normalTex.Sample(_samplerState, (o.uv * 5) + flowVector*phase1);
	norm0 = norm0*2.0f - 1.0f;
	norm1 = norm1*2.0f - 1.0f;
	
	float4 col0 = _tex.Sample(_samplerState, o.uv*5 + flowVector*phase0 );
	float4 col1 = _tex.Sample(_samplerState, o.uv*5 + flowVector*phase1 );
	float f = (abs(halfPhase - flowOffs0) / halfPhase);
	
	float3 normT = lerp(norm0, norm1, f);
	//normT = normT*2.0f - 1.0f;
	normT = normalize(normT);
	float3 lightVec = normalize(o.lightVec.xyz);
	//normT = mul(o.tangentMatrix, normT);


	float bright = saturate(dot(lightVec, normT));//saturate(dot(-o.lightVec, normalVec));
	o.shadowposCS = float4(o.shadowposCS.xyz / o.shadowposCS.w, 1.0f);
	float2 shadowUV = (float2(1, 1) + (o.shadowposCS.xy)*float2(1, -1))*0.5f;
	shadowUV += float2(0.5f / o.windowSize.x, 0.5f / o.windowSize.y);

	float ld = o.shadowposVS.z / o.farZ;
	float shadowWeight = 1.0f;
	shadowWeight = CalcVSWeight(shadowUV, ld);

	bright = min(bright, shadowWeight);
	float lightCol = (1.0f, 1.0f, 1.0f, 1.0f);
																   //フォグをかける
	//col.a = 0.5f;
	//col = lerp(o.fogColor, col, o.fog);
	float4 col;
	col = lerp(col0, col1, f);
	col.rgb=col.rgb*bright;
	col.a = 0.4f;
	return col;
}