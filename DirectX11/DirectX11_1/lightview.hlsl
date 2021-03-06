cbuffer global:register(b0){
	matrix _world;
	matrix _cameraView;
	matrix _cameraProj;
	matrix _lightView;
	matrix _lightProj;

};

matrix TangentMatrix(float3 tangent, float3 binormal, float3 normal)
{
	matrix mat = {
		float4(normalize(tangent), 0.0f),
		float4(normalize(binormal), 0.0f),
		float4(normalize(normal), 0.0f),
		float4(0, 0, 0, 1) };
	matrix test = {
		float4(1, 0, 0, 0),
		float4(0, 1, 0, 0),
		float4(0, 0, 1, 0),
		float4(0, 0, 0, 1)
	};
	return mat;

}
matrix Scalling(float x, float y, float z)
{
	matrix mat = {
		float4(x, 0, 0, 0),
		float4(0, y, 0, 0),
		float4(0, 0, z, 0),
		float4(0, 0, 0, 1)
	};
	return mat;
}

matrix InvTangentMatrix(float3 tangent, float3 binormal, float3 normal)
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

struct Output{
	float4 pos:SV_POSITION;
	float4 shadowpos:POSITION0;
	/*float4 normal:NORMAL;
	float2 uv:TEXCOORD0;

	float4 lightVec:TEXCOORD1;
	float4 eyeVec:TEXCOORD2;

	float4x4 tangentMatrix:TANGENTMATRIX;
	float4x4 noTransMatrix:NOTRANS;*/

};

cbuffer boneMatrix:register(b3)
{
	matrix _boneMatrix[512];
};
cbuffer Global2:register(b5){
	float4 lightVec;
	float4 eyeVec;
	int timer;

};

Texture2D _dispMap:register(t7);
SamplerState _samplerStateDisp:register(s1);

Output LightViewVS(float4 pos : POSITION, float2 uv : TEXCOORD,
	float4 normal : NORMAL, uint boneid : BONE_ID, uint boneWeight : BONE_WEIGHT,
	bool edgeFlag : EDGE, float4 binormal : BINORMAL, float4 tangent : TANGENT)
{
	Output o;


	matrix worldtemp = _world;
	float boneWgt1 = (float)boneWeight / 100.0f;
	float boneWgt2 = (100.0f - (float)boneWeight) / 100.0f;

	worldtemp = mul(worldtemp, _boneMatrix[boneid & 0x0000ffff] * boneWgt1) +
		mul(worldtemp, _boneMatrix[(boneid & 0xffff0000) / 0x10000] * boneWgt2);


	float2 uvOffset = float2(0, (float)timer*0.01f);
		float displaysment = _dispMap.SampleLevel(_samplerStateDisp, uv + uvOffset, 0).r;//_dispMap.Sample(_samplerState, uv);
	//黒い部分は0、白い部分は1が入ってくる
	displaysment = displaysment * 2 - 1.0f;//-1.0〜1.0の範囲に


	float f = (sin((float)timer*0.05f) + 1.0f) / 2;//0.0〜1.0の範囲に
	f *= 0.2f;//0.0〜0.3の範囲に
	displaysment *= f;



	float4 normalTemp = float4(normal.xyz * displaysment, 1.0);
		float4 posTemp = pos;//posTemp = pos + normalTemp;←こうするとx,y,z,wの[w]の要素まで加算されてしまう
		//posTemp.xyz += normalTemp.xyz;//posTemp=float4(pos.xyz+normalTemp.xyz, 1.0)でも可

		matrix scale = Scalling(1.02f, 1.02f, 1.02f);
	worldtemp = mul(worldtemp, scale);

	matrix _lightVP = mul(_lightProj, _lightView);
	matrix tmp = mul(_lightVP/*_lightview*/, worldtemp);


	o.pos = mul(tmp, posTemp);//lightVecの後からズレてるっぽい？
	o.shadowpos = mul(tmp,posTemp);
	//o.uv = uv;

	//o.lightVec = float4(normalize(lightVec));//float4(normalize(lightVec), 1.0);

	//eyeVec=注視点-視点 or 頂点座標-視点
	//float4 eye = float4(mul(worldtemp, posTemp).xyz - eyeVec.xyz, 1);
	//o.eyeVec = float4(normalize(eyeVec));

	return o;
}

float4 LightViewPS(Output o):SV_Target
{
	//return float4(saturate(o.pos.z), saturate(o.shadowpos.z / 100), 0, 1);
	//return float4(o.pos.z/2,saturate(o.shadowpos.z/100),0,1);
	float brightness = o.shadowpos.z / o.shadowpos.w;
	//brightness=pow(brightness, 100);
	return float4(brightness, brightness, brightness, 1.0f);
}

Output PrimitiveLightViewVS(float4 pos:POSITION)
{
	Output o;
	matrix _lightVP = mul(_lightProj, _lightView);
	matrix wlp = mul(_lightVP/*_lightview*/, _world);
	o.pos = mul(wlp, pos);
	o.shadowpos = mul(wlp, pos);

	return o;
}
float4 PrimitiveLightViewPS(Output o):SV_Target
{
	//return float4(saturate(o.pos.z), saturate(o.shadowpos.z / 100), 0, 1);
	//return float4(o.pos.z / 2, saturate(o.shadowpos.z / 100), 0, 1);
	//float2 uv = (float2(1, 1) + (o.shadowpos.xy / o.shadowpos.w)*float2(1, -1))*0.5f;
	float brightness = o.shadowpos.z / o.shadowpos.w;
	//brightness=pow(brightness, 100);
	return float4(brightness, brightness, brightness, 1.0f);
}

Output SlimeLightViewVS(float4 pos:POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD)
{
	Output o;
	float2 uvOffset = float2(0, (float)timer*0.01f);
		float displaysment = _dispMap.SampleLevel(_samplerStateDisp, uv + uvOffset, 0).r;//_dispMap.Sample(_samplerState, uv);
	//黒い部分は0、白い部分は1が入ってくる
	displaysment = displaysment * 2 - 1.0f;//-1.0〜1.0の範囲に


	float f = (sin((float)timer*0.05f) + 1.0f) / 2;//0.0〜1.0の範囲に
	f *= 0.5f;//0.0〜0.3の範囲に
	displaysment *= f;

	matrix world = _world;

	float4 normalTemp = float4(normal.xyz * displaysment, 1.0);
		float4 posTemp = pos;//posTemp = pos + normalTemp;←こうするとx,y,z,wの[w]の要素まで加算されてしまう
		posTemp.xyz += normalTemp.xyz;//posTemp=float4(pos.xyz+normalTemp.xyz, 1.0)でも可

		matrix scale = Scalling(1.02f, 1.02f, 1.02f);
	world = mul(world, scale);

	matrix _lightVP = mul(_lightProj, _lightView);
	matrix tmp = mul(_lightVP/*_lightview*/, world);


	o.pos = mul(tmp, posTemp);//lightVecの後からズレてるっぽい？
	o.shadowpos = mul(tmp, posTemp);
	return o;
}

float4 SlimeLightViewPS(Output o):SV_Target
{
	float brightness = o.shadowpos.z / o.shadowpos.w;
	//brightness=pow(brightness, 100);
	return float4(brightness, brightness, brightness, 1.0f);
}