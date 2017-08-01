#include"ShaderInclude.hlsli"

cbuffer global:register(b0){
	matrix _world;
	matrix _cameraView;
	matrix _cameraProj;
	matrix _lightView;
	matrix _lightProj;

};


// 入力制御点
struct VS_CONTROL_POINT_OUTPUT
{
	float3 vPosition : WORLDPOS;
	float3 normal:NORMAL;
	float2 uv:TEXCOORD0;
};

VS_CONTROL_POINT_OUTPUT TessVS(float4 pos:POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD,
	float4 tangent : TANGENT, float4 binormal : BINORMAL)
{
	VS_CONTROL_POINT_OUTPUT o;
	o.vPosition = pos;
	o.normal = normal;
	o.uv = uv;
	return o;
}

// 出力制御点
struct HS_CONTROL_POINT_OUTPUT
{
	float3 vPosition : WORLDPOS; 
	float3 normal:NORMAL;
	float2 uv:TEXCOORD;

};

// 出力パッチ定数データ。
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[4]			: SV_TessFactor; // たとえば、クワド ドメインの [4] になります
	float InsideTessFactor[2]			: SV_InsideTessFactor; // たとえば、クワド ドメインの Inside[2] になります
	// TODO: 他のスタッフの変更/追加
};

#define NUM_CONTROL_POINTS 4

// パッチ定数関数
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	// ここにコードを挿入して出力を計算します
	Output.EdgeTessFactor[0] = 10;
	Output.EdgeTessFactor[1] = 10;
	Output.EdgeTessFactor[2] = 10;
	Output.EdgeTessFactor[3] = 10;
		Output.InsideTessFactor[0] = 100; // たとえば、代わりに動的テセレーション係数を計算できます
		Output.InsideTessFactor[1] = 100;

	return Output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT TessHS( 
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip, 
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	HS_CONTROL_POINT_OUTPUT o;

	// ここにコードを挿入して出力を計算します
	o.vPosition = ip[i].vPosition;
	o.normal = ip[i].normal;
	o.uv = ip[i].uv;

	return o;
}

struct DS_OUTPUT
{
	float4 pos:SV_POSITION;
	float4 shadowposVS:POSITION1;
	float4 shadowposCS:POSITION2;
	float3 normal:NORMAL;
	float2 uv:TEXCOORD;

	float nearZ:NEAR;
	float farZ:FAR;

	float4 fogColor:COLOR0;
	float fog : TEXCOORD1;
	float2 windowSize:TEXCOORD2;

	float3 lightPos:LIGHTPOS;
	float3 worldPos:WORLDPOS;
};

[domain("quad")]
DS_OUTPUT TessDS(HS_CONSTANT_DATA_OUTPUT In, float2 UV:SV_DomainLocation, const OutputPatch<HS_CONTROL_POINT_OUTPUT, 4> patch)
{
	DS_OUTPUT o;

	float3 top_u = lerp(patch[0].vPosition, patch[1].vPosition, UV.x);
	float3 bottom_u = lerp(patch[3].vPosition, patch[2].vPosition, UV.x);
	o.pos = float4(lerp(top_u, bottom_u, UV.y), 1);
	//↑ローカル座標算出

	float2 uv_top_u = lerp(patch[0].uv, patch[1].uv, UV.x);
		float2 uv_bottom_u = lerp(patch[3].uv, patch[2].uv, UV.x);
		o.uv = lerp(uv_top_u, uv_bottom_u, UV.y);//uvも↑みたいにlerpする

	float2 uvOffset = 0.0f;//float2((float)timer*0.001f, 0);
	float d = _dispMap.SampleLevel(_samplerState, o.uv+uvOffset,0);
	d = d*2.0f - 1.0f;

	float3 displacement = float3(0, 1, 0)*40.0f*d;//そのまま掛けると強すぎるので0.5くらい
	float4 postemp=float4(	o.pos.xyz +displacement,1);

	matrix wvp = mul(mul(_cameraProj,_cameraView), _world);
	o.pos = mul(wvp,postemp );
	o.worldPos = mul(_world, postemp).xyz;
	matrix lvp = mul(mul(_lightProj,_lightView),_world);
	o.shadowposCS = mul(lvp, postemp);
	o.shadowposVS = mul(mul(_lightView, _world), postemp);

	o.normal = float3(0, 1, 0);//とりあえず平面なので今は0,1,0でやっとく

	o.nearZ = nearZ;
	o.farZ = farZ;

	//float dist=
	o.fog=saturate( fogCoord.x + o.pos.w*fogCoord.y);
	o.fogColor = fogColor;

	o.windowSize = windowSize;

	o.lightPos = lightPos.xyz;
	o.worldPos = mul(_world, postemp);

	return o;
}

float4 TessPS(DS_OUTPUT o):SV_Target
{
	float2 shadowUV = (float2(1, 1) + (o.shadowposCS.xy / o.shadowposCS.w)*float2(1, -1))*0.5f;
	shadowUV += float2(0.5f / o.windowSize.x, 0.5f / o.windowSize.y);


	float ld = o.shadowposVS.z / o.farZ;
	float shadowWeight = 1.0f;
	shadowWeight = CalcVSWeight(shadowUV, ld);


	
	//光源からの距離によって明るさを決める
	/*float lightWeight = 1.0f - saturate((length(o.worldPos - o.lightPos) / 80.0f));
	shadowWeight = shadowWeight * lightWeight+0.3f;*/

	float disp = _dispMap.Sample(_samplerState, o.uv);
	float4 texCol = _tex.Sample(_samplerState, o.uv);//地面部分
	float4 subTexCol = _subTex.Sample(_samplerState, o.uv*5);//山部分
	float4 subTexCol2 = _subTex2.Sample(_samplerState, o.uv*3);//くぼみ部分

	disp = disp*2.0f - 1.0f;
	float4 col = lerp(texCol,subTexCol, saturate(disp*3.0f));
	
	if (disp < 0.0f)
	{
		col = lerp(subTexCol2, col, saturate(1.0f-abs(disp)*3.0f));
	}
	col = float4(col.rgb*shadowWeight, 1.0f);/*+addCol*sss*/
	//フォグをかける
	col = lerp(o.fogColor, col, o.fog);
	//col = float4(lightviewDepth, 0, 0, 1);
	return col;
}

float4 DepthTessPS(DS_OUTPUT o) :SV_Target
{

	float d = o.shadowposVS.z / o.farZ;
	
	return float4(d, d*d, 0, 1);
}