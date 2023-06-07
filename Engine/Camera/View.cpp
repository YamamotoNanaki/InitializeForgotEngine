#include "View.h"
#include <cassert>

using namespace IFE;

Matrix View::matBillBoard = MatrixIdentity();
Matrix View::matBillBoardY = MatrixIdentity();

View::View(Float3 eye, Float3 target, Float3 up)
{
	Initialze(eye, target, up);

}

void View::Initialze(Float3 e, Float3 t, Float3 u)
{
	this->eye = e;
	this->target = t;
	this->up = u;
	Update();
}

void View::Update()
{
	//���_���W�A�����_���W�A�����
	Vector3 eyePosition = SetVector3(eye);
	Vector3 targetPosition = SetVector3(target);
	Vector3 upVector = SetVector3(up);

	//�J����Z��
	Vector3 cameraAxisZ = VectorSubtract(targetPosition, eyePosition);
	//�[���x�N�g�������O
	assert(!Vector3Equal(cameraAxisZ, { 0,0,0 }));
	assert(!Vector3Equal(upVector, { 0,0,0 }));
	//�x�N�g�����K��
	cameraAxisZ = Vector3Normalize(cameraAxisZ);

	//�J����X��Y��
	Vector3 cameraAxisX;
	cameraAxisX = Vector3Cross(upVector, cameraAxisZ);
	cameraAxisX = Vector3Normalize(cameraAxisX);
	Vector3 cameraAxisY;
	cameraAxisY = Vector3Cross(cameraAxisZ, cameraAxisX);
	cameraAxisY = Vector3Normalize(cameraAxisY);

	//��]�s��̍쐬
	Matrix matCameraRot;
	matCameraRot.SetX(cameraAxisX);
	matCameraRot.SetY(cameraAxisY);
	matCameraRot.SetZ(cameraAxisZ);
	matCameraRot.SetW(0, 0, 0, 1);

	//�t�s�����
	matView = MatrixTranspose(matCameraRot);

	//�J�����̈ʒu���猴�_�ւ̃x�N�g���𐶐�
	Vector3 reverseEyePosition = VectorNegate(eyePosition);
	matView.m[3][0] = Vector3Dot(cameraAxisX, reverseEyePosition);
	matView.m[3][1] = Vector3Dot(cameraAxisY, reverseEyePosition);
	matView.m[3][2] = Vector3Dot(cameraAxisZ, reverseEyePosition);
	matView.m[3][3] = 1.0f;

	matBillBoard.SetX(cameraAxisX);
	matBillBoard.SetY(cameraAxisY);
	matBillBoard.SetZ(cameraAxisZ);
	matBillBoard.SetW(0, 0, 0, 1);

	//Y���r���{�[�h
	Vector3 yBillAxisX, yBillAxisY, yBillAxisZ;

	yBillAxisX = cameraAxisX;
	yBillAxisY = Vector3Normalize(upVector);
	yBillAxisZ = Vector3Cross(yBillAxisX, yBillAxisY);

	matBillBoardY.SetX(yBillAxisX);
	matBillBoardY.SetY(yBillAxisY);
	matBillBoardY.SetZ(yBillAxisZ);
	matBillBoardY.SetW(0, 0, 0, 1);
}

Matrix View::Get() const
{
	return matView;
}

Matrix* IFE::View::GetAddressOf()
{
	return &matView;
}