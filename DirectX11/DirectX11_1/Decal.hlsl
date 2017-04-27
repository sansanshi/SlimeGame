cbuffer global:register(b0){
	matrix _world;
	matrix _view;//�J�����r���[
	matrix _proj;//�J�����v���W�F�N�V����
	matrix _invWorld;
	matrix _invView;
	matrix _invProj;

	matrix _invWVP;
	matrix _wvp;

};
cbuffer Global2:register(b5){
	float4 lightPos;
	float4 eyePos;
	int timer;

};

//�s��̐������o������D�揇�ɂȂ��Ă�
//3�s��4��ځ@�Ȃ�._43 ._m32
//4�s��3��ځ@�Ȃ�._34 ._m23 �Ŏ��o��
struct Output{
	float4 pos:SV_POSITION;
	float4 postest:POSITION;
	float4 viewPos:POSITION2;
	float2 texcoord:TEXCOORD;
	float3 posWorld:POSITION1;
	float4x4 invWorld:MATRIX0;
	float4x4 invView:MATRIX4;
	float4x4 invWVP:MATRIX8;

	float4x4 proj:MATRIX12;
	float4x4 invProj:MATRIX16;

	float4 posi:TEXCOORD1;

	float4 color:COLOR;

};

matrix MatrixIdentity()
{
	matrix ret = {
		float4(1, 0, 0, 0),
		float4(0, 1, 0, 0),
		float4(0, 0, 1, 0),
		float4(0, 0, 0, 1)
	};
	return ret;
}

Texture2D _tex:register(t0);
Texture2D _sph:register(t1);
Texture2D _spa:register(t2);

Texture2D _normalTex:register(t5);
Texture2D _heightMap:register(t6);
Texture2D _dispMap:register(t7);
Texture2D _dispMask:register(t8);
Texture2D _decalTex:register(t9);//�f�J�[��
Texture2D _shadowTex:register(t10);//���C�g����̃����_�����O���ʂ��e�N�X�`���Ƃ��Ď󂯎��
Texture2D _lightViewTex:register(t11);
Texture2D _cameraDepthTex:register(t12);

SamplerState _samplerState:register(s0);
SamplerState _samplerStateDisp:register(s1);
SamplerState _samplerState_clamp:register(s2);


matrix DisableTranslation(matrix mat)
{
	matrix m = mat;
	m._m03 = m._m13 = m._m23 = 0;
	return m;
}

matrix InverseTranslation(matrix mat)
{
	matrix m = mat;
	m._m03 = -m._m03;
	m._m13 = -m._m13;
	m._m23 = -m._m23;
	return m;
}

Output DecalBoxVS(float4 pos:POSITION)
{
	Output o;
	matrix wvp = mul(_proj, mul(_view, _world));
	o.pos = mul(wvp,pos);

	o.postest = mul(_world, pos);
	o.postest = mul(_view, o.postest);
	o.postest = mul(_proj, o.postest);

	o.texcoord = (float2(1, 1) + (o.postest.xy / o.postest.w)*float2(1, -1))*0.5f;

	float sampleDepth = _cameraDepthTex.SampleLevel(_samplerState, o.texcoord, 0).r;

	float3 positionVS;//ViewSpace Position
	matrix wv = mul(_view, _world);

	positionVS = mul(wv, pos);
	o.viewPos = mul(wv, pos);
	//positionVS = mul(float4(positionVS, 1), _invView).xyz;

	

	o.pos = mul(_world, pos);
	o.pos = mul(_view, o.pos);
	o.viewPos = o.pos;
	o.pos = mul(_proj, (float4x1)o.pos);

	//o.pos = mul(_wvp, pos);

	//directX���E��n�Ȃ̂�positionVS.z�Ƀ}�C�i�X�͗v��Ȃ��H
	float3 viewRay = float3(positionVS.xy, 100.0f);//positionVS.xyz*(100.0f / positionVS.z);//(-positionVS.z / 100.0f);//

		float3 viewPosition = float3(viewRay.xy,viewRay.z*sampleDepth);
		//o.pos = mul( _proj,float4(viewPosition, 1));

		float3 wpos = mul(float4(viewPosition, 1), _invView).xyz;
		o.posWorld = wpos;
	o.posWorld = mul(_world, pos);

	wpos = mul(float4(positionVS, 1), _invView).xyz;
	float3 objpos = mul(float4(wpos, 1), _invWorld).xyz;
		o.posWorld = wpos;
	//float3 objpos = mul(float4(wpos, 1), _invWorld).xyz;

	//o.pos = mul( wvp,float4(objpos,1));

	/*
	float3 objPos = mul(float4(wpos, 1), _invWorld).xyz;
		o.pos = mul(wvp,float4(objPos, 1));*/

	//o.pos = mul(_proj, float4(viewPosition,1));
	//o.pos = wpos;
	o.invWorld = InverseTranslation(_world);
	o.invView = _invView;
	o.invWVP = _invWVP;
	o.proj = _proj;
	o.invProj = _invProj;

	matrix invTest = mul(_invWorld,_world);


	float4 vpos = mul(_invProj, o.postest);
		float4 posw = mul(_invView, vpos);
		float4 lpos = mul(_invWorld, posw);
		//lpos = mul(_invWVP, o.pos);
		if (_invWorld._14 == -5.0f)
		{
			o.color = float4(0, 1, 1, 1);
		}

	return o;
}

