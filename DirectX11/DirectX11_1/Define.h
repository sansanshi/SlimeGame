#pragma once
#include<D3D11.h>
#include<xnamath.h>

extern const float WINDOW_WIDTH ;
extern const float WINDOW_HEIGHT;


extern const float NEAR_Z;
extern const float FAR_Z;

extern const float FOG_START;
extern const float FOG_END;
extern const XMFLOAT4 FOG_COLOR;

extern const float SKYSPHERE_RADIUS;

extern const float SPEED_RISING;//カメラの上昇、下降スピード

extern const XMFLOAT4 DIRECTIONAL_LIGHT_VECTOR;


#pragma pack(1)
struct HUDVertex {
	XMFLOAT3 pos;
	XMFLOAT2 uv;
	/*HUDVertex(XMFLOAT3 p, XMFLOAT2 texcoord) :pos(p), uv(texcoord){}*/
};
#pragma pack()

struct Vector3{
	float x, y, z;

	Vector3(){ x = y = z = 0.f; };
	Vector3(float x, float y, float z){ this->x = x; this->y = y; this->z = z; };
	Vector3& operator =(Vector3 v)
	{
		x = v.x; y = v.y; z = v.z;
		return (*this);
	}
	Vector3 operator -(Vector3 v)
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}
	Vector3 operator +(Vector3 v)
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}
	Vector3* operator ->()
	{
		return (this);
	}

	Vector3& operator =(XMFLOAT3& xmfloat3)
	{
		x = xmfloat3.x; y = xmfloat3.y; z = xmfloat3.z;
		return (*this);
	}

	float Length()
	{
		return sqrt(x*x + y*y + z*z);
	}
	Vector3 Normalize()
	{
		return Vector3(x / Length(), y / Length(), z / Length());
	}

	Vector3 operator *(float f)
	{
		return Vector3(x*f, y*f, z*f);
	}

	void Zero()
	{
		x = y = z = 0;
	}

	Vector3 Cross(const Vector3& vec)
	{
		Vector3 ret;
		//ret=Vector3(x*vec.y - y*vec.x, y*vec.z - z*vec.y, z*vec.x - x*vec.z);
		ret = Vector3(y*vec.z - z*vec.y, z*vec.x - z*vec.x, x*vec.y - y*vec.x);

		return ret;
	}

	Vector3 operator -()
	{
		return Vector3(-x, -y, -z);
	}

};

struct Vector2{
	float x, y;

	Vector2(){ x = y =  0.f; };
	Vector2(float x, float y){ this->x = x; this->y = y;  };
	Vector2& operator =(const Vector2& v)
	{
		x = v.x; y = v.y;
		return (*this);
	}
	Vector2 operator -(Vector2 v)
	{
		return Vector2(x - v.x, y - v.y);
	}
	Vector2 operator +(Vector2 v)
	{
		return Vector2(x + v.x, y + v.y);
	}

	Vector2* operator ->()
	{
		return (this);
	}

	Vector2 operator -()
	{
		return Vector2(-x, -y);
	}

	void Zero()
	{
		x = y = 0;
	}
	
};


//シェーダに渡す用の構造体
#pragma pack(1)
struct ShaderGlobals
{
	XMFLOAT4 lightPos;//16　ライト位置
	XMFLOAT4 eyePos;//16
	XMFLOAT4 directionalLightVec;//平行光源ベクトル
	XMFLOAT4 fogColor;
	XMFLOAT2 fog;//フォグ係数計算用：X成分にZfar/(Zfar-Znear)，Y成分に-1/(Zfar-Znear)
	XMFLOAT2 windowSize;
	float nearZ;
	float farZ;
	int timer;//4
	

	//XMFLOAT3 lightVec;//12
	//int timer;//4
	//XMFLOAT3 eyeVec;//12

	//XMFLOAT3 lightVec;//12
	//int dummy;//4
	//XMFLOAT3 eyeVec;//12
	//int timer;//4
};
#pragma pack()


struct MatrixiesForDecal{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX proj;

	XMMATRIX invWorld;
	XMMATRIX invView;
	XMMATRIX invProj;

	XMMATRIX invWVP;
	XMMATRIX wvp;
};
struct MatrixiesForDecalTest {
	XMMATRIX world[300];
	XMMATRIX view;
	XMMATRIX proj;

	XMMATRIX invWorld[300];
	XMMATRIX invView;
	XMMATRIX invProj;

};

//シェーダに渡す用の構造体
struct Material{
	XMFLOAT3 diffuse;
	float alpha;
	//ここにも色々追加する
	XMFLOAT3 specularColor;
	float specularity;
	XMFLOAT3 ambient;

};


struct MATRIXIES{
	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
};

struct WorldAndCamera{
	XMMATRIX world;
	XMMATRIX cameraView;
	XMMATRIX cameraProj;
	XMMATRIX lightView;
	XMMATRIX lightProj;
};