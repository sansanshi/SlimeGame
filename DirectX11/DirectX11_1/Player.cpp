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



//頂点レイアウト
//５番目のパラメータは先頭からのバイト数なので，COLORにはPOSITIONのfloat型4バイト×3を記述
D3D11_INPUT_ELEMENT_DESC inputElementDescs[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//↓シェーダ側に16ビット整数型が無いのでR16G16だと処理できない　
	//32ビット整数型で渡してシェーダ側でビット演算する
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
	//↓シェーダ側に16ビット整数型が無いのでR16G16だと処理できない　
	//32ビット整数型で渡してシェーダ側でビット演算する
	{ "BONE_ID", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "BONE_WEIGHT", 0, DXGI_FORMAT_R8_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "EDGE", 0, DXGI_FORMAT_R8_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

//std::fill(array.begin(),array.end(),XMMatrixIdentity());


//頂点インデックスレイアウト
D3D11_INPUT_ELEMENT_DESC indexInputElementDesc[]{
	{"INDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

//特定の方向を向かせる行列を返す関数
//	（Zベクトルを特定の方向に向かせる）
//@param lookat 向かせたい方向ベクトル
//@param right 右ベクトル
XMMATRIX LookAtMatrix(XMFLOAT3& lookat,XMFLOAT3& upper, XMFLOAT3& right)
{
	//向かせたい方向が(1,0,0)とかだとゼロベクトルになるかもしれないのでupベクトルも用意しといて
	//どっちかを使う　とかする

	//向かせたい方向
	XMVECTOR vz = XMVector3Normalize(XMLoadFloat3(&lookat));
	//仮のY軸
	XMVECTOR vy = XMVector3Normalize(XMLoadFloat3(&upper));
	XMVECTOR vx;

	float dot= XMVector3Dot(XMLoadFloat3(&XMFLOAT3(0, 2, 0)), XMLoadFloat3(&XMFLOAT3(0, 1, 0))).m128_f32[0];

	//向かせたい方向とy軸が同じ方向を向いてたらright基準で計算し直す
	if (XMVector3Dot(vy, vz).m128_f32[0] >= 1.0f)
	{
		vx = XMVector3Normalize(XMLoadFloat3(&right));
		vy = XMVector3Normalize(XMVector3Cross(vz, vx));
		vx = XMVector3Normalize(XMVector3Cross(vy, vz));
	}
	else
	{
		//x軸を求める
		vx = XMVector3Normalize(XMVector3Cross(vy, vz));
		//仮のY軸ではないY軸を求める
		vy = XMVector3Normalize(XMVector3Cross(vz, vx));
	}

	XMFLOAT3 v1 = { 0, 1, 0.5f };
	XMFLOAT3 v2 = { 0, 1, 1 };
	float f = XMVector3Dot(XMLoadFloat3(&v1), XMLoadFloat3(&v2)).m128_f32[0];
	
	int lk = 0;

	////仮のx軸
	//XMVECTOR vx = XMVector3Normalize(XMLoadFloat3(&right));
	////向かせたい方向を向かせた時のY軸
	//XMVECTOR vy = XMVector3Normalize(XMVector3Cross(vz,vx));

	////仮のx軸ではなく向かせたい方向を向かせた時のx軸を求める
	//vx = XMVector3Normalize(XMVector3Cross(vy, vz));

	XMMATRIX ret = XMMatrixIdentity();
	XMFLOAT3 fx, fy, fz;
	XMStoreFloat3(&fx, vx);
	XMStoreFloat3(&fy, vy);
	XMStoreFloat3(&fz, vz);

	//directXが左手系に対してPMDファイルは右手系で作られているのでZ値がお尻の方に向かって+
	//directXとは座標系が違うのでｘとｚを反転させる
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

//あるベクトルを別のベクトル方向に向かせる行列を返す
//origin　元のベクトル　lookat　向かせたいベクトル
XMMATRIX LookAtMatrix(XMFLOAT3& origin,XMFLOAT3& lookat ,XMFLOAT3& up, XMFLOAT3& right)
{
	XMMATRIX tmp = LookAtMatrix(origin, up, right);
	//回転成分しか無いので転置すると逆行列になる
	tmp = XMMatrixMultiply(XMMatrixTranspose(tmp),LookAtMatrix(lookat, up, right));
	return tmp;
}

//キャラが特定のベクトルを向くようにする
//@param xベクトル
//@param yベクトル
//@param zベクトル
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
	//16バイトアライメントの関係で一度ローカル変数に入れてる
	XMMATRIX bone = boneMatrixies[idx];
	bone = XMMatrixMultiply(bone, parent);
	boneMatrixies[idx] = bone;

	//自分のsibling（兄弟）に親行列を渡す
	while (boneTree[idx].sibling != 0xffff)
	{
		MatrixTransmission(boneTree[idx].sibling, parent, boneMatrixies, boneTree, bones);
		break;
	}
	//自分の子に自分の行列を渡す
	while (boneTree[idx].child != 0xffff)
	{
		MatrixTransmission(boneTree[idx].child, bone, boneMatrixies, boneTree, bones);
		break;
	}

}



//CCD_IKを使ってボーン位置を解決する
//ボーンとIKList
//mesh　対象メッシュ　（必要なのはボーンとIKList
//ikName　解決したいIKボーンの名前
//offset そのIKボーンが元の位置からどれくらい離れたか
//　※普通のボーンは回転しか出来ないがIKボーンは移動ができる

void
Player::CcdIkSolve(PMDMesh& mesh,const std::string& ikName,XMFLOAT3& offset)
{

	//元座標から動いてなかったら解決しない
	if (offset == XMFLOAT3(0, 0, 0))
	{
		return;
	}
	std::map<std::string, PMDMesh::IKList>& ikmap = mesh.IKListMap();
	std::map<std::string, int>& bonemap = mesh.GetBoneMap();



	if (ikmap.find(ikName) == ikmap.end() || bonemap.find(ikName) == bonemap.end()){
		return;//見つからなかったら処理しない
	}

	PMDMesh::IKList& iklist = ikmap[ikName];
	int boneIdx = bonemap[ikName];

	//対象ボーンの取得
	//PMDMesh::Bone& bone = mesh.Bones()[boneIdx];
	BoneVert& bone = mesh.GetBoneVertices()[boneIdx];

	//右足IKボーンがある位置
	XMFLOAT3 ikOriginPos = bone.head.pos;
	//目標位置
	XMFLOAT3 ikTargetPos = bone.head.pos + offset;

	//IK位置にマーカー表示するのでこれが要る
	_ikpos = ikTargetPos;

	//まずはIKの間にあるボーンの座標の一時変数配列を作る
	//理由はIK再帰する毎に、ボーン座標が変更されるからである
	std::vector<XMFLOAT3> tmpBonePositions(iklist.ikchainLen);
	for (int i = 0; i < iklist.ikchainLen; ++i)
	{
		//tmpBonePositions[i] = mesh.Bones()[iklist.ikboneIndices[i]].headPos;
		tmpBonePositions[i] = mesh.GetBoneVertices()[iklist.ikboneIndices[i]].head.pos;
	}

	//ボーンの根っこ部分（IKから最も遠いボーン）からIKへのベクトル
	XMFLOAT3 ikOriginRootVec = ikOriginPos - tmpBonePositions[iklist.ikchainLen - 1];
	
	//ボーンの根っこ部分（IKから最も遠いボーン）から変更後IKへのベクトル
	XMFLOAT3 ikTargetRootVec = ikTargetPos - tmpBonePositions[iklist.ikchainLen - 1];

	//本来の長さ以上になろうとしたらクランプする------------------------------------------
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

	//IK移動前から移動後への回転ベクトルを計算しておく
	XMMATRIX matIkRot = LookAtMatrix(ikOriginRootVec, ikTargetRootVec,XMFLOAT3(0,1,0),XMFLOAT3(1,0,0));

	XMFLOAT3 rootAxis = Cross(Normalize(ikOriginRootVec), Normalize(ikTargetRootVec));
	float rootangle = XMVector3AngleBetweenNormals(XMLoadFloat3(&ikOriginRootVec), XMLoadFloat3(&ikTargetRootVec)).m128_f32[0];
	ikOriginRootVec = Normalize(ikOriginRootVec);
	ikTargetRootVec = Normalize(ikTargetRootVec);
	matIkRot = XMMatrixRotationAxis(XMLoadFloat3(&rootAxis), rootangle);

	matIkRot = LookAtMatrix(ikOriginRootVec, ikTargetRootVec, XMFLOAT3(0, 1, 0), XMFLOAT3(1, 0, 0));

	//ここからCCD_IK
	//ほんとはここでサイクリック（繰り返す）するが、まず1回目の事だけ考える
	for (int c = 0; c < iklist.iterations; ++c)
	{

		matIkRot = LookAtMatrix(ikOriginRootVec, ikTargetRootVec, XMFLOAT3(0, 1, 0), XMFLOAT3(-1, 0, 0));
		
		//中間ボーン座標補正
		for (int i = 0; i < iklist.ikchainLen; ++i)
		{

			int ikboneIdx = iklist.ikboneIndices[i];
			XMFLOAT3 originVec = ikOriginPos - tmpBonePositions[i];
			XMFLOAT3 targetVec = ikTargetPos - tmpBonePositions[i];

			//それぞれのベクトル長が小さすぎる場合は処理を打ち切る
			if (Length(originVec) < 0.0001f || Length(targetVec) < 0.0001f){
				return;
			}

			//正規化する
			originVec = Normalize(originVec);
			targetVec = Normalize(targetVec);


			//外積から軸を作成する
			XMFLOAT3 axis = Normalize(Cross(originVec, targetVec));


			//matIkRot = LookAtMatrix(originVec, targetVec, XMFLOAT3(0, 1, 0), XMFLOAT3(-1, 0, 0));


			//もしひざ系なら、X軸を回転軸とする
			if (mesh.GetBoneVertices()[ikboneIdx].name.find("ひざ") != std::string::npos)//if (bone.name.find("ひざ") != std::string::npos)
			{
				axis.x = -1.0f;
				axis.y = 0.0f;
				axis.z = 0.0f;
				//その軸をmatIkRot
				XMVECTOR tempvec = XMLoadFloat3(&axis);
				tempvec = XMVector3Transform(tempvec, matIkRot);
				XMStoreFloat3(&axis, tempvec);
				int kj = 0;
			}
			else
			{
				if (Length(axis) == 0.0f)
				{
					return;//外積結果が0になってるならリターン
				}
			}
			//二つのベクトルの間の角度を計算
			float angle = XMVector3AngleBetweenNormals(XMLoadFloat3(&originVec), XMLoadFloat3(&targetVec)).m128_f32[0];
			if (angle < 0.0001f)return;

			//制限角度を計算
			float strict = (iklist.limitangle * 4)/(float)iklist.ikchainLen*(float)(i+1);//iklist.limitangle*4;//

			if (angle > strict)
			{
				int jk = 0;
			}
			angle = min(angle,strict);
			angle = max(angle, -strict);
			angle *= 0.5f;//半分にしてる意味はよく分からない


			//ボーン変換行列を計算
			XMMATRIX rotMat = XMMatrixRotationAxis(XMLoadFloat3(&axis), angle);
			//XMVECTOR q = XMQuaternionRotationAxis()

			//rotMatのテスト
			//originVecをrotMatで回転させるとtargetVecになるはず
			XMVECTOR vecTest = XMVector3Transform(XMLoadFloat3(&originVec),rotMat);
			originVec;
			targetVec;
			int jkjk = 0;
			//rotMatはok

			//オフセットを考慮した行列を作る（原点に移動→回転→元の座標分移動
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

			//変換行列を計算（オフセットを考慮
			mesh.BoneMatrixies()[iklist.ikboneIndices[i]] = mesh.BoneMatrixies()[ikboneIdx] * mat;



			XMFLOAT3& tmpbonePosition = tmpBonePositions[i];
			mat = XMMatrixTranslation(-tmpbonePosition.x, -tmpbonePosition.y, -tmpbonePosition.z)*
				rotMat*
				XMMatrixTranslation(tmpbonePosition.x, tmpbonePosition.y, tmpbonePosition.z);


			

			//IKボーンの位置に↑の行列かけてやる
			ikOriginPos = ikOriginPos*mat;
			//自分より末端側のボーン位置に対して自分の変換行列をかけてやる
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
	//ボーン行列の初期化
	std::fill(mesh->BoneMatrixies().begin(), mesh->BoneMatrixies().end(), XMMatrixIdentity());


	for (auto& frames : vmddata->GetKeyframes())
	{
		
		//逆イテレータ
		std::vector<VMDData::MotionData>::reverse_iterator revit = std::find_if(frames.second.rbegin(), frames.second.rend(), 
			[frameNo](const VMDData::MotionData& md){
			return md.frameNo <= frameNo;
		});
		//ここ抜けた時点で20 10 0の順に読んで0じゃなく10のitが帰ってる
		//if (revit == frames.second.rbegin()) revit = revit + 1;
		if (revit == frames.second.rend()) continue;//見つからなかったら

		

		auto r_it = frames.second.rbegin();
		auto r = *r_it;
		auto r_base = r_it.base();
		auto _it = frames.second.begin();
		
		

		//iteratorは特定の要素を指し示すときに使われる
		//今回使っているfind_ifなの「要素を返す関数」もiteratorを返す
		//it++ or ++it は指し示す要素を次の要素にする
		//（EffectiveC++では++itを使えと書かれている
		//×for(;it<vec.end();)　○it!=vec.end()

		//リバースイテレータ
		//逆向きイテレータ
		//begin()～end()がrbegin()～rend()に対応
		//begin()はコンテナの最初の要素の鼻先を示す
		//rbegin()はコンテナの最後の要素を示す
		//end()はコンテナの最後の要素のお尻を示す（最後の要素を指し示しているわけではない
		//rbegin()はコンテナの最後の要素のお尻なんだけど、逆方向に数えるので、end()と違い最後のコンテナを指し示す
		//リバースイテレータからイテレータへの変換
		//it.base()
		//という関数を使うと尻尾見てるやつがその場で振り返って鼻先から数え始める

		//対応するボーンが無かったら処理しないようにする
		if (mesh->GetBoneMap().count(frames.first) == 0) continue;
		int idx = mesh->GetBoneMap()[frames.first];

		auto it = revit.base();
		float t = 0.0f;
		XMVECTOR v;
		if (it != frames.second.end())
		{
			t = (float)(frameNo - revit->frameNo) / (float)(it->frameNo - revit->frameNo);//この辺
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

	//pmxはデフォルトでutf16（Unicode）を使うのでL付けてワイド文字列にする
	//PMXLoader pmxloader;
	//pmxloader.LoadPMX(L"models/shame/shame.pmx");

	//pmdファイルの読み込み
	PMDLoader loader;
	//_mesh = loader.Load("models/kouhai/kouhai_v1.pmd");
	_mesh = loader.Load("models/kouhai/kouhai_v1.pmd");

	//vmdロードのフレーム数の大小を比較して一番大きいやつ（最終フレーム）を保持しとく
	//maxframe=std::max(frame, maxframe);
	//vmd側にリピートフラグを持たせる
	//VMDファイルの読み込み
	VMDLoader vmdLoader;
	_vmd = vmdLoader.Load("neutral.vmd",true);
	RegisterAnim("charge", _vmd);
	VMDData* run = vmdLoader.Load("run.vmd",true);
	RegisterAnim("run", run);
	VMDData* neutral = vmdLoader.Load("neutral.vmd", true);
	RegisterAnim("neutral", neutral);


	_currentVMD = _animations["charge"];//_vmd の部分を_currentVMDに変える

	unsigned int stride = _mesh->GetVertexStride();
	unsigned int offset = 0;
	ID3D11Buffer* pmdVertBuff = _mesh->GetVertexBuffer();
	dev.Context()->IASetVertexBuffers(0, 1, &pmdVertBuff, &stride, &offset);


	dev.Context()->IASetIndexBuffer(_mesh->GetIndexBuffer(), DXGI_FORMAT_R16_UINT, 0);

	dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//シェーダコンパイル、ロード
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

	
	dev.Context()->VSSetShader(*_vertexShader.lock(), nullptr, 0);//ＰＭＤモデル表示用シェーダセット
	dev.Context()->PSSetShader(*_pixelShader.lock(), nullptr, 0);//PMDモデル表示用シェーダセット

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


	


	//mvp行列用のバッファ作る
	//
	D3D11_BUFFER_DESC matBuffDesc = {};
	matBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matBuffDesc.ByteWidth = sizeof(WorldAndCamera);
	matBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;//バッファの中身はCPUで書き換える
	matBuffDesc.Usage = D3D11_USAGE_DYNAMIC;//CPUによる書き込み、GPUによる読み込みが行われるという意味
	//matBuffDesc.ByteWidth = sizeof(XMMATRIX);
	//matBuffDesc.StructureByteStride = sizeof(XMMATRIX);
	
	D3D11_SUBRESOURCE_DATA d;
	d.pSysMem = &_worldAndCamera;

	result = dev.Device()->CreateBuffer(&matBuffDesc, &d, &_matrixBuffer);

	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
	
	dev.Context()->Unmap(_matrixBuffer, 0);

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);


	//シェーダに渡す用のマテリアル構造体
	_material = {};

	D3D11_BUFFER_DESC materialBuffDesc = {};
	materialBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//コンスタントバッファのByteWidthは16の倍数でないといけない
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


	//シェーダに渡すボーン行列
	
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

	
	//LookAt行列のテスト
	XMMATRIX lookMatrix = LookAtMatrix(XMFLOAT3(0, 1, 0), XMFLOAT3(0,1,0), XMFLOAT3(1, 0, 0));
	XMVECTOR test = { 0, 0, 1, 0 };

	test = XMVector3Transform(test, lookMatrix);
	XMFLOAT3 ftest;
	XMStoreFloat3(&ftest,test);
	int jj = 0;


	//ボーンのツリー構造を作る
	_boneTree.resize(_mesh->BoneSize());
	//ツリー初期化
	for (auto& node : _boneTree)
	{
		memset((void*)(&node.child), 0xffff, sizeof(unsigned short));
		memset((void*)(&node.sibling), 0xffff, sizeof(unsigned short));
	}


	std::vector<BONE> pmdBones = _mesh->GetBones();
	//ツリー探索
	for (unsigned int i = 0; i <_boneTree.size(); i++)
	{
		unsigned short parentIdx = pmdBones[i].parentBoneIdx;
		//親が居ないなら処理しない
		if (parentIdx == 0xffff) continue;
		//まず親ノードを決める　ポインタ型なのでここで弄れば元の方にも反映される
		BoneNode& parentNode = _boneTree[parentIdx];
		//子がいなければ子をつくる
		if (parentNode.child == 0xffff)
		{
			parentNode.child = i;
			continue;
		}
		unsigned int idx = parentNode.child;
		//子を既に１つ以上持っていたら、子のsibling（兄弟）に設定する
		//子も既にsiblingを持っていたらそのsiblingのsiblingに・・・という感じでやっていく
		while (_boneTree[idx].sibling != 0xffff)
		{
			idx = _boneTree[idx].sibling;
		}
		_boneTree[idx].sibling = i;
	}



	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	
	


	//サンプラの設定
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


	//ボーンの変換行列を子のボーンに伝播
	MatrixTransmission(0, XMMatrixIdentity(), _mesh->BoneMatrixies(), _boneTree, _mesh->GetBones());


	//LookAtテスト
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
	


	//LookAtのテスト
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
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
	
	dev.Context()->Unmap(_matrixBuffer, 0);

	//dev.Context()->VSSetConstantBuffers(0, 1, &matrixBuffer);

	//マップされたボーンバッファに書き込み
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
		dev.Context()->VSSetShader(*_boneVertexShader.lock(), nullptr, 0);//ボーン表示用シェーダセット
		dev.Context()->PSSetShader(*_bonePixelShader.lock(), nullptr, 0);//ボーン表示用シェーダセット
		dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}
	else
	{
		unsigned int stride = _mesh->GetVertexStride();
		unsigned int offset = 0;

		dev.Context()->IASetInputLayout(*_vertInputLayout.lock());
		dev.Context()->IASetVertexBuffers(0, 1, &pmdVertBuff, &stride, &offset);
		dev.Context()->VSSetShader(*_vertexShader.lock(), nullptr, 0);//ＰＭＤモデル表示用シェーダセット
		dev.Context()->PSSetShader(*_pixelShader.lock(), nullptr, 0);//PMDモデル表示用シェーダセット		dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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

	DeformBones(_mesh, _currentVMD, _frameNo/2);// /2はモーションを遅くするために付けてるだけ
	if (_currentVMD->IsRepeat()){
		_frameNo = (_frameNo + 1) % (_currentVMD->Duration() * 2 + 2);//前の写しただけ
	}
	else{
		_frameNo = min(_frameNo + 1, _currentVMD->Duration()*2+2);
		if (_frameNo == _currentVMD->Duration() * 2 + 2) SetAnimation("charge");
	}

	std::string name = "右足ＩＫ";
	CcdIkSolve(*_mesh, name, _ikOffset);
	int id = _mesh->GetBoneMap()[name];
	XMMATRIX m = _mesh->BoneMatrixies()[69];


	//ボーンの変換行列を子のボーンに伝播
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
		dev.Context()->VSSetShader(*_vertexShader.lock(), nullptr, 0);//ＰＭＤモデル表示用シェーダセット
		dev.Context()->PSSetShader(*_pixelShader.lock(), nullptr, 0);//PMDモデル表示用シェーダセット
		dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//XMMATRIX world = XMMatrixIdentity();

		//XMMATRIX scaleMatrix = XMMatrixScaling(0.8f, 0.8f, 0.8f);
		//world = XMMatrixMultiply(scaleMatrix,world);

		rotAngle += 1 * XM_PI / 180;//とりあえず回転させておく

		//LookAtのテス
		//XMMATRIX transMatrix = XMMatrixTranslation(_pos.x, _pos.y, _pos.z);//LookAt(1, 0, 0);//
		//world = XMMatrixMultiply(transMatrix, world);


		_worldAndCamera.cameraView = _cameraPtr.lock()->CameraView();
		_worldAndCamera.cameraProj = _cameraPtr.lock()->CameraProjection();
		_worldAndCamera.lightView = _cameraPtr.lock()->LightView();
		_worldAndCamera.lightProj = _cameraPtr.lock()->LightProjection();

		dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
		//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
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


			dev.Context()->DrawIndexed(pmdMaterials[i].indicesNum, pmdMaterials[i].offset, 0);//インデックスは順番に並んでる？ので↑で描画した分オフセット値を足す

		}

		


		////影描画
		//XMMATRIX shadow = XMMatrixShadow(XMVectorSet(0, 1, 0, -0.2), XMVectorSet(-1, 1, -1, 0));
		////Map
		//XMMATRIX temp = _worldAndCamera.world;
		//temp = XMMatrixMultiply(shadow, temp);
		//_worldAndCamera.world = temp;
		//dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
		////ここでこのメモリの塊に、マトリックスの値をコピーしてやる
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
		////ここでこのメモリの塊に、マトリックスの値をコピーしてやる
		//memcpy(_mem.pData, (void*)(&_matrixMVP), sizeof(_matrixMVP));
		//dev.Context()->Unmap(_matrixBuffer, 0);
	}
	else
	{

		unsigned int stride = _mesh->GetBoneVertStride();
		unsigned int offset = 0;
		dev.Context()->IASetInputLayout(*_boneInputLayout.lock());
		dev.Context()->IASetVertexBuffers(0, 1, &boneVertBuff, &stride, &offset);
		dev.Context()->VSSetShader(*_boneVertexShader.lock(), nullptr, 0);//ボーン表示用シェーダセット
		dev.Context()->PSSetShader(*_bonePixelShader.lock(), nullptr, 0);//ボーン表示用シェーダセット
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
	dev.Context()->VSSetShader(*_depthVS.lock(), nullptr, 0);//ＰＭＤモデル表示用シェーダセット
	dev.Context()->PSSetShader(*_depthPS.lock(), nullptr, 0);//PMDモデル表示用シェーダセット
	dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	dev.Context()->VSSetConstantBuffers(0, 1, &_matrixBuffer);
	dev.Context()->IASetIndexBuffer(_mesh->GetIndexBuffer(), DXGI_FORMAT_R16_UINT, 0);

	XMMATRIX view = _cameraPtr.lock()->LightView();
	XMMATRIX proj = _cameraPtr.lock()->LightProjection();
	_worldAndCamera.lightView = view;
	_worldAndCamera.lightProj = proj;

	//バッファの更新してない↑のでする
	dev.Context()->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &_mem);
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
	
	dev.Context()->Unmap(_matrixBuffer, 0);

	std::vector<PMDMaterial> pmdMaterials = _mesh->GetMaterials();
	for (int i = 0; i < (int)pmdMaterials.size(); i++)
	{
		dev.Context()->DrawIndexed(pmdMaterials[i].indicesNum, pmdMaterials[i].offset, 0);//インデックスは順番に並んでる？ので↑で描画した分オフセット値を足す

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
	//ここでこのメモリの塊に、マトリックスの値をコピーしてやる
	memcpy(_mem.pData, (void*)(&_worldAndCamera), sizeof(_worldAndCamera));
	
	dev.Context()->Unmap(_matrixBuffer, 0);


	ID3D11Buffer* pmdVertBuff = _mesh->GetVertexBuffer();
	unsigned int stride = _mesh->GetVertexStride();
	unsigned int offset = 0;
	dev.Context()->IASetInputLayout(*_depthViewInputLayout.lock());
	dev.Context()->IASetVertexBuffers(0, 1, &pmdVertBuff, &stride, &offset);
	dev.Context()->VSSetShader(*_depthVS.lock(), nullptr, 0);//ＰＭＤモデル表示用シェーダセット
	dev.Context()->PSSetShader(*_depthPS.lock(), nullptr, 0);//PMDモデル表示用シェーダセット
	dev.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	std::vector<PMDMaterial> pmdMaterials = _mesh->GetMaterials();
	for (int i = 0; i < (int)pmdMaterials.size(); i++)
	{
		dev.Context()->DrawIndexed(pmdMaterials[i].indicesNum, pmdMaterials[i].offset, 0);//インデックスは順番に並んでる？ので↑で描画した分オフセット値を足す

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
	if (_animations.find(motionName) == _animations.end())//今持ってきたモーションがロード、登録されてない
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