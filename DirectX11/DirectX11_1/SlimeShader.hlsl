
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
	float nearZ;
	float farZ;

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

	float timer : TIMER;

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
Texture2D _shadowTex:register(t10);//���C�g����̃����_�����O���ʂ��e�N�X�`���Ƃ��Ď󂯎��
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


Output SlimeVS(float4 pos:POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD,
	float4 tangent : TANGENT, float4 binormal : BINORMAL)
{
	Output o;
	matrix world_noTr = DisableTranslation(_world);
	o.noTransMatrix = world_noTr;
	o.normal = normalize(mul(world_noTr, normal));
	o.binormal = normalize(mul(world_noTr, binormal));
	o.tangent = normalize(mul(world_noTr, tangent));

	o.tangentMatrix = TangentMatrix(o.tangent, o.binormal, o.normal);

	//dispMap�͂������傢��G�c�Ȕg�ɂ��Ă݂�
	float2 uvOffset = float2((float)timer*0.002f, (float)timer*0.001f);
		float displaysment = _dispMap.SampleLevel(_samplerStateDisp, uv + uvOffset, 0).r;//_dispMap.Sample(_samplerState, uv);
	//����������0�A����������1�������Ă���
	displaysment = displaysment * 2 - 1.0f;//�e�N�X�`�����������l��-1.0�`1.0�͈̔͂�


	//�K�p���H�I�Ȓl
	uvOffset = float2(/*sin((float)timer*0.0005f)*/0, (float)timer*0.002f);
	float dispMask = _dispMask.SampleLevel(_samplerStateDisp, uv + uvOffset, 0).r;
	dispMask = dispMask*2.0f;
	//dispMask *= 2.0f;

	//�^�C�}�[���̂܂܂��Ƒ�������̂œK���ɏ���������
	float f = 1;// (sin((float)timer*0.0005f) + 1.0f) / 2;//�^�C�}�[��0.0�`1.0�͈̔͂�
	f *= dispMask;//2.0f;//0.0�`0.3�͈̔͂�
	displaysment *= f;

	float4 normalTemp = float4(normal.xyz * displaysment, 1.0);
	float t = cos(timer*(1.0f/60.0f)*6.0f);
	t = t*0.4f;
	/*if (t < -0.2f)
	{
		t = t*0.5f;
	}*/
	normalTemp.xyz = normalTemp.xyz + normal.xyz*t;

		//normalTemp.w = 1.0f;
		float4 posTemp = pos;//posTemp = pos + normalTemp;�����������x,y,z,w��[w]�̗v�f�܂ŉ��Z����Ă��܂�
		posTemp.xyz += normalTemp.xyz;//posTemp=float4(pos.xyz+normalTemp.xyz, 1.0)�ł���

		matrix _cameraVP = mul(_cameraProj, _cameraView);
	matrix tmp = mul(_cameraVP, _world);

	o.pos = mul(tmp, posTemp);//lightVec�̌ォ��Y���Ă���ۂ��H
	//�m�[�}���}�b�v���X�N���[�������邽�߂ɓn��uv�����炷
	//�s�N�Z���V�F�[�_�Ƀ^�C�}�n���Čv�Z������኱�v�Z�ʏ��Ȃ��Ȃ�H
	uvOffset = float2((float)timer*0.00008f, (float)timer*0.00002f);
	o.uv = uv;
	o.diffuse.rgb = _diffuse;
	o.diffuse.a = _alpha;
	o.specular = _specularColor*_specularity;
	o.ambient = _ambient;

	float4 posWorld = mul(_world, posTemp);
		o.lightVec = float4(normalize(posWorld.xyz - lightPos.xyz), 0);//float4(normalize(lightVec), 1.0);
	matrix invTang = InvTangentMatrix(o.tangent, o.binormal, o.normal);
	o.lightVec = normalize(mul(invTang, o.lightVec));//���C�g�x�N�g����ڋ�Ԃɕϊ�����

	//eyeVec=���_���W-���_
	o.eyeVec = float4(normalize(posWorld-eyePos));
	o.eyeVec = normalize(o.eyeVec);
	//o.eyeVec = normalize(mul(invTang, o.eyeVec));

	o.postest = mul(tmp, posTemp);
	matrix _lightVP = mul(_lightProj, _lightView);
	matrix lightview = mul(_lightVP, _world);//_lightView
	o.shadowpos = mul(lightview, posTemp);

	o.timer = timer;

	return o;
}

float4 SlimePS(Output o) :SV_Target
{
	float2 uvOffset = float2((float)o.timer*0.00002f, 0);
	float h = _heightMap.Sample(_samplerState, o.uv+uvOffset).r;//�����}�b�s���O�p�e�N�X�`������ǂݎ����
	float2 heightUV = (o.uv + uvOffset) - 0.03*h*-o.eyeVec.xy;

		//�@���e�N�X�`������F���擾
		uvOffset.y = -(float)o.timer*0.002f;
	float3 normalColor = _normalTex.Sample(_samplerState, o.uv+uvOffset).rgb;
	//�x�N�g���֕ϊ��@���Ŏ擾�������_�ł͔͈͂�0~1�Ȃ̂�-1~1�ɂȂ�悤��
	float3 normalVec = 2 * normalColor - 1.0f;
	normalVec = normalize(normalVec);

	//float3 n_local = mul(normalVec,o.tangentMatrix);//���_�V�F�[�_�Ń��C�g�x�N�g����ڋ�Ԃɕϊ��������炱�����͑��������v��Ȃ�
	//n_local = normalize(mul(o.noTransMatrix, n_local));

	//float3 ref = reflect(-o.eyeVec, n_local);

	//saturate��0�`1�ɃN�����v����֐�
	//�@���@�����x�N�g���́u�t�v�x�N�g���Ƃ̓��ς����
	float bright = saturate(dot(-o.lightVec, normalVec/*n_local*/));
	//return float4(bright, bright, bright, 1);

	float2 shadowUV = (float2(1, 1) + (o.shadowpos.xy / o.shadowpos.w)*float2(1, -1))*0.5f;
		float lightviewDepth = _shadowTex.Sample(_samplerState_clamp, shadowUV).r;

	float ld = o.shadowpos.z / o.shadowpos.w;
	float shadowWeight = 1.0f;
	if (ld > lightviewDepth + 0.01f){
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

	bright = min(bright, shadowWeight);
	//return float4(bright, bright, bright, 1);
	float4 col = _tex.Sample(_samplerState, heightUV);
		col = float4(0.6, 0.6, 0.4, 1);
	col.r *= cos((float)o.timer*0.001f*o.uv.x*bright);
	col.g *= cos((float)o.timer*0.006f*o.uv.y*(1-bright));
	col.b *= cos((float)o.timer*(o.uv.x+o.uv.y)*0.002f);
	float3 sphCol = _sph.Sample(_samplerState, o.normal.xy / 2 * float2(1, -1) + float2(0.5f, 0.5f));
		return float4((bright*o.diffuse.rgb + o.ambient.rgb)*col.rgb
		/*+ 0.3f*pow(max(0, dot(ref, o.lightVec)), 8)*/, o.diffuse.a*(bright+0.6));//VSSetConstantBuffer�œn���Ă����f�[�^�͒��ڃs�N�Z���V�F�[�_�ł͎g���Ȃ����ۂ��@�S���̒l��0�ɂȂ��Ă���
	col.a = o.diffuse.a;
}