
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

};

struct depthVS_Out {
	float4 pos:SV_POSITION;
};

Texture2D _tex:register(t0);
Texture2D _sph:register(t1);
Texture2D _spa:register(t2);

Texture2D _normalTex:register(t5);
Texture2D _heightMap:register(t6);
Texture2D _dispMap:register(t7);
Texture2D _dispMask:register(t8);
Texture2D _shadowTex:register(t10);//ライトからのレンダリング結果をテクスチャとして受け取る
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
	o.fogColor = fogColor;
	o.fog = fogCoord.x + dist*fogCoord.y;
	//スカイスフィアに他と同じ距離フォグを適用すると強くかかり過ぎたので調整
	o.fog = clamp(o.fog*2.5f, 0.0f, 1.0f);
	

	o.posWorld = mul(_world, pos);
	//高さフォグ　とりあえず決め打ちでやってみる
	float heightFog = clamp((200.0f - abs(o.posWorld.y)) / (200.0f - 40.0f), 0.0f, 1.0f);
	heightFog = pow(heightFog,3);
	o.fog = clamp(heightFog * o.fog, 0.0f, 1.0f);

	return o;
}

float4 SkySpherePS(Output o) :SV_Target
{
	
	float4 col = _tex.Sample(_samplerState, o.uv);
	//フォグをかける
	col = lerp(o.fogColor, col, o.fog);
	return col;
	
}