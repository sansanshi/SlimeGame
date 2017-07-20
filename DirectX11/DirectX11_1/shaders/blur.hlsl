#include"ShaderInclude.hlsli"

cbuffer blurWeightBuff : register(b0)
{
	float  weight0;
	float  weight1;
	float  weight2;
	float  weight3;
	float  weight4;
	float  weight5;
	float  weight6;
	float  weight7;
	/*float  OffsetX;
	float  OffsetY;*/
};

struct VS_Out
{
	float4 pos    : SV_POSITION;
	float2 texcoord:TEXCOORD0;
	float2 windowSize:TEXCOORD1;
};

VS_Out BlurFilterVS(float4 pos:POSITION, float2 uv : TEXCOORD)
{
	VS_Out o;

	o.pos = pos;
	o.texcoord = uv;
	o.windowSize = windowSize;

	return o;
}

float4 XBlurFilterPS(VS_Out In):SV_Target
{
	float4 col;
float4 test = _tex.Sample(_samplerState, In.texcoord);

	     col = weight0 * _tex.Sample(_samplerState_clamp, saturate(In.texcoord));
	     col += weight1
		 *(_tex.Sample(_samplerState, saturate(In.texcoord + float2(+2.0f / In.windowSize.x, 0)))
			      +_tex.Sample(_samplerState, saturate(In.texcoord + float2(-2.0f / In.windowSize.x, 0))));
		 if (weight2 < 0.0f) { return float4(0, 0, 0, 1); }
	     col += weight2
		 *(_tex.Sample(_samplerState, saturate(In.texcoord + float2(+4.0f / In.windowSize.x, 0)))
			      +_tex.Sample(_samplerState, saturate(In.texcoord + float2(-4.0f / In.windowSize.x, 0))));
	     col += weight3
		 *(_tex.Sample(_samplerState, saturate(In.texcoord + float2(+6.0f / In.windowSize.x, 0)))
			      +_tex.Sample(_samplerState, saturate(In.texcoord + float2(-6.0f / In.windowSize.x, 0))));
	     col += weight4
		 *(_tex.Sample(_samplerState, saturate(In.texcoord + float2(+8.0f / In.windowSize.x, 0)))
			      +_tex.Sample(_samplerState, saturate(In.texcoord + float2(-8.0f / In.windowSize.x, 0))));
	     col += weight5
		 *(_tex.Sample(_samplerState, saturate(In.texcoord + float2(+10.0f / In.windowSize.x, 0)))
			      +_tex.Sample(_samplerState, saturate(In.texcoord + float2(-10.0f / In.windowSize.x, 0))));
	     col += weight6
		 *(_tex.Sample(_samplerState, saturate(In.texcoord + float2(+12.0f / In.windowSize.x, 0)))
			      +_tex.Sample(_samplerState, saturate(In.texcoord + float2(-12.0f / In.windowSize.x, 0))));
	     col += weight7
			 *(_tex.Sample(_samplerState, saturate(In.texcoord + float2(+14.0f / In.windowSize.x, 0)))
				      +_tex.Sample(_samplerState, saturate(In.texcoord + float2(-14.0f / In.windowSize.x, 0))));

		 return col;
}

float4 YBlurFilterPS(VS_Out In):SV_Target
{
	float4 col;

	col = weight0 * _tex.Sample(_samplerState_clamp, saturate(In.texcoord));
	col += weight1
		*(_tex.Sample(_samplerState, saturate(In.texcoord + float2( 0 , +2.0f / In.windowSize.y)))
			+ _tex.Sample(_samplerState, saturate(In.texcoord + float2( 0 , -2.0f / In.windowSize.y))));
	col += weight2
		*(_tex.Sample(_samplerState, saturate(In.texcoord + float2( 0 , +4.0f / In.windowSize.y)))
			+ _tex.Sample(_samplerState, saturate(In.texcoord + float2( 0 , -4.0f / In.windowSize.y))));
	col += weight3
		*(_tex.Sample(_samplerState, saturate(In.texcoord + float2( 0 , +6.0f / In.windowSize.y)))
			+ _tex.Sample(_samplerState, saturate(In.texcoord + float2( 0 , -6.0f / In.windowSize.y))));
	col += weight4
		*(_tex.Sample(_samplerState, saturate(In.texcoord + float2( 0 , +8.0f / In.windowSize.y)))
			+ _tex.Sample(_samplerState, saturate(In.texcoord + float2( 0 , -8.0f / In.windowSize.y))));

	col += weight5
		*(_tex.Sample(_samplerState, saturate(In.texcoord + float2( 0 , +10.0f / In.windowSize.y)))
			+ _tex.Sample(_samplerState, saturate(In.texcoord + float2( 0 , -10.0f / In.windowSize.y))));
	col += weight6
		*(_tex.Sample(_samplerState, saturate(In.texcoord + float2( 0 , +12.0f / In.windowSize.y)))
			+ _tex.Sample(_samplerState, saturate(In.texcoord + float2( 0 , -12.0f / In.windowSize.y))));
	col += weight7
		*(_tex.Sample(_samplerState, saturate(In.texcoord + float2( 0 , +14.0f / In.windowSize.y)))
			+ _tex.Sample(_samplerState, saturate(In.texcoord + float2( 0 , -14.0f / In.windowSize.y))));
	return col;
}