#include "Player.h"
#include<xnamath.h>
#include<algorithm>
#include<thread>
#include<string>

#include<cassert>
#include"Geometry.h"
#include"ShaderDefine.h"
#include"Camera.h"
#include"ResourceManager.h"



//���_���C�A�E�g
//�T�Ԗڂ̃p�����[�^�͐擪����̃o�C�g���Ȃ̂ŁCCOLOR�ɂ�POSITION��float�^4�o�C�g�~3���L�q
D3D11_INPUT_ELEMENT_DESC inputElementDescs[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//���V�F�[�_����16�r�b�g�����^�������̂�R16G16���Ə����ł��Ȃ��@
	//32�r�b�g�����^�œn���ăV�F�[�_���Ńr�b�g���Z����
	{ "BONE_ID", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "BONE_WEIGHT", 0, DXGI_FORMAT_R8_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "EDGE", 0, DXGI_FORMAT_R8_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	
};

D3D11_INPUT_ELEMENT_DESC boneInputElementDescs[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "BONE_ID", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

D3D11_INPUT_ELEMENT_DESC lightViewInputElementDescs[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//���V�F�[�_����16�r�b�g�����^�������̂�R16G16���Ə����ł��Ȃ��@
	//32�r�b�g�����^�œn���ăV�F�[�_���Ńr�b�g���Z����
	{ "BONE_ID", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "BONE_WEIGHT", 0, DXGI_FORMAT_R8_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "EDGE", 0, DXGI_FORMAT_R8_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

//std::fill(array.begin(),array.end(),XMMatrixIdentity());


//���_�C���f�b�N�X���C�A�E�g
D3D11_INPUT_ELEMENT_DESC indexInputElementDesc[]{
	{"INDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

//����̕�������������s���Ԃ��֐�
//	�iZ�x�N�g�������̕����Ɍ�������j
//@param lookat ���������������x�N�g��
//@param right �E�x�N�g��
XMMATRIX LookAtMatrix(XMFLOAT3& lookat,XMFLOAT3& upper, XMFLOAT3& right)
{
	//����������������(1,0,0)�Ƃ����ƃ[���x�N�g���ɂȂ邩������Ȃ��̂�up�x�N�g�����p�ӂ��Ƃ���
	//�ǂ��������g���@�Ƃ�����

	//��������������
	XMVECTOR vz = XMVector3Normalize(XMLoadFloat3(&lookat));
	//����Y��
	XMVECTOR vy = XMVector3Normalize(XMLoadFloat3(&upper));
	XMVECTOR vx;

	float dot= XMVector3Dot(XMLoadFloat3(&XMFLOAT3(0, 2, 0)), XMLoadFloat3(&XMFLOAT3(0, 1, 0))).m128_f32[0];

	//����������������y�������������������Ă���right��Ōv�Z������
	if (XMVector3Dot(vy, vz).m128_f32[0] >= 1.0f)
	{
		vx = XMVector3Normalize(XMLoadFloat3(&right));
		vy = XMVector3Normalize(XMVector3Cross(vz, vx));
		vx = XMVector3Normalize(XMVector3Cross(vy, vz));
	}
	else
	{
		//x�������߂�
		vx = XMVector3Normalize(XMVector3Cross(vy, vz));
		//����Y���ł͂Ȃ�Y�������߂�
		vy = XMVector3Normalize(XMVector3Cross(vz, vx));
	}

	XMFLOAT3 v1 = { 0, 1, 0.5f };
	XMFLOAT3 v2 = { 0, 1, 1 };
	float f = XMVector3Dot(XMLoadFloat3(&v1), XMLoadFloat3(&v2)).m128_f32[0];
	
	int lk = 0;

	////����x��
	//XMVECTOR vx = XMVector3Normalize(XMLoadFloat3(&right));
	////����������������������������Y��
	//XMVECTOR vy = XMVector3Normalize(XMVector3Cross(vz,vx));

	////����x���ł͂Ȃ�����������������������������x�������߂�
	//vx = XMVector3Normalize(XMVector3Cross(vy, vz));

	XMMATRIX ret = XMMatrixIdentity();
	XMFLOAT3 fx, fy, fz;
	XMStoreFloat3(&fx, vx);
	XMStoreFloat3(&fy, vy);
	XMStoreFloat3(&fz, vz);

	//directX������n�ɑ΂���PMD�t�@�C���͉E��n�ō���Ă���̂�Z�l�����K�̕��Ɍ�������+
	//directX�Ƃ͍��W�n���Ⴄ�̂ł��Ƃ��𔽓]������
	ret._11 = -fx.x; ret._12 = -fx.y; ret._13 = -fx.z; ret._14 = 0;
	ret._21 = fy.x; ret._22 = fy.y; ret._23 = fy.z; ret._24 = 0;
	ret._31 = -fz.x; ret._32 = -fz.y; ret._33 = -fz.z; ret._34 = 0;
	ret._41 = 0; ret._42 = 0; ret._43 = 0; ret._44 = 1;

	
	XMVECTOR tes = { 0, 1, 0 ,1};
	tes = XMVector3TransformCoord(tes, ret);
	XMFLOAT4 r;
	XMStoreFloat4(&r, tes);
	int jj = 0;

	return ret;
}

//����x�N�g����ʂ̃x�N�g�������Ɍ�������s���Ԃ�
//origin�@���̃x�N�g���@lookat�@�����������x�N�g��
XMMATRIX LookAtMatrix(XMFLOAT3& origin,XMFLOAT3& lookat ,XMFLOAT3& up, XMFLOAT3& right)
{
	XMMATRIX tmp = LookAtMatrix(origin, up, right);
	//��]�������������̂œ]�u����Ƌt�s��ɂȂ�
	tmp = XMMatrixMultiply(XMMatrixTranspose(tmp),LookAtMatrix(lookat, up, right));
	return tmp;
}

//�L����������̃x�N�g���������悤�ɂ���
//@param x�x�N�g��
//@param y�x�N�g��
//@param z�x�N�g��
XMMATRIX LookAt(float x, float y, float z)
{
	XMMATRIX lookMatrix = LookAtMatrix(XMFLOAT3(x, y, z), XMFLOAT3(0,1,0), XMFLOAT3(1, 0, 0));
	XMVECTOR test = { 0, 0, 3, 0 };

	test = XMVector3Transform(test, lookMatrix);
	XMFLOAT3 ftest;
	XMStoreFloat3(&ftest, test);
	int jj = 0;
	return lookMatrix;
}

void MatrixTransmission(int idx, XMMATRIX& parent, std::vector<XMMATRIX>& boneMatrixies, std::vector<BoneNode>& boneTree, std::vector<BONE>& bones)
{
	//16�o�C�g�A���C�����g�̊֌W�ň�x���[�J���ϐ��ɓ���Ă�
	XMMATRIX bone = boneMatrixies[idx];
	bone = XMMatrixMultiply(bone, parent);
	boneMatrixies[idx] = bone;

	//������sibling�i�Z��j�ɐe�s���n��
	while (boneTree[idx].sibling != 0xffff)
	{
		MatrixTransmission(boneTree[idx].sibling, parent, boneMatrixies, boneTree, bones);
		break;
	}
	//�����̎q�Ɏ����̍s���n��
	while (boneTree[idx].child != 0xffff)
	{
		MatrixTransmission(boneTree[idx].child, bone, boneMatrixies, boneTree, bones);
		break;
	}

}



//CCD_IK���g���ă{�[���ʒu����������
//�{�[����IKList
//mesh�@�Ώۃ��b�V���@�i�K�v�Ȃ̂̓{�[����IKList
//ikName�@����������IK�{�[���̖��O
//offset ����IK�{�[�������̈ʒu����ǂꂭ�炢���ꂽ��
//�@�����ʂ̃{�[���͉�]�����o���Ȃ���IK�{�[���͈ړ����ł���

void
Player::CcdIkSolve(PMDMesh& mesh,const std::string& ikName,XMFLOAT3& offset)
{

	//�����W���瓮���ĂȂ�������������Ȃ�
	if (offset == XMFLOAT3(0, 0, 0))
	{
		return;
	}
	std::map<std::string, PMDMesh::IKList>& ikmap = mesh.IKListMap();
	std::map<std::string, int>& bonemap = mesh.GetBoneMap();



	if (ikmap.find(ikName) == ikmap.end() || bonemap.find(ikName) == bonemap.end()){
		return;//������Ȃ������珈�����Ȃ�
	}

	PMDMesh::IKList& iklist = ikmap[ikName];
	int boneIdx = bonemap[ikName];

	//�Ώۃ{�[���̎擾
	//PMDMesh::Bone& bone = mesh.Bones()[boneIdx];
	BoneVert& bone = mesh.GetBoneVertices()[boneIdx];

	//�E��IK�{�[��������ʒu
	XMFLOAT3 ikOriginPos = bone.head.pos;
	//�ڕW�ʒu
	XMFLOAT3 ikTargetPos = bone.head.pos + offset;

	//IK�ʒu�Ƀ}�[�J�[�\������̂ł��ꂪ�v��
	_ikpos = ikTargetPos;

	//�܂���IK�̊Ԃɂ���{�[���̍��W�̈ꎞ�ϐ��z������
	//���R��IK�ċA���閈�ɁA�{�[�����W���ύX����邩��ł���
	std::vector<XMFLOAT3> tmpBonePositions(iklist.ikchainLen);
	for (int i = 0; i < iklist.ikchainLen; ++i)
	{
		//tmpBonePositions[i] = mesh.Bones()[iklist.ikboneIndices[i]].headPos;
		tmpBonePositions[i] = mesh.GetBoneVertices()[iklist.ikboneIndices[i]].head.pos;
	}

	//�{�[���̍����������iIK����ł������{�[���j����IK�ւ̃x�N�g��
	XMFLOAT3 ikOriginRootVec = ikOriginPos - tmpBonePositions[iklist.ikchainLen - 1];
	
	//�{�[���̍����������iIK����ł������{�[���j����ύX��IK�ւ̃x�N�g��
	XMFLOAT3 ikTargetRootVec = ikTargetPos - tmpBonePositions[iklist.ikchainLen - 1];

	//�{���̒����ȏ�ɂȂ낤�Ƃ�����N�����v����------------------------------------------
	float ikmaxlen = Length(ikOriginRootVec);
	if (Length(ikTargetRootVec) > ikmaxlen)
	{
		XMVECTOR v = XMLoadFloat3(&ikTargetRootVec);
		v = XMVector3ClampLength(v, 0.1f, ikmaxlen);
		XMStoreFloat3(&ikTargetRootVec, v);

		ikTargetPos = tmpBonePositions[iklist.ikchainLen - 1] + ikTargetRootVec ;
	}
	_ikpos = ikTargetPos;
	//------------------------------------------------------------------------------------
	

	if (ikOriginRootVec == ikTargetRootVec)
	{
		return;
	}

	//IK�ړ��O����ړ���ւ̉�]�x�N�g�����v�Z���Ă���
	XMMATRIX matIkRot = LookAtMatrix(ikOriginRootVec, ikTargetRootVec,XMFLOAT3(0,1,0),XMFLOAT3(1,0,0));

	XMFLOAT3 rootAxis = Cross(Normalize(ikOriginRootVec), Normalize(ikTargetRootVec));
	float rootangle = XMVector3AngleBetweenNormals(XMLoadFloat3(&ikOriginRootVec), XMLoadFloat3(&ikTargetRootVec)).m128_f32[0];
	ikOriginRootVec = Normalize(ikOriginRootVec);
	ikTargetRootVec = Normalize(ikTargetRootVec);
	matIkRot = XMMatrixRotationAxis(XMLoadFloat3(&rootAxis), rootangle);

	matIkRot = LookAtMatrix(ikOriginRootVec, ikTargetRootVec, XMFLOAT3(0, 1, 0), XMFLOAT3(1, 0, 0));

	//��������CCD_IK
	//�ق�Ƃ͂����ŃT�C�N���b�N�i�J��Ԃ��j���邪�A�܂�1��ڂ̎������l����
	for (int c = 0; c < iklist.iterations; ++c)
	{

		matIkRot = LookAtMatrix(ikOriginRootVec, ikTargetRootVec, XMFLOAT3(0, 1, 0), XMFLOAT3(-1, 0, 0));
		
		//���ԃ{�[�����W�␳
		for (int i = 0; i < iklist.ikchainLen; ++i)
		{

			int ikboneIdx = iklist.ikboneIndices[i];
			XMFLOAT3 originVec = ikOriginPos - tmpBonePositions[i];
			XMFLOAT3 targetVec = ikTargetPos - tmpBonePositions[i];

			//���ꂼ��̃x�N�g����������������ꍇ�͏�����ł��؂�
			if (Length(originVec) < 0.0001f || Length(targetVec) < 0.0001f){
				return;
			}

			//���K������
			originVec = Normalize(originVec);
			targetVec = Normalize(targetVec);


			//�O�ς��玲���쐬����
			XMFLOAT3 axis = Normalize(Cross(originVec, targetVec));


			//matIkRot = LookAtMatrix(originVec, targetVec, XMFLOAT3(0, 1, 0), XMFLOAT3(-1, 0, 0));


			//�����Ђ��n�Ȃ�AX������]���Ƃ���
			if (mesh.GetBoneVertices()[ikboneIdx].name.find("�Ђ�") != std::string::npos)//if (bone.name.find("�Ђ�") != std::string::npos)
			{
				axis.x = -1.0f;
				axis.y = 0.0f;
				axis.z = 0.0f;
				//���̎���matIkRot
				XMVECTOR tempvec = XMLoadFloat3(&axis);
				tempvec = XMVector3Transform(tempvec, matIkRot);
				XMStoreFloat3(&axis, tempvec);
				int kj = 0;
			}
			else
			{
				if (Length(axis) == 0.0f)
				{
					return;//�O�ό��ʂ�0�ɂȂ��Ă�Ȃ烊�^�[��
				}
			}
			//��̃x�N�g���̊Ԃ̊p�x���v�Z
			float angle = XMVector3AngleBetweenNormals(XMLoadFloat3(&originVec), XMLoadFloat3(&targetVec)).m128_f32[0];
			if (angle < 0.0001f)return;

			//�����p�x���v�Z
			float strict = (iklist.limitangle * 4)/(float)iklist.ikchainLen*(float)(i+1);//iklist.limitangle*4;//

			if (angle > strict)
			{
				int jk = 0;
			}
			angle = min(angle,strict);
			angle = max(angle, -strict);
			angle *= 0.5f;//�����ɂ��Ă�Ӗ��͂悭������Ȃ�


			//�{�[���ϊ��s����v�Z
			XMMATRIX rotMat = XMMatrixRotationAxis(XMLoadFloat3(&axis), angle);
			//XMVECTOR q = XMQuaternionRotationAxis()

			//rotMat�̃e�X�g
			//originVec��rotMat�ŉ�]�������targetVec�ɂȂ�͂�
			XMVECTOR vecTest = XMVector3Transform(XMLoadFloat3(&originVec),rotMat);
			originVec;
			targetVec;
			int jkjk = 0;
			//rotMat��ok

			//�I�t�Z�b�g���l�������s������i���_�Ɉړ�����]�����̍��W���ړ�
			/*XMMATRIX mat = XMMatrixTranslation(-bone.head.pos.x, -bone.head.pos.y, -bone.head.pos.z)
				*rotMat*
				XMMatrixTranslation(bone.head.pos.x, bone.head.pos.y, bone.head.pos.z);*/
			XMMATRIX mat = XMMatrixTranslation(
				-mesh.GetBoneVertices()[ikboneIdx].head.pos.x,
				-mesh.GetBoneVertices()[ikboneIdx].head.pos.y,
				-mesh.GetBoneVertices()[ikboneIdx].head.pos.z)
				*rotMat
				*XMMatrixTranslation(
				mesh.GetBoneVertices()[ikboneIdx].head.pos.x,
				mesh.GetBoneVertices()[ikboneIdx].head.pos.y,
				mesh.GetBoneVertices()[ikboneIdx].head.pos.z);

			//�ϊ��s����v�Z�i�I�t�Z�b�g���l��
			mesh.BoneMatrixies()[iklist.ikboneIndices[i]] = mesh.BoneMatrixies()[ikboneIdx] * mat;



			XMFLOAT3& tmpbonePosition = tmpBonePositions[i];
			mat = XMMatrixTranslation(-tmpbonePosition.x, -tmpbonePosition.y, -tmpbonePosition.z)*
				rotMat*
				XMMatrixTranslation(tmpbonePosition.x, tmpbonePosition.y, tmpbonePosition.z);


			

			//IK�{�[���̈ʒu�Ɂ��̍s�񂩂��Ă��
			ikOriginPos = ikOriginPos*mat;
			//������薖�[���̃{�[���ʒu�ɑ΂��Ď����̕ϊ��s��������Ă��
			for (int j = 0; j < i; ++j)
			{
				tmpBonePositions[j] = tmpBonePositions[j] * mat;
			}
			//_ikpos = ikOriginPos;
		}
	}

}


void DeformBones(PMDMesh* mesh, VMDData* vmddata, unsigned int frameNo)
{
	//�{�[���s��̏�����
	std::fill(mesh->BoneMatrixies().begin(), mesh->BoneMatrixies().end(), XMMatrixIdentity());


	for (auto& frames : vmddata->GetKeyframes())
	{
		
		//�t�C�e���[�^
		std::vector<VMDData::MotionData>::reverse_iterator revit = std::find_if(frames.second.rbegin(), frames.second.rend(), 
			[frameNo](const VMDData::MotionData& md){
			return md.frameNo <= frameNo;
		});
		//�������������_��20 10 0�̏��ɓǂ��0����Ȃ�10��it���A���Ă�
		//if (revit == frames.second.rbegin()) revit = revit + 1;
		if (revit == frames.second.rend()) continue;//������Ȃ�������

		

		auto r_it = frames.second.rbegin();
		auto r = *r_it;
		auto r_base = r_it.base();
		auto _it = frames.second.begin();
		
		

		//iterator�͓���̗v�f���w�������Ƃ��Ɏg����
		//����g���Ă���find_if�Ȃ́u�v�f��Ԃ��֐��v��iterator��Ԃ�
		//it++ or ++it �͎w�������v�f�����̗v�f�ɂ���
		//�iEffectiveC++�ł�++it���g���Ə�����Ă���
		//�~for(;it<vec.end();)�@��it!=vec.end()

		//���o�[�X�C�e���[�^
		//�t�����C�e���[�^
		//begin()�`end()��rbegin()�`rend()�ɑΉ�
		//begin()�̓R���e�i�̍ŏ��̗v�f�̕@�������
		//rbegin()�̓R���e�i�̍Ō�̗v�f������
		//end()�̓R���e�i�̍Ō�̗v�f�̂��K�������i�Ō�̗v�f���w�������Ă���킯�ł͂Ȃ�
		//rbegin()�̓R���e�i�̍Ō�̗v�f�̂��K�Ȃ񂾂��ǁA�t�����ɐ�����̂ŁAend()�ƈႢ�Ō�̃R���e�i���w������
		//���o�[�X�C�e���[�^����C�e���[�^�ւ̕ϊ�
		//it.base()
		//�Ƃ����֐����g���ƐK�����Ă������̏�ŐU��Ԃ��ĕ@�悩�琔���n�߂�

		//�Ή�����{�[�������������珈�����Ȃ��悤�ɂ���
		if (mesh->GetBoneMap().count(frames.first) == 0) continue;
		int idx = mesh->GetBoneMap()[frames.first];

		auto it = revit.base();
		float t = 0.0f;
		XMVECTOR v;
		if (it != frames.second.end())
		{
			t = (float)(frameNo - revit->frameNo) / (float)(it->frameNo - revit->frameNo);//���̕�
			v = XMQuaternionSlerp(revit->quaternion, it->quaternion, t);
		}
		else
		{
			v = revit->quaternion;
		}


		XMFLOAT3 head = mesh->GetBoneVertices()[idx].head.pos;
		mesh->BoneMatrixies()[idx] =
			XMMatrixTranslation(-head.x, -head.y, -head.z)*
			XMMatrixRotationQuaternion(v)/*XMMatrixRotationQuaternion(revit->quaternion)*/*
			XMMatrixTranslation(head.x, head.y, head.z);
	}

}


Player::Player(const std::shared_ptr<Camera> camera) :dev(DeviceDx11::Instance()), _cameraPtr(camera)
{
	_pos = XMFLOAT3(0, 0, 0);
	_scale = XMFLOAT3(1, 1, 1);
	_rot = XMFLOAT3(0, 0, 0);
}

void
Player::Init()
{
	HRESULT result;
	ResourceManager& resourceMgr = ResourceManager::Instance();
	

	_pos = { 0.f,0.0f, 0.f };
	rotAngle = 0.f;

	//pmx�̓f�t�H���g��utf16�iUnicode�j���g���̂�L�t���ă��C�h������ɂ���
	//PMXLoader pmxloader;
	//pmxloader.LoadPMX(L"models/shame/shame.pmx");

	//pmd�t�@�C���̓ǂݍ���
	PMDLoader loader;
	//_mesh = loader.Load("models/kouhai/kouhai_v1.pmd");
	_mesh = loader.Load("models/kouhai/kouhai_v1.pmd");

	//vmd���[�h�̃t���[�����̑召���r���Ĉ�ԑ傫����i�ŏI�t���[���j��ێ����Ƃ�
	//maxframe=std::max(frame, maxframe);
	//vmd���Ƀ��s�[�g�t���O����������
	//VMD�t�@�C���̓ǂݍ���
	VMDLoader vmdLoader;
	_vmd = vmdLoader.Load("neutral.vmd",true);
	RegisterAnim("charge", _vmd);
	VMDData* run = vmdLoader.Load("run.vmd",true);
	RegisterAnim("run", run);
	VMDData* neutral = vmdLoader.Load("neutral.vmd", true);
	RegisterAnim("neutral", neutral);


	_currentVMD = _animations["charge"];//_vmd �̕�����_currentVMD�ɕς���

	unsigned int stride = _mesh->GetVertexStride();
	unsigned int offset = 0;
	ID3D11Buffer* pmdVertBuff = _mesh->GetVertexBuffer();
	dev.Context()->IASetVertexBuffers(0, 1, &pmdVertBuff, &stride, &offset);


	dev.Context()->IASetIndexBuffer(_mesh->GetIndexBuffer(), DXGI_FORMAT_R16_UINT, 0);

	dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//�V�F�[�_�R���p�C���A���[�h
	resourceMgr.LoadVS("Player_VS",
		"BaseShader.hlsl", "BaseVS", "vs_5_0", _vertexShader, inputElementDescs,
		sizeof(inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		_vertInputLayout);
	resourceMgr.LoadPS("Player_PS",
		"BaseShader.hlsl", "BasePS", "ps_5_0", _pixelShader);

	
	resourceMgr.LoadVS("Player_boneVS",
		"BaseShader.hlsl", "BoneVS", "vs_5_0", _boneVertexShader, boneInputElementDescs,
		sizeof(boneInputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		_boneInputLayout);
	resourceMgr.LoadPS("Player_bonePS",
		"BaseShader.hlsl", "BonePS", "ps_5_0", _bonePixelShader);

	
	dev.Context()->VSSetShader(*_vertexShader.lock(), nullptr, 0);//�o�l�c���f���\���p�V�F�[�_�Z�b�g
	dev.Context()->PSSetShader(*_pixelShader.lock(), nullptr, 0);//PMD���f���\���p�V�F�[�_�Z�b�g

	resourceMgr.LoadVS("Player_lightVS",
		"lightview.hlsl", "LightViewVS", "vs_5_0", _depthVS,
		lightViewInputElementDescs, sizeof(lightViewInputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		_depthViewInputLayout);
	resourceMgr.LoadPS("Player_lightPS",
		"lightview.hlsl", "LightViewPS", "ps_5_0", _depthPS);

	

	_worldAndCamera.world = XMMatrixIdentity();
	_worldAndCamera.cameraView = _cameraPtr.lock()->CameraView();
	_worldAndCamera.cameraProj = _cameraPtr.lock()->CameraProjection();
	_worldAndCamera.lightView = _cameraPtr.lock()->LightView();
	_worldAndCamera.lightProj = _cameraPtr.lock()->LightProjection();
	/*_matrixMVP.worldMatrix = XMMatrixIdentity();

	_matrixMVP.viewMatrix = _cameraPtr.lock()->GetMatrixies().view;


	_matrixMVP.projectionMatrix = _cameraPtr.lock()->GetMatrixies().projection;*/


	


	//mvp�s��p�̃o�b�t�@���
	//
	D3D11_BUFFER_DESC matBuffDesc = {};
	matBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matBuffDesc.ByteWidth = sizeof(WorldAndCamera);
	matBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;//�o�b�t�@�̒��g��CPU�ŏ���������
	matBuffDesc.Usage = D3D11_USAGE_DYNAMIC;//CPU�ɂ�鏑�����݁AGPU�ɂ��ǂݍ��݂��s����Ƃ����Ӗ�
	//matBuffDesc.ByteWidth = sizeof(XMMATRIX);
	//matBuffDesc.StructureByteStride = sizeof(XMMATRIX);
	
	D3D11_SUBRESOURCE_DATA d;
	d.pSysMem = &_worldAndCamera;

	result = dev.Device()->CreateBuffer(&matBuffDesc, &d, &_matrixBuffer);

	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
	//�����ł��̃������̉�ɁA�}�g���b�N�X�̒l���R�s�[���Ă��
	memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
	
	dev.Context()->Unmap(_matrixBuffer, 0);

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);


	//�V�F�[�_�ɓn���p�̃}�e���A���\����
	_material = {};

	D3D11_BUFFER_DESC materialBuffDesc = {};
	materialBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//�R���X�^���g�o�b�t�@��ByteWidth��16�̔{���łȂ��Ƃ����Ȃ�
	materialBuffDesc.ByteWidth = sizeof(Material) + (16 - sizeof(Material) % 16) % 16;
	materialBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	materialBuffDesc.Usage = D3D11_USAGE_DYNAMIC;
	materialBuffDesc.StructureByteStride = sizeof(Material);

	D3D11_SUBRESOURCE_DATA materialData;
	materialData.pSysMem = &_material;

	result = dev.Device()->CreateBuffer(&materialBuffDesc, &materialData, &_materialBuffer);

	dev.Context()->VSSetConstantBuffers(1, 1, &_materialBuffer);


	

	unsigned int boneVertOffset = 0;
	unsigned int boneVertStride = _mesh->GetBoneVertStride();
	ID3D11Buffer* boneVertBuff = _mesh->GetBoneVertBuffer();


	//�V�F�[�_�ɓn���{�[���s��
	
	/*for (auto& bonemat : _boneMatrixies)
	{
		bonemat = XMMatrixIdentity();
	}
	D3D11_BUFFER_DESC boneMatrixBuffDesc = {};
	boneMatrixBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	boneMatrixBuffDesc.ByteWidth = sizeof(XMMATRIX) * 512;
	boneMatrixBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	boneMatrixBuffDesc.MiscFlags = 0;
	boneMatrixBuffDesc.Usage = D3D11_USAGE_DYNAMIC;
	boneMatrixBuffDesc.StructureByteStride = sizeof(XMMATRIX);

	D3D11_SUBRESOURCE_DATA boneMatData;
	boneMatData.pSysMem = &_boneMatrixies[0];

	dev.Device()->CreateBuffer(&boneMatrixBuffDesc, &boneMatData, &_boneMatrixBuff);*/
	ID3D11Buffer* buf = _mesh->GetBoneMatrixBuffer();
	dev.Context()->VSSetConstantBuffers(3, 1, &buf);

	
	//LookAt�s��̃e�X�g
	XMMATRIX lookMatrix = LookAtMatrix(XMFLOAT3(0, 1, 0), XMFLOAT3(0,1,0), XMFLOAT3(1, 0, 0));
	XMVECTOR test = { 0, 0, 1, 0 };

	test = XMVector3Transform(test, lookMatrix);
	XMFLOAT3 ftest;
	XMStoreFloat3(&ftest,test);
	int jj = 0;


	//�{�[���̃c���[�\�������
	_boneTree.resize(_mesh->BoneSize());
	//�c���[������
	for (auto& node : _boneTree)
	{
		memset((void*)(&node.child), 0xffff, sizeof(unsigned short));
		memset((void*)(&node.sibling), 0xffff, sizeof(unsigned short));
	}


	std::vector<BONE> pmdBones = _mesh->GetBones();
	//�c���[�T��
	for (unsigned int i = 0; i <_boneTree.size(); i++)
	{
		unsigned short parentIdx = pmdBones[i].parentBoneIdx;
		//�e�����Ȃ��Ȃ珈�����Ȃ�
		if (parentIdx == 0xffff) continue;
		//�܂��e�m�[�h�����߂�@�|�C���^�^�Ȃ̂ł����ŘM��Ό��̕��ɂ����f�����
		BoneNode& parentNode = _boneTree[parentIdx];
		//�q�����Ȃ���Ύq������
		if (parentNode.child == 0xffff)
		{
			parentNode.child = i;
			continue;
		}
		unsigned int idx = parentNode.child;
		//�q�����ɂP�ȏ㎝���Ă�����A�q��sibling�i�Z��j�ɐݒ肷��
		//�q������sibling�������Ă����炻��sibling��sibling�ɁE�E�E�Ƃ��������ł���Ă���
		while (_boneTree[idx].sibling != 0xffff)
		{
			idx = _boneTree[idx].sibling;
		}
		_boneTree[idx].sibling = i;
	}



	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	
	


	//�T���v���̐ݒ�
	ID3D11SamplerState* samplerState;
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	dev.Device()->CreateSamplerState(&samplerDesc, &samplerState);

	dev.Context()->PSSetSamplers(0, 1, &samplerState);
	dev.Context()->VSSetSamplers(1, 1, &samplerState);

	ID3D11SamplerState* samplerState_clamp;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	dev.Device()->CreateSamplerState(&samplerDesc, &samplerState_clamp);
	dev.Context()->PSSetSamplers(2, 1, &samplerState_clamp);


	_transMatrix = XMMatrixIdentity();
	_rotMatrix = XMMatrixIdentity();
	_scaleMatrix = XMMatrixIdentity();

	_frameNo = 0;

	_ikOffset = { 0, 0, 0 };

	DeformBones(_mesh, _currentVMD, _frameNo);


	//�{�[���̕ϊ��s����q�̃{�[���ɓ`�d
	MatrixTransmission(0, XMMatrixIdentity(), _mesh->BoneMatrixies(), _boneTree, _mesh->GetBones());


	//LookAt�e�X�g
	XMFLOAT3 t = { 0, 1, 0 };
	XMFLOAT3 ori = { 0, 1, 0 };
	XMFLOAT3 target = { 1, 0, 0 };

	XMMATRIX te = LookAtMatrix(ori, target, XMFLOAT3(0, 1, 0), XMFLOAT3(1, 0, 0));
	XMVECTOR v = XMVector3Transform(XMLoadFloat3(&t), te);
	int kj = 0;

	_ikpos = { 0, 0, 0 };

}


Player::~Player()
{

}

void
Player::Update()
{
	//_frameNo++;
	//_frameNo %= _frameNo=_frameNo%_currentVMD->Duration();//180;//
	


	//LookAt�̃e�X�g
	XMMATRIX transMatrix = XMMatrixTranslation(_pos.x, _pos.y, _pos.z);//LookAt(0, 1, 1);//
	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(_rot.x, _rot.y, _rot.z);//LookAt(1, 0, 0);
	XMMATRIX scaleMat = XMMatrixScaling(_scale.x, _scale.y, _scale.z);
	XMMATRIX world = XMMatrixMultiply( XMMatrixMultiply(rotMat,scaleMat),transMatrix);


	_worldAndCamera.world = world;
	_worldAndCamera.cameraView = _cameraPtr.lock()->CameraView();
	_worldAndCamera.cameraProj = _cameraPtr.lock()->CameraProjection();
	_worldAndCamera.lightView = _cameraPtr.lock()->LightView();
	_worldAndCamera.lightProj = _cameraPtr.lock()->LightProjection();

	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
	//�����ł��̃������̉�ɁA�}�g���b�N�X�̒l���R�s�[���Ă��
	memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
	
	dev.Context()->Unmap(_matrixBuffer, 0);

	//dev.Context()->VSSetConstantBuffers(0, 1, &matrixBuffer);

	//�}�b�v���ꂽ�{�[���o�b�t�@�ɏ�������
	dev.Context()->Map(_mesh->GetBoneMatrixBuffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &_mappedBones);
	memcpy(_mappedBones.pData, (void*)(&_mesh->BoneMatrixies()[0]), sizeof(XMMATRIX) * 512);
	dev.Context()->Unmap(_mesh->GetBoneMatrixBuffer(), 0);

	std::copy(_key, _key + sizeof(_key), _oldkey);
	GetKeyboardState(_key);


	ID3D11Buffer* pmdVertBuff = _mesh->GetVertexBuffer();
	ID3D11Buffer* boneVertBuff = _mesh->GetBoneVertBuffer();

	if (_key[VK_RETURN] & 0x80)
	{
		if (!(_oldkey[VK_RETURN] & 0x80))
		{
			_isBoneView = !_isBoneView;
		}
	}

	if (_isBoneView)
	{
		unsigned int stride = _mesh->GetBoneVertStride();
		unsigned int offset = 0;
		dev.Context()->IASetInputLayout(*_boneInputLayout.lock());
		dev.Context()->IASetVertexBuffers(0, 1, &boneVertBuff, &stride, &offset);
		dev.Context()->VSSetShader(*_boneVertexShader.lock(), nullptr, 0);//�{�[���\���p�V�F�[�_�Z�b�g
		dev.Context()->PSSetShader(*_bonePixelShader.lock(), nullptr, 0);//�{�[���\���p�V�F�[�_�Z�b�g
		dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}
	else
	{
		unsigned int stride = _mesh->GetVertexStride();
		unsigned int offset = 0;

		dev.Context()->IASetInputLayout(*_vertInputLayout.lock());
		dev.Context()->IASetVertexBuffers(0, 1, &pmdVertBuff, &stride, &offset);
		dev.Context()->VSSetShader(*_vertexShader.lock(), nullptr, 0);//�o�l�c���f���\���p�V�F�[�_�Z�b�g
		dev.Context()->PSSetShader(*_pixelShader.lock(), nullptr, 0);//PMD���f���\���p�V�F�[�_�Z�b�g		dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	if (_key[VK_CONTROL] & 0x80)
	{
		if (_key[VK_UP] & 0x80)
		{
			_ikOffset.y += 0.1f;
		}
		if (_key[VK_DOWN] & 0x80)
		{
			_ikOffset.y -= 0.1f;
		}
		if (_key[VK_RIGHT] & 0x80)
		{
			_ikOffset.x += 0.1f;
		}
		if (_key[VK_LEFT] & 0x80)
		{
			_ikOffset.x -= 0.1f;
		}
	}

	DeformBones(_mesh, _currentVMD, _frameNo/2);// /2�̓��[�V������x�����邽�߂ɕt���Ă邾��
	if (_currentVMD->IsRepeat()){
		_frameNo = (_frameNo + 1) % (_currentVMD->Duration() * 2 + 2);//�O�̎ʂ�������
	}
	else{
		_frameNo = min(_frameNo + 1, _currentVMD->Duration()*2+2);
		if (_frameNo == _currentVMD->Duration() * 2 + 2) SetAnimation("charge");
	}

	std::string name = "�E���h�j";
	CcdIkSolve(*_mesh, name, _ikOffset);
	int id = _mesh->GetBoneMap()[name];
	XMMATRIX m = _mesh->BoneMatrixies()[69];


	//�{�[���̕ϊ��s����q�̃{�[���ɓ`�d
	MatrixTransmission(0, XMMatrixIdentity(), _mesh->BoneMatrixies(), _boneTree, _mesh->GetBones());
	
}

void
Player::Draw()
{
	

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);
	dev.Context()->IASetIndexBuffer(_mesh->GetIndexBuffer(), DXGI_FORMAT_R16_UINT, 0);

	ID3D11Buffer* pmdVertBuff = _mesh->GetVertexBuffer();
	ID3D11Buffer* boneVertBuff = _mesh->GetBoneVertBuffer();
	if (!_isBoneView){
		unsigned int stride = _mesh->GetVertexStride();
		unsigned int offset = 0;

		dev.Context()->IASetInputLayout(*_vertInputLayout.lock());
		dev.Context()->IASetVertexBuffers(0, 1, &pmdVertBuff, &stride, &offset);
		dev.Context()->VSSetShader(*_vertexShader.lock(), nullptr, 0);//�o�l�c���f���\���p�V�F�[�_�Z�b�g
		dev.Context()->PSSetShader(*_pixelShader.lock(), nullptr, 0);//PMD���f���\���p�V�F�[�_�Z�b�g
		dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//XMMATRIX world = XMMatrixIdentity();

		//XMMATRIX scaleMatrix = XMMatrixScaling(0.8f, 0.8f, 0.8f);
		//world = XMMatrixMultiply(scaleMatrix,world);

		rotAngle += 1 * XM_PI / 180;//�Ƃ肠������]�����Ă���

		//LookAt�̃e�X
		//XMMATRIX transMatrix = XMMatrixTranslation(_pos.x, _pos.y, _pos.z);//LookAt(1, 0, 0);//
		//world = XMMatrixMultiply(transMatrix, world);


		_worldAndCamera.cameraView = _cameraPtr.lock()->CameraView();
		_worldAndCamera.cameraProj = _cameraPtr.lock()->CameraProjection();
		_worldAndCamera.lightView = _cameraPtr.lock()->LightView();
		_worldAndCamera.lightProj = _cameraPtr.lock()->LightProjection();

		dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
		//�����ł��̃������̉�ɁA�}�g���b�N�X�̒l���R�s�[���Ă��
		memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
		
		dev.Context()->Unmap(_matrixBuffer, 0);


		std::vector<PMDMaterial> pmdMaterials = _mesh->GetMaterials();
		std::vector<ID3D11ShaderResourceView*> pmdTextures = _mesh->GetTextures();

		for (int i = 0; i < (int)pmdMaterials.size(); i++)
		{
			dev.Context()->PSSetShaderResources(TEXTURE_MAIN, 1, &pmdMaterials[i].texture);//texture
			dev.Context()->PSSetShaderResources(TEXTURE_SPH, 1, &pmdMaterials[i].sph);//sph
			dev.Context()->PSSetShaderResources(TEXTURE_SPA, 1, &pmdMaterials[i].spa);//spa
			_material.diffuse = pmdMaterials[i].diffuse;
			_material.alpha = pmdMaterials[i].alpha;
			_material.specularColor = pmdMaterials[i].specularColor;
			_material.specularity = pmdMaterials[i].specularity;
			_material.ambient = pmdMaterials[i].ambient;
			dev.Context()->Map(_materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mappedMaterial);
			memcpy(_mappedMaterial.pData, (void*)(&_material), sizeof(_material));
			dev.Context()->Unmap(_materialBuffer, 0);


			dev.Context()->DrawIndexed(pmdMaterials[i].indicesNum, pmdMaterials[i].offset, 0);//�C���f�b�N�X�͏��Ԃɕ���ł�H�̂Ł��ŕ`�悵�����I�t�Z�b�g�l�𑫂�

		}

		


		////�e�`��
		//XMMATRIX shadow = XMMatrixShadow(XMVectorSet(0, 1, 0, -0.2), XMVectorSet(-1, 1, -1, 0));
		////Map
		//XMMATRIX temp = _worldAndCamera.world;
		//temp = XMMatrixMultiply(shadow, temp);
		//_worldAndCamera.world = temp;
		//dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
		////�����ł��̃������̉�ɁA�}�g���b�N�X�̒l���R�s�[���Ă��
		//memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
		//dev.Context()->Unmap(_matrixBuffer, 0);
		////Unmap

		//_material.ambient = XMFLOAT3(0, 0, 0);
		//dev.Context()->Map(_materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mappedMaterial);
		//memcpy(_mappedMaterial.pData, (void*)(&_material), sizeof(_material));
		//dev.Context()->Unmap(_materialBuffer, 0);


		//dev.Context()->DrawIndexed(GetMesh()->IndexSize(), 0, 0);

		//XMMATRIX inv;
		//XMVECTOR dummy;
		//inv = XMMatrixInverse(&dummy, shadow);
		//XMMATRIX t = _matrixMVP.worldMatrix;
		//_matrixMVP.worldMatrix=XMMatrixMultiply(inv, t);
		//dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
		////�����ł��̃������̉�ɁA�}�g���b�N�X�̒l���R�s�[���Ă��
		//memcpy(_mem.pData, (void*)(&_matrixMVP), sizeof(_matrixMVP));
		//dev.Context()->Unmap(_matrixBuffer, 0);
	}
	else
	{

		unsigned int stride = _mesh->GetBoneVertStride();
		unsigned int offset = 0;
		dev.Context()->IASetInputLayout(*_boneInputLayout.lock());
		dev.Context()->IASetVertexBuffers(0, 1, &boneVertBuff, &stride, &offset);
		dev.Context()->VSSetShader(*_boneVertexShader.lock(), nullptr, 0);//�{�[���\���p�V�F�[�_�Z�b�g
		dev.Context()->PSSetShader(*_bonePixelShader.lock(), nullptr, 0);//�{�[���\���p�V�F�[�_�Z�b�g
		dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		dev.Context()->Draw(_mesh->BoneSize() * 2, 0);
	}
}

void 
Player::DrawLightView()
{

	ID3D11Buffer* pmdVertBuff = _mesh->GetVertexBuffer();
	unsigned int stride = _mesh->GetVertexStride();
	unsigned int offset = 0;
	dev.Context()->IASetInputLayout(*_depthViewInputLayout.lock());
	dev.Context()->IASetVertexBuffers(0, 1, &pmdVertBuff, &stride, &offset);
	dev.Context()->VSSetShader(*_depthVS.lock(), nullptr, 0);//�o�l�c���f���\���p�V�F�[�_�Z�b�g
	dev.Context()->PSSetShader(*_depthPS.lock(), nullptr, 0);//PMD���f���\���p�V�F�[�_�Z�b�g
	dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);
	dev.Context()->IASetIndexBuffer(_mesh->GetIndexBuffer(), DXGI_FORMAT_R16_UINT, 0);

	XMMATRIX view = _cameraPtr.lock()->LightView();
	XMMATRIX proj = _cameraPtr.lock()->LightProjection();
	_worldAndCamera.lightView = view;
	_worldAndCamera.lightProj = proj;

	//�o�b�t�@�̍X�V���ĂȂ����̂ł���
	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
	//�����ł��̃������̉�ɁA�}�g���b�N�X�̒l���R�s�[���Ă��
	memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
	
	dev.Context()->Unmap(_matrixBuffer, 0);

	std::vector<PMDMaterial> pmdMaterials = _mesh->GetMaterials();
	for (int i = 0; i < (int)pmdMaterials.size(); i++)
	{
		dev.Context()->DrawIndexed(pmdMaterials[i].indicesNum, pmdMaterials[i].offset, 0);//�C���f�b�N�X�͏��Ԃɕ���ł�H�̂Ł��ŕ`�悵�����I�t�Z�b�g�l�𑫂�

	}
}

void 
Player::DrawCameraDepth()
{
	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);
	dev.Context()->IASetIndexBuffer(_mesh->GetIndexBuffer(), DXGI_FORMAT_R16_UINT, 0);

	XMMATRIX view = _cameraPtr.lock()->CameraView();
	XMMATRIX proj = _cameraPtr.lock()->CameraProjection();
	_worldAndCamera.lightView = view;
	_worldAndCamera.lightProj = proj;


	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
	//�����ł��̃������̉�ɁA�}�g���b�N�X�̒l���R�s�[���Ă��
	memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
	
	dev.Context()->Unmap(_matrixBuffer, 0);


	ID3D11Buffer* pmdVertBuff = _mesh->GetVertexBuffer();
	unsigned int stride = _mesh->GetVertexStride();
	unsigned int offset = 0;
	dev.Context()->IASetInputLayout(*_depthViewInputLayout.lock());
	dev.Context()->IASetVertexBuffers(0, 1, &pmdVertBuff, &stride, &offset);
	dev.Context()->VSSetShader(*_depthVS.lock(), nullptr, 0);//�o�l�c���f���\���p�V�F�[�_�Z�b�g
	dev.Context()->PSSetShader(*_depthPS.lock(), nullptr, 0);//PMD���f���\���p�V�F�[�_�Z�b�g
	dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	std::vector<PMDMaterial> pmdMaterials = _mesh->GetMaterials();
	for (int i = 0; i < (int)pmdMaterials.size(); i++)
	{
		dev.Context()->DrawIndexed(pmdMaterials[i].indicesNum, pmdMaterials[i].offset, 0);//�C���f�b�N�X�͏��Ԃɕ���ł�H�̂Ł��ŕ`�悵�����I�t�Z�b�g�l�𑫂�

	}
}

void 
Player::DrawBone()
{
	dev.Context()->Draw(_mesh->BoneSize() * 2, 0);
}

void 
Player::RegisterAnim(std::string motionName, VMDData* vmd)
{
	if (_animations.find(motionName) == _animations.end())//�������Ă������[�V���������[�h�A�o�^����ĂȂ�
	{
		/*VMDLoader vmdLoader;

		std::string filePath = _mesh->FolderPath() + motionName + ".vmd";*/
		_animations[motionName] = vmd;
	}
}

void
Player::SetAnimation(const char* animName)
{
	_currentVMD = _animations[animName];
	_frameNo = 0;
}

void
Player::BlendAnimation(const char* animName,float t)
{
	assert(t > 0.0f&&t < 1.0f);
}

void
Player::SetPos(const XMFLOAT3 pos)
{
	_pos = pos;
}