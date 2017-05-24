cbuffer global:register(b0){
	matrix _world;
	matrix _camera;
	matrix _lightview;

};

cbuffer Global2:register(b5){
	float4 lightPos;
	float4 eyePos;
	int timer;
	float nearZ;
	float farZ;

};
Texture2D _tex:register(t0);
Texture2D _sph:register(t1);
Texture2D _spa:register(t2);

Texture2D _normalTex:register(t5);
Texture2D _heightMap:register(t6);
Texture2D _dispMap:register(t7);
Texture2D _dispMask:register(t8);
Texture2D _shadowTex:register(t10);//ライトからのレンダリング結果をテクスチャとして受け取る
Texture2D _lightViewTex:register(t11);//ライトから見たレンダリング結果（カラー
SamplerState _samplerState:register(s0);
SamplerState _samplerStateDisp:register(s1);
SamplerState _samplerState_clamp:register(s2);
matrix DisableTranslation(matrix mat)
{
	matrix m = mat;
	m._m03 = m._m13 = m._m23 = 0;
	return m;
}
matrix InitMat()
{
	matrix mat;
	mat._m00 = mat._m01 = mat._m02 = mat._m03 = 0;
	mat._m10 = mat._m11 = mat._m12 = mat._m13 = 0;
	mat._m20 = mat._m21 = mat._m22 = mat._m23 = 0;
	mat._m30 = mat._m31 = mat._m32 = mat._m33 = 0;
	mat._m00 = mat._m11 = mat._m22 = mat._m33 = 1;
	return mat;
}

// 入力制御点
struct VS_CONTROL_POINT_OUTPUT
{
	float3 vPosition : WORLDPOS;
	// TODO: 他のスタッフの変更/追加
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
	Output.EdgeTessFactor[0] = 50;
	Output.EdgeTessFactor[1] = 50;
	Output.EdgeTessFactor[2] = 50;
	Output.EdgeTessFactor[3] = 50;
		Output.InsideTessFactor[0] = 1000; // たとえば、代わりに動的テセレーション係数を計算できます
		Output.InsideTessFactor[1] = 1000;

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
	HS_CONTROL_POINT_OUTPUT Output;

	// ここにコードを挿入して出力を計算します
	Output.vPosition = ip[i].vPosition;
	Output.normal = ip[i].normal;
	Output.uv = ip[i].uv;

	return Output;
}

struct DS_OUTPUT
{
	float4 pos:SV_POSITION;
	float4 shadowpos:POSITION1;
	float3 normal:NORMAL;
	float2 uv:TEXCOORD;
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

	float2 uvOffset = float2((float)timer*0.001f, 0);
	float d = _dispMap.SampleLevel(_samplerState, o.uv+uvOffset,0);
	d = d*2.0f - 1.0f;

	float3 temp = float3(0, 1, 0)*3.0f*d;//そのまま掛けると強すぎるので0.5くらい
	float4 postemp=float4(	o.pos.xyz +temp,1);

	matrix wvp = mul(_camera, _world);
	o.pos = mul(wvp,postemp );
	matrix lvp = mul(_lightview, _world);
	o.shadowpos = mul(lvp, postemp);

	o.normal = float3(0, 1, 0);//とりあえず平面なので今は0,1,0でやっとく

	return o;
}

float4 TessPS(DS_OUTPUT o):SV_Target
{
	float2 shadowUV = (float2(1, 1) + (o.shadowpos.xy / o.shadowpos.w)*float2(1, -1))*0.5f;
	shadowUV += float2(0.5f / 1280.0f, 0.5f / 720.0f);
	float lightviewDepth = _shadowTex.Sample(_samplerState_clamp, shadowUV).r;

	float4 addCol = _lightViewTex.Sample(_samplerState_clamp, shadowUV);

	float ld = o.shadowpos.z / o.shadowpos.w;
	float2 satUV = saturate(shadowUV);
		float shadowWeight = 1.0f;
	if (shadowUV.x == satUV.x&&shadowUV.y == satUV.y&&ld > lightviewDepth + 0.0005f){
		shadowWeight = 0.3f;
	}

	float thickness = abs(ld - lightviewDepth)*100.0f;
	float sss = 1.0f - saturate(thickness / 2.4f);
	//return addCol;
	//float4 col = float4(1, 1, 1, 1);
	float4 col = _tex.Sample(_samplerState, o.uv)/**shadowWeight*/+addCol*sss;
	return col;
}
