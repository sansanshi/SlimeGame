
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
	float3 posWorld:WORLDPOS;

	float4 lightVec:TEXCOORD1;
	float4 eyeVec:TEXCOORD2;

	float4x4 tangentMatrix:TANGENTMATRIX;
	float4x4 noTransMatrix:NOTRANS;
	float4 tangent:TEXCOORD3;
	float4 binormal:TEXCOORD4;


	float timer : TIMER;
	float nearZ : NEAR_Z;
	float farZ : FAR_Z;

	float4 fogColor:COLOR3;
	float fog : TEXCOORD5;

	float2 windowSize:TEXCOORD6;

};

struct depthVS_Out {
	float4 pos:SV_POSITION;
};






Output SkySphereVS(float4 pos:POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD,
	float4 tangent : TANGENT, float4 binormal : BINORMAL)
{
	Output o;
	matrix world_noTr = DisableTranslation(_world);
	o.noTransMatrix = world_noTr;
	o.normal = normalize(mul(world_noTr, normal));
	o.binormal = normalize(mul(world_noTr, binormal));
	o.tangent = normalize(mul(world_noTr, tangent));

	o.tangentMatrix = TangentMatrix(o.tangent, o.binormal, o.normal);

	float2 uvOffset = float2((float)timer*0.002f, (float)timer*0.001f);
	
	uvOffset = float2(/*sin((float)timer*0.0005f)*/0, (float)timer*0.002f);
	


	
	matrix cameraVP = mul(_cameraProj, _cameraView);
	matrix cameraWVP = mul(cameraVP, _world);

	o.pos = mul(cameraWVP, pos);//lightVecの後からズレてるっぽい？
							  //ノーマルマップをスクロールさせるために渡すuvをずらす
							  //ピクセルシェーダにタイマ渡して計算するより若干計算量少なくなる？
	uvOffset = float2((float)timer*0.00008f, (float)timer*0.00002f);
	o.uv = uv;
	o.diffuse.rgb = _diffuse;
	o.diffuse.a = _alpha;
	o.specular = _specularColor*_specularity;
	o.ambient = _ambient;


	o.timer = timer;
	o.nearZ = nearZ;
	o.farZ = farZ;

	float dist = length(mul(_world, pos).xyz - eyePos.xyz);
	dist = length(mul(mul(_cameraView, _world), pos));
	o.fogColor = fogColor;
	//o.fog = fogCoord.x + dist*fogCoord.y;
	//スカイスフィアに他と同じ距離フォグを適用すると見えなくなるので調整
	o.fog = (350.0f - dist) / (350.0f - 40.0f);
	o.fog = clamp(o.fog*4.5f, 0.0f, 1.0f);
	

	o.posWorld = mul(_world, pos);
	//高さフォグ　とりあえず決め打ちでやってみる
	float heightFog = clamp((300.0f - abs(o.posWorld.y)) / (300.0f - 50.0f), 0.0f, 1.0f);
	heightFog = pow(heightFog,3);
	o.fog = clamp(heightFog * o.fog, 0.0f, 1.0f);
	
	o.windowSize = windowSize;

	return o;
}

float4 SkySpherePS(Output o) :SV_Target
{
	
	float4 col = _tex.Sample(_samplerState, o.uv);
	//フォグをかける
	col = lerp(o.fogColor, col, o.fog);
	return col;
	
}