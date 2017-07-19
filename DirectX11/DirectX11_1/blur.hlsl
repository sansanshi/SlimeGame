cbuffer blurWeightBuff : register(b0)
{
	float  weight0;    // �d��
	float  weight1;    // �d��
	float  weight2;    // �d��
	float  weight3;    // �d��
	float  weight4;    // �d��
	float  weight5;    // �d��
	float  weight6;    // �d��
	float  weight7;    // �d��
	float  OffsetX;
	float  OffsetY;
};

struct VS_Out
{
	float4 pos    : SV_POSITION;
	float2 texcoord:TEXCOORD0;
};

VS_Out BlurXFilterVS(float4 pos:POSITION, float2 uv : TEXCOORD)
{
	VS_Out o;

	o.pos = pos;
	o.texcoord = uv;


	return o;
}

float4 BlurXFilterPS(VS_Out In)
{
	float4 col;

	     col = weight0 * _mainTex.Sample(_samplerState, In.Tex);
	     col += weight1
		 *(_mainTex.Sample(_samplerState, In.texcoord + float2(+2.0f / windowSize.x, 0))
			      +_mainTex.Sample(_samplerState, In.texcoord + float2(-2.0f / windowSize.x, 0)));
	     col += weight2
		 *(_mainTex.Sample(_samplerState, In.texcoord + float2(+4.0f / windowSize.x, 0))
			      +_mainTex.Sample(_samplerState, In.texcoord + float2(-4.0f / windowSize.x, 0)));
	     col += weight3
		 *(_mainTex.Sample(_samplerState, In.texcoord + float2(+6.0f / windowSize.x, 0))
			      +_mainTex.Sample(_samplerState, In.texcoord + float2(-6.0f / windowSize.x, 0)));
	     col += weight4
		 *(_mainTex.Sample(_samplerState, In.texcoord + float2(+8.0f / windowSize.x, 0))
			      +_mainTex.Sample(_samplerState, In.texcoord + float2(-8.0f / windowSize.x, 0)));
	     col += weight5
		 *(_mainTex.Sample(_samplerState, In.texcoord + float2(+10.0f / windowSize.x, 0))
			      +_mainTex.Sample(_samplerState, In.texcoord + float2(-10.0f / windowSize.x, 0)));
	     col += weight6
		 *(_mainTex.Sample(_samplerState, In.texcoord + float2(+12.0f / windowSize.x, 0))
			      +_mainTex.Sample(_samplerState, In.texcoord + float2(-12.0f / windowSize.x, 0)));
	     col += weight7
			 *(_mainTex.Sample(_samplerState, In.texcoord + float2(+14.0f / windowSize.x, 0))
				      +_mainTex.Sample(_samplerState, In.texcoord + float2(-14.0f / windowSize.x, 0)));

}