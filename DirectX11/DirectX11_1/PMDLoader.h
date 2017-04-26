#pragma once
//PMDMesh�I�u�W�F�N�g�𐶐����A�N���C�A���g�ɒ񋟂���
#include<D3D11.h>
#include<string>
#include<map>
#include"PMDMesh.h"
#include"Define.h"

struct PMDHeader{
	//signature��3�o�C�g���R���s���[�^��4�o�C�g���ǂݍ���
	//���R���p�C�������������l�߂ď������悤�Ƃ���1�o�C�g���Y���Ă��܂�
	//char signature[3];//�V�O�l�`���@�ǂ������t�@�C���Ȃ̂�
	float version;//�o�[�W����
	char name[20];//���f���̖��O
	char comment[256];//�R�����g
	//unsigned int vertexCount;//���_�� 
};

struct TempVertex
{
	Vector3 pos;
	Vector2 uv;
	Vector3 normal;
};

class PMDLoader
{
private:
	std::map<std::string, PMDMesh*> _map;
	PMDMesh* LoadPMD(std::string filepath);

	ID3D11ShaderResourceView* _nulltexture;
	ID3D11ShaderResourceView* _nulltextureb;

	//�ǂݍ��ނƂ���mesh�̎���materials[i]��n��
	//void LoadTexture(std::string filename,std::string foldername,PMDMesh::Material& material);

	PMDHeader _header;
	char signature[3];//�������������ēǂݍ���

	void CalculateTangentBinormal(TempVertex* v0, TempVertex* v1, TempVertex* v2,
		std::vector<PMDVertForBuff>& verts,const std::vector<unsigned short>& indices,int idx);

	void CalculateTangent(TempVertex * v0, TempVertex* v1, TempVertex* v2,
		std::vector<PMDVertForBuff>& verts, const std::vector<unsigned short>& indices, int idx);
	
public:
	PMDLoader();
	~PMDLoader();
		
	//�t�@�C���p�X����Pmdmesh�𐶐����ĕԂ�
	//�߂�l�FPMDMesh�I�u�W�F�N�g�ւ̃|�C���^
	//�t�@�C�����Ȃ����nullptr��Ԃ�
	PMDMesh* Load(std::string filePath);

	void LoadTexture(std::string, std::string, PMDMaterial&);

};

