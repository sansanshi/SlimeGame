
Texture2D _tex:register(t0);
Texture2D _sph:register(t1);
Texture2D _spa:register(t2);

Texture2D _normalTex:register(t5);
Texture2D _heightMap:register(t6);
Texture2D _dispMap:register(t7);
Texture2D _dispMask:register(t8);
Texture2D _decalTex:register(t9);//デカール
Texture2D _shadowTex:register(t10);//ライトからの深度値をテクスチャとして受け取る
Texture2D _blurShadowTex:register(t11);
Texture2D _cameraDepthTex:register(t12);
Texture2D _subTex:register(t13);//サブテクスチャ
Texture2D _flowTex:register(t14);
Texture2D _subTex2:register(t15);
SamplerState _samplerState:register(s0);
SamplerState _samplerStateDisp:register(s1);
SamplerState _samplerState_clamp:register(s2);


matrix TangentMatrix(float4 tangent, float4 binormal, float4 normal)
{
	matrix mat = {
		float4(normalize(tangent.xyz),0),
		float4(normalize(binormal.xyz),0),
		float4(normalize(normal.xyz),0),
		float4(0, 0, 0, 1) };
	return mat;

}
matrix TangentMatrix(float3 tangent, float3 binormal, float3 normal)
{
	matrix mat = {
		float4(normalize(tangent),0),
		float4(normalize(binormal),0),
		float4(normalize(normal),0),
		float4(0, 0, 0, 1) };
	return mat;

}

matrix InvTangentMatrix(float4 tangent, float4 binormal, float4 normal)
{
	matrix r = TangentMatrix(tangent, binormal, normal);
	return transpose(r);
}
matrix InvTangentMatrix(float3 tangent, float3 binormal, float3 normal)
{
	matrix r = TangentMatrix(tangent, binormal, normal);
	return transpose(r);
}

matrix DisableTranslation(matrix mat)
{
	matrix m = mat;
	m._m03 = m._m13 = m._m23 = 0.0f;
	return m;
}

float GetRandomNumber(float2 texCoord, int Seed)
{
	return frac(sin(dot(texCoord.xy, float2(12.9898, 78.233)) + Seed) * 43758.5453);
}


cbuffer Global2:register(b5) {
	float4 lightPos;
	float4 eyePos;
	float4 directionalLightVec;
	float4 fogColor;
	float2 fogCoord;
	float2 windowSize;
	float nearZ;
	float farZ;
	int timer;

};

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

//CalculateVarianceShadowWeight
//shadowUV:ライト空間のスクリーン座標　lightDepth:ライトビュー空間深度
float CalcVSWeight(float2 shadowUV, float ld)
{
	float2 satUV = saturate(shadowUV);
	float4 d = _blurShadowTex.Sample(_samplerState, shadowUV);
	if (shadowUV.x == satUV.x&&shadowUV.y == satUV.y&&d.r < ld - 0.005f
		&&satUV.x<0.95f&&satUV.y>0.05f)
	{
		//↑ライトからのビューの切れ目に影ができるので応急処置
		//線形補間、サンプリング（WRAP）のせいではなかったっぽい　後で直す	
		float variance = d.g - d.r*d.r;
		float md = ld - d.r;
		float p = variance / (variance + (md*md));

		float ret = saturate(max(p, 0.3f));
		//ret = lerp(0.2f,1.0f, ret);
		return ret;
	}
	return 1.0f;

};