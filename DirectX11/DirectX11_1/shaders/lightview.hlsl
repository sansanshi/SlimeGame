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
	//•‚¢•”•ª‚Í0A”’‚¢•”•ª‚Í1‚ª“ü‚Á‚Ä‚­‚é
	displaysment = displaysment * 2 - 1.0f;//-1.0`1.0‚Ì”ÍˆÍ‚É


	float f = (sin((float)timer*0.05f) + 1.0f) / 2;//0.0`1.0‚Ì”ÍˆÍ‚É
	f *= 0.2f;//0.0`0.3‚Ì”ÍˆÍ‚É
	displaysment *= f;



	float4 normalTemp = float4(normal.xyz * displaysment, 1.0);
		float4 posTemp = pos;//posTemp = pos + normalTemp;©‚±‚¤‚·‚é‚Æx,y,z,w‚Ì[w]‚Ì—v‘f‚Ü‚Å‰ÁZ‚³‚ê‚Ä‚µ‚Ü‚¤
		//posTemp.xyz += normalTemp.xyz;//posTemp=float4(pos.xyz+normalTemp.xyz, 1.0)‚Å‚à‰Â


	matrix _lightVP = mul(_lightProj, _lightView);
	matrix tmp = mul(_lightVP/*_lightview*/, worldtemp);


	o.pos = mul(tmp, posTemp);//lightVec‚ÌŒã‚©‚çƒYƒŒ‚Ä‚é‚Á‚Û‚¢H
	o.shadowpos = mul(tmp,posTemp);

	o.shadowpos = mul(worldtemp, pos);
	o.shadowpos = mul(_lightView, o.shadowpos);
	o.nearZ = nearZ;
	o.farZ = farZ;

	o.windowSize = windowSize;
	//o.uv = uv;

	//o.lightVec = float4(normalize(lightVec));//float4(normalize(lightVec), 1.0);

	//eyeVec=’‹“_-‹“_ or ’¸“_À•W-‹“_
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
	float2 uvOffset = float2(0, (float)timer*0.01f);
		float displaysment = _dispMap.SampleLevel(_samplerStateDisp, uv + uvOffset, 0).r;//_dispMap.Sample(_samplerState, uv);
	//•‚¢•”•ª‚Í0A”’‚¢•”•ª‚Í1‚ª“ü‚Á‚Ä‚­‚é
	displaysment = displaysment * 2 - 1.0f;//-1.0`1.0‚Ì”ÍˆÍ‚É


	float f = (sin((float)timer*0.05f) + 1.0f) / 2;//0.0`1.0‚Ì”ÍˆÍ‚É
	f *= 0.5f;//0.0`0.3‚Ì”ÍˆÍ‚É
	displaysment *= f;

	matrix world = _world;

	float4 normalTemp = float4(normal.xyz * displaysment, 1.0);
		float4 posTemp = pos;//posTemp = pos + normalTemp;©‚±‚¤‚·‚é‚Æx,y,z,w‚Ì[w]‚Ì—v‘f‚Ü‚Å‰ÁZ‚³‚ê‚Ä‚µ‚Ü‚¤
		posTemp.xyz += normalTemp.xyz;//posTemp=float4(pos.xyz+normalTemp.xyz, 1.0)‚Å‚à‰Â

		matrix scale = Scalling(1.02f, 1.02f, 1.02f);
	world = mul(world, scale);

	matrix _lightVP = mul(_lightProj, _lightView);
	matrix tmp = mul(_lightVP/*_lightview*/, world);

	o.pos = mul(tmp, posTemp);//lightVec‚ÌŒã‚©‚çƒYƒŒ‚Ä‚é‚Á‚Û‚¢H
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