float4 DecalBoxPS(Output o):SV_Target
{
	float4 col;

	float2 screenpos = o.postest.xy / o.postest.w;

	float2 coord = float2(
	(1.0f + screenpos.x) / 2.0f + (0.5f / 640.0f),
	(1.0f - screenpos.y) / 2.0f + (0.5f / 480.0f));
	//								�����s�N�Z�����炷
	//�s�N�Z���͎��ۂɂ̓h�b�g�i�_�j�ł���A�s�N�Z���i�����`�j�̒��S�ɂ���
	//(1.0 + screenpos.x)/2.0f,(1.0f - screenpos.y)/2.0f
	//���ŏo�Ă���̂̓s�N�Z���̍���̍��W�ł���A
	//�𑜓x����ɔ��s�N�Z�����炷�Ɛ��m�ȃe�N�Z�������߂���
	//�e�N�Z���@�F�e�N�X�`���G�������g�@�e�N�X�`�����\�������f
	//���s�N�Z�����炳�Ȃ��ꍇ���ۂɕ`�悷��ƃe�N�X�`�����኱�c��


	float d = _cameraDepthTex.Sample(_samplerState, coord);

	
	float2 Deproject = float2(o.proj._m00, o.proj._m11);
		float dep = _cameraDepthTex.Sample(_samplerState,coord);
	float4 scenePosView = float4(o.postest.xy*d / (Deproject.xy*o.postest.w),
	dep, 1);

	//o.invView = DisableTranslation(o.invView);
	float4 scenePosWorld = mul(o.invView, scenePosView);
		float4 scenePosLocal = mul(o.invWorld,scenePosWorld);

	//matrix invWV = mul(o.invWorld, o.invView);
	//scenePosLocal = mul(invWV,scenePosView);
	//clip(4.0f - scenePosLocal.xyz);

		

	//float3 positionVS = mul(o.invProj, o.postest);//o.viewPos;
	//	float3 viewRay = float3(positionVS.xy*(100.0f / positionVS.z),100.0f);
	//	//viewRay = normalize(positionVS.xyz)*100.0f;

	//	float3 viewPosition = viewRay*d;
	//	float3 positionW = mul(o.invView,float4(viewPosition, 1)).xyz;
	//	float4 positionL = mul(o.invWorld,float4(positionW, 1));

	float4 positionVS = mul(o.invProj, o.postest);//o.viewPos;
	//positionVS = float4(positionVS.xyz / positionVS.w, 1.0f);
		
		float3 viewRay = float3(positionVS.xy*(100.0f / positionVS.z),100.0f);
		//viewRay = normalize(positionVS.xyz)*100.0f;

		float3 viewPosition = viewRay*d;
		float3 positionW = mul(o.invView,float4(viewPosition, 1)).xyz;
		float4 positionL = mul(o.invWorld,float4(positionW, 1));

		//clip(8.0f - abs(positionL.xyz));




	float depth = _cameraDepthTex.Sample(_samplerState, coord);
	//x=0~1 y=0~1�̃e�N�X�`�����W�i�H�j����
	//x=-1~1 y=-1~1�̃N���b�s���O��Ԃ̍��W�ɂ���
	//z�̒l�̓T���v�����O�����[�x�l���g��
	//�[�x�l�̓J�������W����FarPlane�܂ł̋�����1.0�Ƃ��Đ��K����������
	//���K�����ꂽ���W���o�Ă���
	float4 screenPos = float4(coord*2.0f - 1.0f, depth, 1.0f);

		//WVP�s��̋t�s��������Ă����ăI�u�W�F�N�g��Ԃ̍��W�ɂ���
		//float4 localPos = mul(o.invWVP, screenPos);
		float4 localPos = mul(o.invProj, screenPos);
	//���ˉe�s��̋t�s�񂩂����w�̒l��
	//�N���b�s���O��ԂŐ��K�����ꂽ���l��0.0�Ȃ�1.0�A����1.0�Ȃ�0.009999(0.01)�ɂȂ�
	//������̊g�嗦�i���_��WVP���������̂��j�̋t���H
	//���̂���
	//�u�N���b�s���O��ԂŐ��K�����ꂽ���W�ɑ΂��ˉe�s��̋t�s����|���ďo�Ă������������W�v
	//������Ɗg�嗦���|�������ɂȂ�
	//			���r���[��Ԃł̍��W�����߂���
	localPos = float4(localPos.xyz / localPos.w, 1);
	localPos = mul(o.invView, localPos);
	localPos = mul(o.invWorld, localPos);
	//localPos = float4(localPos.xyz / localPos.w, localPos.w);

	//clip(8.0f-abs(localPos.xz));



	float4 posv = mul(o.invProj, screenPos);
		posv = float4(posv.xyz / posv.w, 1);
	float4	posw = mul(o.invView, posv);
	//posw = float4((posw.xyz / posw.w), posw.w);
	float4 posl = mul(o.invWorld, posw);
		//clip(4.0f - abs(posl.xz));


		//col = float4(0, 0, 0, 1);

	float2 uv = localPos.xz / float2(16.0f, -16.0f) + 0.5f;
	col = _decalTex.Sample(_samplerState, uv);//_decalTex.Sample(_samplerState, uv);
	if (localPos.x > 0.0f)
	{
		col.g = 1.0f;
	}

	//return float4(1, 0, 0, 0.7f);
	if (uv.x<1.0f&&uv.x>0.0f
		&&uv.y<1.0f&&uv.y>0.0f)
	{
		col.b = 1.0f;
	}
		return col;

	return float4(0, 0, 0, 1);
	//return (1, 1, 1, 1);
}

