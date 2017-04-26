#pragma once
#include<xnamath.h>



XMFLOAT3 operator- (XMFLOAT3& lval, XMFLOAT3& rval);
XMFLOAT3 operator+ (XMFLOAT3& lval, XMFLOAT3& rval);
XMFLOAT3 operator* (XMFLOAT3& vec, float value);

float Length(XMFLOAT3& vec);

//����
float Dot(XMFLOAT3& lval, XMFLOAT3& rval);
//�O��
XMFLOAT3 Cross(XMFLOAT3& lval, XMFLOAT3& rval);
//���K��
XMFLOAT3 Normalize(XMFLOAT3& vec);

bool operator== (XMFLOAT3 lval, XMFLOAT3 rval);

XMFLOAT3 operator* (XMFLOAT3 lval, XMMATRIX& mat);

XMFLOAT3 operator/ (XMFLOAT3 lval, float div);