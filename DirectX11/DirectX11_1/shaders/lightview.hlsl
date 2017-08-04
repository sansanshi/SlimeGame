#include"ShaderInclude.hlsli"

cbuffer global:register(b0){
	matrix _world;
	matrix _cameraView;
	matrix _cameraProj;
	matrix _lightView;
	matrix _lightProj;

};


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


struct Output{
	float4 pos:SV_POSITION;
	float4 shadowpos:POSITION0;
	/*float4 normal:NORMAL;
	float2 uv:TEXCOORD0;

	float4 lightVec:TEXCOORD1;
	float4 eyeVec:TEXCOORD2;

	float4x4 tangentMatrix:TANGENTMATRIX;
	float4x4 noTransMatrix:NOTRANS;*/

	float nearZ : NEAR;
	float farZ : FAR;
	float2 windowSize:TEXCOORD0;
};

cbuffer boneMatrix:register(b3)
{
	matrix _boneMatrix[512];
};



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
	displaysment = displaysment * 2 - 1.0f;//-1.0～1.0の範囲に


	float f = (sin((float)timer*0.05f) + 1.0f) / 2;//0.0～1.0の範囲に
	f *= 0.2f;//0.0～0.3の範囲に
	displaysment *= f;



	float4 normalTemp = float4(normal.xyz * displaysment, 1.0);
		float4 posTemp = pos;//posTemp = pos + normalTemp;←こうするとx,y,z,wの[w]の要素まで加算されてしまう
		//posTemp.xyz += normalTemp.xyz;//posTemp=float4(pos.xyz+normalTemp.xyz, 1.0)でも可


	matrix _lightVP = mul(_lightProj, _lightView);
	matrix tmp = mul(_lightVP/*_lightview*/, worldtemp);


	o.pos = mul(tmp, posTemp);//lightVecの後からズレてるっぽい？
	o.shadowpos = mul(tmp,posTemp);

	o.shadowpos = mul(worldtemp, pos);
	o.shadowpos = mul(_lightView, o.shadowpos);
	o.nearZ = nearZ;
	o.farZ = farZ;

	o.windowSize = windowSize;
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
	float d = o.shadowpos.z / o.farZ;
	//brightness=pow(brightness, 100);
	return float4(d, d*d, 0, 1.0f);
}

Output PrimitiveLightViewVS(float4 pos:POSITION)
{
	Output o;
	matrix _lightVP = mul(_lightProj, _lightView);
	matrix wlp = mul(_lightVP/*_lightview*/, _world);
	o.pos = mul(wlp, pos);
	o.shadowpos = mul(wlp, pos);

	o.shadowpos = mul(_world, pos);
	o.shadowpos = mul(_lightView, o.shadowpos);
	o.farZ = farZ;
	o.nearZ = nearZ;

	o.windowSize = windowSize;

	return o;
}
float4 PrimitiveLightViewPS(Output o):SV_Target
{
	//return float4(saturate(o.pos.z), saturate(o.shadowpos.z / 100), 0, 1);
	//return float4(o.pos.z / 2, saturate(o.shadowpos.z / 100), 0, 1);
	//float2 uv = (float2(1, 1) + (o.shadowpos.xy / o.shadowpos.w)*float2(1, -1))*0.5f;
	float d = o.shadowpos.z / o.farZ;//o.shadowpos.z / o.shadowpos.w;//
	//brightness=pow(brightness, 100);
	return float4(d, d*d, d, 1.0f);
}

Output SlimeLightViewVS(float4 pos:POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD)
{
	Output o;
	//dispMapはもうちょい大雑把な波にしてみる
	float2 uvOffset = float2((float)timer*0.002f, (float)timer*0.001f);
	float displaysment = _dispMap.SampleLevel(_samplerStateDisp, uv + uvOffset, 0).r;//_dispMap.Sample(_samplerState, uv);
																					 //黒い部分は0、白い部分は1が入ってくる
	displaysment = displaysment * 2 - 1.0f;//テクスチャから取った値を-1.0～1.0の範囲に


										   //適用率？的な値
	uvOffset = float2(/*sin((float)timer*0.0005f)*/0, (float)timer*0.002f);
	float dispMask = _dispMask.SampleLevel(_samplerStateDisp, uv + uvOffset, 0).r;
	dispMask = dispMask*2.0f;
	//dispMask *= 2.0f;

	//タイマーそのままだと速すぎるので適当に小さくする
	float f = 1;// (sin((float)timer*0.0005f) + 1.0f) / 2;//タイマーを0.0～1.0の範囲に
	f *= dispMask;//2.0f;//0.0～0.3の範囲に
	displaysment *= f;

	float4 normalTemp = float4(normal.xyz * displaysment, 1.0);
	float t = cos(timer*(1.0f / 60.0f)*6.0f);
	t = t*0.4f;
	/*if (t < -0.2f)
	{
	t = t*0.5f;
	}*/
	normalTemp.xyz = normalTemp.xyz + normal.xyz*t;

	//normalTemp.w = 1.0f;
	float4 posTemp = pos;//posTemp = pos + normalTemp;←こうするとx,y,z,wの[w]の要素まで加算されてしまう
	posTemp.xyz += normalTemp.xyz;//posTemp=float4(pos.xyz+normalTemp.xyz, 1.0)でも可

	matrix _lightVP = mul(_lightProj, _lightView);
	matrix tmp = mul(_lightVP, _world);

	o.pos = mul(tmp, posTemp);//lightVecの後からズレてるっぽい？
	tmp = mul(_lightView, _world);
	o.shadowpos = mul(tmp, posTemp);

	o.nearZ = nearZ;
	o.farZ = farZ;

	o.windowSize = windowSize;
	return o;
}

float4 SlimeLightViewPS(Output o):SV_Target
{
	float d = o.shadowpos.z / o.farZ;
	//brightness=pow(brightness, 100);
	return float4(d, d*d, d, 1.0f);
}