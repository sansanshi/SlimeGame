#pragma once
#include<D3D11.h>
#include<xnamath.h>
#include<vector>

#pragma pack(1)
struct PMXMaterial{
	XMFLOAT4 diffuse;
	XMFLOAT3 specular;
	float specularity;
	XMFLOAT3 ambient;

	//�r�b�g�t���O 8bit�@�ebit 0:OFF 1:ON
	//0x01:���ʕ`��, 0x02:�n�ʉe, 0x04:�Z���t�V���h�E�}�b�v�ւ̕`��, 
	//0x08:�Z���t�V���h�E�̕`��, 0x10:�G�b�W�`��
	//�����l�l�c�APMX�G�f�B�^���ŕ\�����鎞�̐ݒ�Ȃ̂Ŏ��O�ŕ`�悷��Ȃ�v��Ȃ�
	BYTE bitFlag;

	XMFLOAT4 edgeColor;
	float edgeScale;

	unsigned int offset;
	unsigned int indicesCnt;

	unsigned int textureIdx;
	unsigned int sphereTexIdx;
	unsigned int toonIdx;
};
#pragma pack()

class PMXLoader;

class PMXMesh
{
	friend PMXLoader;
private:
	//���Ltoon�e�N�X�`���@toon01~toon10�ɑΉ��@���Ltoon�t���O��1�̎��Ɏg���@���̓��[�h���ĂȂ�
	std::vector<ID3D11ShaderResourceView*> _toonTextures;
	std::vector<ID3D11ShaderResourceView*> _textures;
	std::vector<PMXMaterial> _materials;

	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* _indexBuffer;


public:
	PMXMesh();
	~PMXMesh();
};

