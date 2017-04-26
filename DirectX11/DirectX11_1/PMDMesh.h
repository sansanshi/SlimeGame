#pragma once
#include<D3D11.h>
#include<xnamath.h>
#include<vector>
#include"Define.h"
#include<map>

class PMDLoader;

//�}�e���A�����̓ǂݍ���
#pragma pack(1)
struct MATERIAL {
	XMFLOAT4 diffuse;//�f�B�t���[�Y�J���[����
	float specularity;//�X�y�L�������x
	XMFLOAT3 specularColor;//�X�y�L�����J���[����
	XMFLOAT3 ambient;//��������
	BYTE toonIdx;//�g�[���}�b�s���O�C���f�b�N�X
	BYTE edgeFlag;//�֊s���t���O�@�����ŃA���C�����g�ɋC��t����
	unsigned int indicesNum;//�ΏۃC���f�b�N�X��
	char textureName[20];//�e�N�X�`���t�@�C����

};
#pragma pack()

// 01/24�ɐV���������
struct Bone{
	std::string name;//���ʂȂ悤�����ǁu�Ђ��v���������邽��
	XMFLOAT3 headpos;
	XMFLOAT3 tailpos;

};


#pragma pack(1)
struct PMDMaterial {
	XMFLOAT3 diffuse;//�f�B�t���[�Y�J���[����
	float alpha;
	float specularity;//�X�y�L�������x
	XMFLOAT3 specularColor;//�X�y�L�����J���[����
	XMFLOAT3 ambient;//��������
	unsigned int offset;
	unsigned int indicesNum;//�ΏۃC���f�b�N�X��
	ID3D11ShaderResourceView* texture;
	ID3D11ShaderResourceView* spa;
	ID3D11ShaderResourceView* sph;

};
#pragma pack()

#pragma pack(1)
struct PMDVertForBuff//�v62
{
	XMFLOAT3 pos;//���_���W 12�o�C�g
	XMFLOAT3 normal;//�@���x�N�g���@12�o�C�g
	Vector2 uv;//8�o�C�g
	WORD boneId[2];//unsigned short * 2 = 4�o�C�g
	BYTE boneWeight;//1�o�C�g
	BYTE edgeFlag;//1�o�C�g
	Vector3 binormal;//12
	Vector3 tangent;//12
	//XMMATRIX uvzMatrix;//4*4*4 64�o�C�g ����ϗv��Ȃ�
};
#pragma pack()

//�{�[���\���́i�ǂݍ���
#pragma pack(1)
struct BONE{
	char boneName[20];
	unsigned short parentBoneIdx;
	unsigned short tailPosBoneIdx;
	BYTE boneType;
	unsigned short ikParentBoneIdx;
	XMFLOAT3 bonePos;
};
#pragma pack()

struct BoneVertForBuff
{
	XMFLOAT3 pos;
	unsigned int boneId;
};

#pragma pack(1)
struct BoneVert
{
	std::string name;//���ʂȂ悤�����ǁu�Ђ��v���������邽��
	BoneVertForBuff head;
	BoneVertForBuff tail;
};
#pragma pack()

class PMDMesh
{
	friend PMDLoader;

public:
	struct IKList{
		std::vector<int> ikboneIndices;
		int ikchainLen;
		int iterations;
		//float controlWeight;//���Ɠ���
		float limitangle;
	};

private:
	std::string _folderPath;

	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* _indexBuffer;
	ID3D11Buffer* _boneVertBuffer;

	ID3D11Buffer* _boneMatrixBuff;

	unsigned int _vertexSize;
	unsigned int _indexSize;
	unsigned int _materialSize;
	unsigned short _boneSize;
	const unsigned int _vertexStride;
	const unsigned int _boneVertStride;

	std::vector<PMDMaterial> _materials;
	std::vector<ID3D11ShaderResourceView*> _textures;
	std::vector<BONE> _bones;
	std::vector<BoneVert> _boneVerts;

	std::map<std::string, int> _boneMap;

	std::map<std::string, IKList> _ikMap;

	std::vector<XMMATRIX> _boneMatrixies;

public:
	PMDMesh();
	~PMDMesh();


	

	ID3D11Buffer* GetVertexBuffer(){ return _vertexBuffer; };
	ID3D11Buffer* GetIndexBuffer(){ return _indexBuffer; };
	ID3D11Buffer* GetBoneVertBuffer(){ return _boneVertBuffer; };

	ID3D11Buffer* GetBoneMatrixBuffer(){ return _boneMatrixBuff; };

	unsigned int GetVertexStride()const{ return _vertexStride; };
	unsigned int GetBoneVertStride()const{ return _boneVertStride; };

	unsigned int VertexSize()const { return _vertexSize; };
	unsigned int VertexStride()const{ return _vertexStride; };
	unsigned int IndexSize()const{ return _indexSize; };
	unsigned int MaterialSize()const{ return _materialSize; };
	unsigned short BoneSize()const { return _boneSize; };

	std::vector<PMDMaterial>& GetMaterials(){ return _materials; };
	std::vector<ID3D11ShaderResourceView*>& GetTextures(){ return _textures; };
	std::vector<BONE>& GetBones(){ return _bones; };
	std::vector<BoneVert>& GetBoneVertices(){ return _boneVerts; };
	std::map<std::string, int>& GetBoneMap(){ return _boneMap; };
	std::vector<XMMATRIX>& BoneMatrixies(){ return _boneMatrixies; };

	std::string FolderPath(){ return _folderPath; };

	void Draw();

	std::map<std::string, IKList>& IKListMap(){return _ikMap;}
		
	
};

