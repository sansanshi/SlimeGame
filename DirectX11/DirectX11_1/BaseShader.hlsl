//�R���X�^���g�o�b�t�@�̓������̉�Ƃ��đ����Ă���̂�
//float3�ő����Ă�������float4�Ŏ󂯎�����肷���float(4�o�C�g)���Y����
//���_�V�F�[�_�̈�����float3�ő����Ă����f�[�^��float4�Ŏ󂯎���Ă�
//����ɑ�4�v�f��1�����Ă������ۂ��i�H�j

cbuffer global:register(b0){
	matrix _world;
	matrix _cameraView;
	matrix _cameraProj;
	matrix _lightView;
	matrix _lightProj;

};

cbuffer material:register(b1){

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

cbuffer Global2:register(b5){
	float4 lightPos;
	float4 eyePos;
	int timer;

};


struct Output{
	float4 pos:SV_POSITION;
	float4 normal:NORMAL;
	float2 uv:TEXCOORD0;
	float4 diffuse:COLOR0;
	float3 specular:COLOR1;//�Z�}���e�B�N�X��ݒ肵�Ȃ��ƃG���[�f���i�H�j
	float3 ambient:COLOR2;

	float4 lightVec:TEXCOORD1;
	float4 eyeVec:TEXCOORD2;

	float4x4 tangentMatrix:TANGENTMATRIX;
	float4x4 noTransMatrix:NOTRANS;
	float4 tangent:TEXCOORD3;
	float4 binormal:TEXCOORD4;

	float4 postest:POSITION1;
	float4 shadowpos:POSITION2;
	float2 texcoord:TEXCOORD5;//�V���h�E�}�b�v�i�e�X�g�p

	int timer : TIMER;

};

struct depthVS_Out{
	float4 pos:SV_POSITION;
};

Texture2D _tex:register(t0);
Texture2D _sph:register(t1);
Texture2D _spa:register(t2);

Texture2D _normalTex:register(t5);
Texture2D _heightMap:register(t6);
Texture2D _dispMap:register(t7);
Texture2D _dispMask:register(t8);
Texture2D _decalMap:register(t9);//�f�J�[��
Texture2D _shadowTex:register(t10);//���C�g����̃����_�����O���ʂ��e�N�X�`���Ƃ��Ď󂯎��
Texture2D _lightViewTex:register(t11);
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

float GetRandomNumber(float2 texCoord, int Seed)
{
	return frac(sin(dot(texCoord.xy, float2(12.9898, 78.233)) + Seed) * 43758.5453);
}

//�������i�V�F�[�_���j�Œ�`�ς݂̃Z�}���e�B�N�X�ɑΉ�����InputLayout���Ȃ���
//CreateInputLayout�̎��_�ŃG���[�H
//main������float3��float4�Ŏ󂯎��Ƃ��̗v�f�ɂ�1.0�������Ă���
//�uw�v�̗v�f�ɋC��t���Čv�Z����
Output BaseVS(float4 pos : POSITION, float2 uv : TEXCOORD,
	float4 normal : NORMAL, uint boneid : BONE_ID, uint boneWeight : BONE_WEIGHT,
	bool edgeFlag : EDGE, float4 binormal : BINORMAL, float4 tangent : TANGENT)
{
	Output o;


	matrix worldtemp = _world;
	float boneWgt1 = (float)boneWeight / 100.0f;
	float boneWgt2 = (100.0f - (float)boneWeight) / 100.0f;

	worldtemp = mul(worldtemp, _boneMatrix[boneid & 0x0000ffff] * boneWgt1) +
		mul(worldtemp, _boneMatrix[(boneid & 0xffff0000) / 0x10000] * boneWgt2);


	matrix worldtemp_noTr = DisableTranslation(worldtemp);
	o.noTransMatrix = worldtemp_noTr;

	//�{�[���s��ɂ͕��s�ړ��������܂܂�Ă�̂Ŗ@���ɓK�p����Ƃ��͕��s�ړ�����������
	o.normal = normalize(mul(worldtemp_noTr,normal));
	//o.normal.w = 1.0f;
	o.tangent = normalize(mul(worldtemp_noTr, tangent));
	o.binormal = normalize(mul(worldtemp_noTr, binormal));

	//o.tangentMatrix = TangentMatrix(normalize(o.tangent), normalize(o.binormal), normalize(o.normal));
	o.tangentMatrix = TangentMatrix(o.tangent, o.binormal, o.normal);

	float2 uvOffset = float2(0, (float)timer*0.01f);
		float displaysment = _dispMap.SampleLevel(_samplerStateDisp, uv + uvOffset, 0).r;//_dispMap.Sample(_samplerState, uv);
	//����������0�A����������1�������Ă���
	displaysment = displaysment * 2 - 1.0f;//-1.0�`1.0�͈̔͂�


	float f = (sin((float)timer*0.05f) + 1.0f) / 2;//0.0�`1.0�͈̔͂�
	f *= 0.2f;//0.0�`0.3�͈̔͂�
	displaysment *= f;

	

	float4 normalTemp = float4(normal.xyz * displaysment, 1.0);
		//normalTemp.w = 1.0f;
		float4 posTemp = pos;//posTemp = pos + normalTemp;�����������x,y,z,w��[w]�̗v�f�܂ŉ��Z����Ă��܂�
		//posTemp.xyz += normalTemp.xyz;//posTemp=float4(pos.xyz+normalTemp.xyz, 1.0)�ł���

		matrix _camera = mul(_cameraProj, _cameraView);
	matrix tmp = mul(_camera, worldtemp);

	
	o.pos = mul(tmp, posTemp);//lightVec�̌ォ��Y���Ă���ۂ��H
	o.uv = uv;
	o.diffuse.rgb = _diffuse;
	o.diffuse.a = _alpha;
	o.specular = _specularColor*_specularity;
	o.ambient = _ambient;

	float4 posWorld = mul( worldtemp,pos);
		o.lightVec = float4(normalize(posWorld.xyz-lightPos.xyz ), 0);//float4(normalize(lightVec), 1.0);

	//eyeVec=���_���W-���_
	o.eyeVec = float4(normalize(eyePos.xyz-posWorld.xyz),1);

	o.postest = mul(tmp, posTemp);
	matrix _lightVP = mul(_lightProj, _lightView);
	matrix lightview = mul(_lightVP, worldtemp);//_lightView
	o.shadowpos = mul(lightview, posTemp);

	o.timer = timer;

	return o;
}

float4 BasePS(Output o) :SV_Target
{

	/*float h = _heightMap.Sample(_samplerState, o.uv);

	float2 heightUV = o.uv + 0.03*h*o.eyeVec.xy;*/

	//�@���e�N�X�`������F���擾
	float3 normalColor = _normalTex.Sample(_samplerState, o.uv).rgb;
	//�x�N�g���֕ϊ��@���Ŏ擾�������_�ł͔͈͂�0~1�Ȃ̂�-1~1�ɂȂ�悤��
	float3 normalVec = 2 * normalColor - 1.0f;
	normalVec = normalize(normalVec);
	//���͒��_�V�F�[�_�̕��ŏ���������itangentMatrix������Ă������ɓn��
	/*float3 ul = float3(normalVec.x*o.tangent.x, normalVec.x*o.tangent.y, normalVec.x*o.tangent.z);
	float3 vl = float3(normalVec.y*o.binormal.x, normalVec.y*o.binormal.y, normalVec.y*o.binormal.z);
	float3 zl = float3(normalVec.z*o.normal.x, normalVec.z*o.normal.y, normalVec.z*o.normal.z);
	float3 n_local = ul + vl + zl;*/
	//normalVec = n_local;
	float3 n_local = mul(normalVec, o.tangentMatrix);//����Ƀ��f���̉�]�Ƃ��K�p����ĂȂ������H
		//n_local = mul(o.noTransMatrix, n_local);
	//�����_�V�F�[�_�̕��ŉ�]���normal�Abinormal�Atangent�g����tangentMatrix���������v��Ȃ��H
	float3 ref = reflect(-o.eyeVec, n_local);

	//saturate��0�`1�ɃN�����v����֐�
	//�@���@�����x�N�g���́u�t�v�x�N�g���Ƃ̓��ς����
	float bright = saturate(dot(-o.lightVec, o.normal/*n_local*/));
	//return float4(bright, bright, bright, 1);
	//float4 col = v.diffuse*bright;//float4(v.diffuse*bright, v.diffuse*bright, v.diffuse*bright, 0);//_tex.Sample(_samplerState, v.uv);

	//v.diffuse�ɂ�RGBA�������Ă��āAbright�ɂ�0~1�̒l�������Ă���
	//2����Z�����Alpha�l�ɂ�bright��0~1���������Ă��܂��̂ŁA�{���Â��Ȃ�ꏊ�������ɂȂ��Ă��܂�
	//float4 col = bright*(v.diffuse + ambient)*_tex.Sample(_samplerState, v.uv);//*sphcol;//bright��0~1�̒l������Ă���
	//float3 sphcol = _sph.Sample(sample, v.normal.xy*float2(0.5f,-0.5f)+ / 2 + 0.5f);//sph�K�p
	//spa��nulltextureb sph��nulltexture
	//return float4(o.normal.x, o.normal.y, o.normal.z, 1.0f);

	float2 shadowUV = (float2(1, 1) + (o.shadowpos.xy / o.shadowpos.w)*float2(1, -1))*0.5f;
	float lightviewDepth = _shadowTex.Sample(_samplerState_clamp, shadowUV).r;

	float ld = o.shadowpos.z / o.shadowpos.w;
	float shadowWeight = 1.0f;
	if (ld > lightviewDepth+0.01f){
		shadowWeight = 0.1f;
	}

	//SSS�̃e�X�g�@��ŋ��̃��f���ɂ��Ď���
	float ed = o.postest.z / o.postest.w;
	float thickness = abs(ld - lightviewDepth)*100.0f;
	float4 transparent = float4(_tex.Sample(_samplerState, o.uv).rgb*(1.0f - saturate(thickness / 0.2f)), 1);
		transparent = _tex.Sample(_samplerState, o.uv)*(1.0f - saturate(thickness / 3.5f));
	float sss = 1.0f - saturate(thickness / 3.4f);
	//return transparent;
	//SSS

	//�W���W���\���e�X�g
	float rand = GetRandomNumber(o.uv,(int)(o.timer));
	float _dot = saturate(dot(o.eyeVec.xyz, o.normal));
	rand = rand * _dot;

	float4 test = _tex.Sample(_samplerState, o.uv);
		return float4(test.r*rand, 0.2f, 0.2f, rand+0.4f);

	//
	


	bright = min(bright, shadowWeight);
	//return float4(bright, bright, bright, 1);
	float4 col = _tex.Sample(_samplerState, o.uv);
		float3 sphCol = _sph.Sample(_samplerState, o.normal.xy / 2 * float2(1, -1) + float2(0.5f, 0.5f));
		return float4((bright*o.diffuse.rgb + o.ambient.rgb)*col.rgb*sphCol
		/*+ 0.3f*pow(max(0, dot(ref, o.lightVec)), 8)*/, rand);//VSSetConstantBuffer�œn���Ă����f�[�^�͒��ڃs�N�Z���V�F�[�_�ł͎g���Ȃ����ۂ��@�S���̒l��0�ɂȂ��Ă���
	col.a = o.diffuse.a;
}

float4 BoneVS(float4 pos:POSITION, uint boneId : BONE_ID) :SV_POSITION
{
	matrix temp = mul(_world, _boneMatrix[boneId & 0x0000ffff]);
	matrix _lightVP = mul(_lightProj, _lightView);
	matrix mvp = mul(_lightVP, temp);//_lightView
	return  mul(mvp, pos);
}
float4 BonePS(float4 pos:SV_POSITION) :SV_Target
{
	return float4(0.0f, 0.0f, 0.0f, 1.0f);
}

Output PrimitiveVS(float4 pos:POSITION,float4 normal:NORMAL,float2 uv:TEXCOORD,
float4 tangent:TANGENT,float4 binormal:BINORMAL)
{
	Output o;
	matrix _camera = mul(_cameraProj, _cameraView);
	matrix m = mul(_camera, _world);
	o.pos = mul(m,pos);
	//�@������]����i���s�ړ������͏���
	matrix model_noTrans = DisableTranslation(_world);
	o.normal = normalize(mul(normal,model_noTrans));
	o.uv = uv;

	float3 posWorld = mul(pos, _world);
	o.lightVec = float4(normalize(lightPos.xyz-posWorld),0);
	o.eyeVec = float4(normalize(eyePos.xyz - posWorld),1);

	o.postest = mul(pos,m);
	matrix _lightVP = mul(_lightProj, _lightView);
	matrix lightview = mul(_lightVP, _world);//_lightView
	o.shadowpos = mul(lightview, pos);
	o.texcoord = (float2(1, 1) + (o.shadowpos.xy / o.shadowpos.w)*float2(1, -1))*0.5f;

	return o;
}
float4 PrimitivePS(Output o):SV_Target
{
	/*float h = _heightMap.Sample(_samplerState, o.uv);
	float2 heightUV = o.uv + 0.03*h*o.eyeVec.xy;*/

	float4 diffuse = float4(1.0, 1.0, 1.0, 1.0);
	float4 ambient = float4(0.4, 0.4, 0.4, 1);
	float bright = saturate(dot(o.lightVec, o.normal));
	//�e�N�X�`������F�擾
	float4 texColor = _shadowTex.Sample(_samplerState_clamp,o.uv);
	//�@���e�N�X�`������F���擾
	float3 normalColor = _normalTex.Sample(_samplerState, o.uv);
		//�x�N�g���֕ϊ��@���Ŏ擾�������_�ł͔͈͂�0~1�Ȃ̂�-1~1�ɂȂ�悤��
		float3 normalVec = 2 * normalColor - 1.0f;
		normalVec = normalize(normalVec);

	bright = saturate(dot(o.lightVec, o.normal));//saturate(dot(-o.lightVec, normalVec));

	float2 shadowUV = (float2(1, 1) + (o.shadowpos.xy / o.shadowpos.w)*float2(1, -1))*0.5f;
		float lightviewDepth = _shadowTex.Sample(_samplerState_clamp, shadowUV).r;

	float ld = o.shadowpos.z / o.shadowpos.w;
	float2 satUV = saturate(shadowUV);
	float shadowWeight = 1.0f;
	if (shadowUV.x==satUV.x&&shadowUV.y==satUV.y&&ld > lightviewDepth+0.001f){
		shadowWeight = 0.1f;
	}

	//SSS�̃e�X�g�@��ŋ��̃��f���ɂ��Ď���
	float ed = o.postest.z / o.postest.w;//���_����̐[�x
	float thickness = abs(ld - lightviewDepth)*200.0f;
	float4 transparent = float4(diffuse.rgb*(1.0f - saturate(thickness / 0.2f)), 1);
		float sss = 1.0f - saturate(thickness / 2.4f);
	//return transparent;
	//SSS

	bright = min(bright, shadowWeight);
	//return float4(bright, bright, bright, 1);
	//return _shadowTex.Sample(_samplerState_clamp, o.uv);
	return float4((ambient.rgb+bright)/**texColor.rgb*/,1);//ambient+bright;
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

textureCUBE cubeTex;
samplerCUBE cubeSampler;

struct HUDOut
{
	float4 pos:SV_POSITION;
	float2 uv:TEXCOORD0;
};

//HUD�V�F�[�_
//@param vs 
//@param layout
//@param ps
//@param result �����Ȃ�S_OK 
HUDOut HUDVS(float4 pos:POSITION,float2 uv : TEXCOORD)
{
	HUDOut o;
	o.pos = mul(_world,pos);
	o.uv = uv;
	//o.pos = pos;
	return o;
}
//�e�̃e�N�X�`������ɕ\������
float4 HUDPS(HUDOut o):SV_Target
{
	//return (0,0,0,0);
	float r = _shadowTex.Sample(_samplerState_clamp, o.uv);
	//return r;
	r = pow(r, 50);
	return float4(r,0,0,1);
}

HUDOut BillBoardVS(float4 pos:POSITION, float2 uv:TEXCOORD)
{
	HUDOut o;
	matrix _camera = mul(_cameraProj, _cameraView);
	matrix wvp = mul(_camera, _world);
	o.pos = mul(wvp, pos);
	o.uv = uv;
	return o;
}
float4 BillBoardPS(HUDOut o):SV_Target
{
	float4 col = _tex.Sample(_samplerState, o.uv);
	return col;
	return float4(0, 0, 1, 1);
}




technique Render
{
	pass p0{
		VertexShader = compile vs_5_0 BaseVS();
		PixelShader = compile ps_5_0 BasePS();
	}
	pass p1{
		VertexShader = compile vs_5_0 BoneVS();
		PixelShader = compile ps_5_0 BonePS();
	}
};