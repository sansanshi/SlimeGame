cbuffer global:register(b0){
	matrix _world;
	matrix _camera;
	matrix _lightview;

};

struct Material{
	float4 color:COLOR;
	float roughness : ROUGHNESS;
	float fresnelReflectance : FRESNEL;
};

struct VS_OUTPUT
{
	float4 pos:SV_POSITION;
	float2 uv:TEXCOORD;
	float3 normal:NORMAL;
	float3 lightVec:LIGHTVECTOR;
	float3 eyeVec:EYEVECTOR;

	float4 color:COLOR;
	float roughness : ROUGHNESS;
	float fresnel : FRESNEL;
};

float D_GGX(float3 H, float3 N,float R_)
{
	float NdotH = saturate(dot(H, N));
	float roughness = saturate(R_);
	float alpha = R_*R_;
	float alpha2 = alpha*alpha;
	float t = ((NdotH*NdotH)*(alpha2 - 1.0f) + 1.0f);
	float PI = 3.1415926535897;
	return alpha2 / (PI*t*t);
}
float Flesnel(float3 V, float3 H,float F_)
{
	float VdotH = saturate(dot(V, H));
	float F0 = saturate(F_);
	float F = pow(1.0f - VdotH, 5);
	F *= (1.0f - F0);
	F += F0;
	return F;

}
float G_CookTorrance(float3 L, float3 V, float3 H, float3 N)
{
	float NdotH = saturate(dot(N, H));
	float NdotL = saturate(dot(N, L));
	float NdotV = saturate(dot(N, V));
	float VdotH = saturate(dot(V, H));

	float NH2 = 2.0f*NdotH;
	float g1 = (NH2*NdotV) / VdotH;
	float g2 = (NH2*NdotL) / VdotH;
	float G = min(1.0f, min(g1, g2));
	return G;
}

VS_OUTPUT PBR_VS( float4 pos : POSITION,float3 normal:NORMAL,float2 uv:TEXCOORD,float3 tangent:TANGENT,float3 binormal:BINORMAL )
{
	VS_OUTPUT o;
	matrix wvp = mul(_camera, _world);
	o.pos = mul(wvp,pos);

	o.color = float4(1, 1, 1, 1);
	o.roughness = 0.5f;
	o.fresnel = 0.5f;

	return o;
}

float4 PBR_PS(VS_OUTPUT o):SV_Target
{
	//�����ƃ}�e���A���̐F�����Z �@�����̐F�Ƃ��l���ĂȂ��̂łƂ肠�����K����
	float3 ambientLight = float3(0.3f, 0.3f, 0.3f);


	//���[���h��ԏ�̃��C�g��Ɩ@���Ƃ̓���
	float NdotL = saturate(dot(o.normal, o.lightVec));
	//���[���h��ԏ�̎��_�ʒu�Ɩ@���Ƃ̓��ς��v�Z
	float NdotV = saturate(dot(o.normal, o.eyeVec));

	//���C�g�Ǝ��_�x�N�g���̃n�[�t�x�N�g�����v�Z
	float3 halfVec = normalize(o.lightVec + o.eyeVec);
	//D_GGX�̍�
	float D = D_GGX(halfVec, o.normal,o.roughness);

	//F�̍�
	float F = Flesnel(o.eyeVec, halfVec, o.fresnel);

	//G�̍�
	float G = G_CookTorrance(o.lightVec, o.eyeVec, halfVec, o.normal);

	//�X�y�L�����y�уf�B�t���[�Y���v�Z
	float specularReflection = (D*F*G) / (4.0f*NdotV*NdotL + 0.000001f);
	float3 diffuseReflection = o.color.xyz*NdotL;

		//�Ō�ɐF�����Z���ďo��
		float4 col = float4(ambientLight + diffuseReflection + specularReflection, 1.0f);

		return col;

	return float4(1, 1, 1, 1);
}