Output DecalPlaneVS(float4 pos:POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD)
{
	Output o;
	matrix wvp = mul(_proj, mul(_view, _world));
	o.pos = mul( wvp,pos);
	o.postest = o.pos;

	o.invWorld = _invWorld;
	o.invView = _invView;
	o.invProj = _invProj;

	o.texcoord = uv;

	return o;
}

float4 DecalPlanePS(Output o):SV_Target
{
	float2 screenpos = o.postest.xy / o.postest.w;

	float2 coord = float2(
		(1.0f + screenpos.x) / 2.0f + (0.5f / 640.0f),
		(1.0f - screenpos.y) / 2.0f + (0.5f / 480.0f));

	float depth = _cameraDepthTex.Sample(_samplerState, coord).r;

	float4 clipPos = float4(coord*2.0f - 1.0f, depth, 1.0f);

	float4 localPos = mul(o.invProj, clipPos);

	localPos = float4(localPos.xyz / localPos.w, 1);
	localPos = mul(o.invView, localPos);
	localPos = mul(o.invWorld, localPos);

	float2 uv = localPos.xz / float2(4.0f, 4.0f) + 0.5f;

	float4 col = _decalTex.Sample(_samplerState, uv);//_decalTex.Sample(_samplerState, o.texcoord);
	//col = lerp(col,float4(0.1f,0.1f,0.1f,1.0f), depth);
	//col.r = pow(col.r, 10);



	return col;
}