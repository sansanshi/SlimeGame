#include"ShaderInclude.hlsli"

cbuffer global:register(b0){
	matrix _world[300];
	matrix _view;//�J�����r���[
	matrix _proj;//�J�����v���W�F�N�V����
	matrix _invWorld[300];
	matrix _invView;
	matrix _invProj;


};
cbuffer global_debug:register(b1)
{
	matrix __world;
	matrix __view;//�J�����r���[
	matrix __proj;//�J�����v���W�F�N�V����
	matrix __invWorld;
	matrix __invView;
	matrix __invProj;
}


//�s��̐������o������D�揇�ɂȂ��Ă���
//3�s��4��ځ@�Ȃ�._43 ._m32
//4�s��3��ځ@�Ȃ�._34 ._m23 �Ŏ��o��
struct Output{
	float4 pos:SV_POSITION;
	float4 postest:POSITION;
	float2 texcoord:TEXCOORD;
	float4x4 invWorld:MATRIX0;
	float4x4 invView:MATRIX4;

	float4x4 invProj:MATRIX8;

	float nearZ : NEAR;
	float farZ : FAR;

	float4 modelpos:MODELPOS;

	float fog:TEXCOORD1;
	float4 fogColor:COLOR1;
	float2 windowSize:TEXCOORD2;


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
Output DecalBoxVS_Debug(float4 pos:POSITION)
{
	Output o;
	matrix wvp = mul(_proj, mul(_view, __world));
	o.pos = mul(wvp, pos);
	o.postest = o.pos;//SV_POSITION��ݒ肷��ƃ��X�^���C�U��w�Ŋ����Ă����̂ł����ok



	o.invWorld = __invWorld;//InverseTranslation(_world);
	o.invView = __invView;
	o.invProj = __invProj;



	o.nearZ = nearZ;
	o.farZ = farZ;
	o.modelpos = pos;

	float dist = length(mul(mul(__view, __world), pos));
	o.fogColor = fogColor;
	o.fog = fogCoord.x + dist*fogCoord.y;

	o.windowSize = windowSize;
	

	return o;
}




Output DecalBoxVS(float4 pos:POSITION,uint instNum:SV_InstanceID)
{
	Output o;
	matrix wvp = mul(_proj, mul(_view, _world[instNum]));
	o.pos = mul(wvp,pos);
	o.postest = o.pos;//SV_POSITION��ݒ肷��ƃ��X�^���C�U��w�Ŋ����Ă����̂ł����ok



	o.invWorld = _invWorld[instNum];//InverseTranslation(_world);
	o.invView = _invView;
	o.invProj = _invProj;



	o.nearZ = nearZ;
	o.farZ = farZ;
	o.modelpos = pos;

	float dist = length(mul(mul(_view, _world[instNum]), pos));
	o.fogColor = fogColor;
	o.fog = fogCoord.x + dist*fogCoord.y;

	o.windowSize = windowSize;

	return o;
}

float4 DecalBoxPS_Debug(Output o):SV_Target
{
	float4 col;

	float2 screenpos = o.postest.xy / o.postest.w;

	float2 coord = float2(
	(1.0f + screenpos.x) / 2.0f + (0.5f / o.windowSize.x),
	(1.0f - screenpos.y) / 2.0f + (0.5f / o.windowSize.y));
	//								�����s�N�Z�����炷
	//�s�N�Z���͎��ۂɂ̓h�b�g�i�_�j�ł���A�s�N�Z���i�����`�j�̒��S�ɂ���
	//(1.0 + screenpos.x)/2.0f,(1.0f - screenpos.y)/2.0f
	//���ŏo�Ă���̂̓s�N�Z���̍���̍��W�ł���A
	//�𑜓x����ɔ��s�N�Z�����炷�Ɛ��m�ȃe�N�Z�������߂���
	//�e�N�Z���@�F�e�N�X�`���G�������g�@�e�N�X�`�����\�������f
	//���s�N�Z�����炳�Ȃ��ꍇ���ۂɕ`�悷��ƃe�N�X�`�����኱�c��


	float d = _cameraDepthTex.Sample(_samplerState, coord);
		
		
	float4 positionVS = mul(o.invProj, o.postest);
	positionVS = float4(positionVS.xyz / positionVS.w, 1.0f);
	//���ˉe�s��̋t�s�񂩂����w�̒l��
	//�N���b�s���O��ԂŐ��K�����ꂽ���l��0.0�Ȃ�1.0�A����1.0�Ȃ�0.009999(0.01)�ɂȂ�
	//������̊g�嗦�i���_��WVP���������̂��j�̋t��
	//���̂���
	//�u�N���b�s���O��ԂŐ��K�����ꂽ���W�ɑ΂��ˉe�s��̋t�s����|���ďo�Ă������������W�v
	//������Ɗg�嗦���|�������ɂȂ�
	//			���r���[��Ԃł̍��W�����߂���
		
		float3 viewRay = float3(positionVS.xy*(o.farZ / positionVS.z),o.farZ);
		//viewRay = normalize(positionVS.xyz)*100.0f;

		float3 viewPosition = viewRay*d;
		
		float3 positionW = mul(o.invView,float4(viewPosition, 1));
		float3 positionL = mul(o.invWorld,float4(positionW, 1));


		float valueXY = abs(o.modelpos.x) +abs(o.modelpos.y);
		float valueYZ = abs(o.modelpos.y) + abs(o.modelpos.z);
		float valueXZ = abs(o.modelpos.x) + abs(o.modelpos.z);
		if (valueXY>0.98f||valueYZ>0.98f||valueXZ>0.98f)
		{
			return float4(1, 0, (o.modelpos.z+0.5f), 1);
		}
		clip(0.5f - abs(positionL.xyz));
		


	//�f�v�X�o�b�t�@����LinearizedDepth�����߂Ă݂�
	//float z = (2.0f*1.0f) / (100.0f*1.0f - depth*(100.0f - 1.0f));

	
	
	float2 uv = (positionL.xy + 0.5f)*float2(1.0f,-1.0f);
	if (abs(0.5f-uv.x)>0.49f||abs(0.5f+uv.y)>0.49f)
	{
		return float4(1, 0, 0, 1);
	}
	col = _decalTex.Sample(_samplerState, uv);//_decalTex.Sample(_samplerState, uv);
	
	//a=0�̕����̓t�H�O�̉e�����󂯂����Ȃ�
	col = float4(lerp(o.fogColor.rgb, col.rgb, o.fog), col.a);
	return col;
}

float4 DecalBoxPS(Output o) :SV_Target
{
	float4 col;

float2 screenpos = o.postest.xy / o.postest.w;

float2 coord = float2(
(1.0f + screenpos.x) / 2.0f + (0.5f / o.windowSize.x),
(1.0f - screenpos.y) / 2.0f + (0.5f / o.windowSize.y));
//								�����s�N�Z�����炷
//�s�N�Z���͎��ۂɂ̓h�b�g�i�_�j�ł���A�s�N�Z���i�����`�j�̒��S�ɂ���
//(1.0 + screenpos.x)/2.0f,(1.0f - screenpos.y)/2.0f
//���ŏo�Ă���̂̓s�N�Z���̍���̍��W�ł���A
//�𑜓x����ɔ��s�N�Z�����炷�Ɛ��m�ȃe�N�Z�������߂���
//�e�N�Z���@�F�e�N�X�`���G�������g�@�e�N�X�`�����\�������f
//���s�N�Z�����炳�Ȃ��ꍇ���ۂɕ`�悷��ƃe�N�X�`�����኱�c��


float d = _cameraDepthTex.Sample(_samplerState, coord);


float4 positionVS = mul(o.invProj, o.postest);
positionVS = float4(positionVS.xyz / positionVS.w, 1.0f);
//���ˉe�s��̋t�s�񂩂����w�̒l��
//�N���b�s���O��ԂŐ��K�����ꂽ���l��0.0�Ȃ�1.0�A����1.0�Ȃ�0.009999(0.01)�ɂȂ�
//������̊g�嗦�i���_��WVP���������̂��j�̋t��
//���̂���
//�u�N���b�s���O��ԂŐ��K�����ꂽ���W�ɑ΂��ˉe�s��̋t�s����|���ďo�Ă������������W�v
//������Ɗg�嗦���|�������ɂȂ�
//			���r���[��Ԃł̍��W�����߂���

float3 viewRay = float3(positionVS.xy*(o.farZ / positionVS.z),o.farZ);
//viewRay = normalize(positionVS.xyz)*100.0f;

float3 viewPosition = viewRay*d;
float3 positionW = mul(o.invView,float4(viewPosition, 1)).xyz;
float4 positionL = mul(o.invWorld,float4(positionW, 1));


clip(0.5f - abs(positionL.xyz));



//�f�v�X�o�b�t�@����LinearizedDepth�����߂Ă݂�
//float z = (2.0f*1.0f) / (100.0f*1.0f - depth*(100.0f - 1.0f));



float2 uv = (positionL.xy + 0.5f)*float2(1.0f,-1.0f);

col = _decalTex.Sample(_samplerState, uv);//_decalTex.Sample(_samplerState, uv);

//a=0�̕����̓t�H�O�̉e�����󂯂����Ȃ�
col = float4(lerp(o.fogColor.rgb, col.rgb, o.fog), col.a);
return col;
}