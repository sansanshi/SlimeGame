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


cbuffer boneMatrix:register(b3)
{
	matrix _boneMatrix[512];
};

cbuffer Global2:register(b5) {
	float4 lightPos;
	float4 eyePos;
	float4 fogColor;
	float2 fogCoord;
	float nearZ;
	float farZ;
	int timer;
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
};


Texture2D _tex:register(t0);
Texture2D _sph:register(t1);
Texture2D _spa:register(t2);

Texture2D _normalTex:register(t5);
Texture2D _heightMap:register(t6);
Texture2D _dispMap:register(t7);
Texture2D _dispMask:register(t8);
Texture2D _decalMap:register(t9);//デカール
Texture2D _shadowTex:register(t10);//ライトからの深度値をテクスチャとして受け取る
Texture2D _lightViewTex:register(t11);//ライトからのレンダリング（カラー）
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

	bright = saturate(dot(o.lightVec, o.normal));//saturate(dot(-o.lightVec, normalVec));
	o.shadowposCS = float4(o.shadowposCS.xyz / o.shadowposCS.w, 1.0f);
	float2 shadowUV = (float2(1, 1) + (o.shadowposCS.xy)*float2(1, -1))*0.5f;
	shadowUV += float2(0.5f / 1280.0f, 0.5f / 720.0f);
	float lightviewDepth = _shadowTex.Sample(_samplerState_clamp, shadowUV).r;

	float ld = o.shadowposVS.z / o.farZ;
	float2 satUV = saturate(shadowUV);
	float shadowWeight = 1.0f;
	if (shadowUV.x == satUV.x&&shadowUV.y == satUV.y&&ld > lightviewDepth + 0.01f) {
		shadowWeight = 0.1f;
		//return float4(lightviewDepth, 0, 0, 1);
	}

	

	bright = min(bright, shadowWeight);
	//return float4(bright, bright, bright, 1);
	//return _shadowTex.Sample(_samplerState_clamp, o.uv);
	float4 col = float4((ambient.rgb + bright)/**texColor.rgb*/,1);//ambient+bright;
																   //フォグをかける
	col.a = 0.5f;
	//col = lerp(o.fogColor, col, o.fog);
	return col;
}