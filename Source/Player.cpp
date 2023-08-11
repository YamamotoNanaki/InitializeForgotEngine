#include "Player.h"
#include "Input.h"
#include "Transform.h"
#include "Object3D.h"
#include "Material.h"
#include "CameraManager.h"
#include "ObjectManager.h"
#include "Collider.h"
#include "Enemy.h"
#include "Bullet.h"
#include "BulletParticle.h"
#include "IFMath.h"
#include "IFETime.h"
#include "FireworkChrysanthemum.h"
#include "Sound.h"
#include "TransferGeometryBuffer.h"
#include "Ease.h"
#include "Scene.h"
#include "PlayerCamera.h"
#include "StartCamera.h"
#include <cmath>

using namespace IFE;

void IFE::Player::Initialize()
{
	moveVec_ = Float2{ 0,-1 };
	Enemy::SetPlayerTransform(transform_);
	pos_ = transform_->position_;
	oldPos_ = pos_;
	pm_ = ParticleManager::Instance();
	FireworkChrysanthemum::InitDefaultVelocity();
	auto num = Sound::Instance()->LoadWave("main");
	Sound::Instance()->SetVolume(num, 25);
	sMoveFlag_ = false;
	PlayerCamera::sPlayerPtr_ = this;
	StartCamera::sPlayerPtr_ = this;
	playerCamera_ = CameraManager::Instance()->GetCamera("playerCamera");
}

void IFE::Player::Update()
{
	if (!soundFlag_)
	{
		Sound::Instance()->SoundPlay("main");
		soundFlag_ = true;
	}

	if (hp_ <= 0)
	{
		Death();
		return;
	}
	if (sMoveFlag_)
	{
		Move();
		Rota();
		Shoot();
	}
	Gravity();
	objectPtr_->GetComponent<Material>()->color_ = { 1,1,1,1 };
	transform_->position_ = pos_;

	if (invincible_)
	{
		invincibleTimer_ += IFETime::sDeltaTime_;
		if (int32_t(invincibleTimer_ * 10) % 6 < 3)objectPtr_->GetComponent<Material>()->color_ = { 0.6f,0.6f,0.6f,1 };
		else objectPtr_->GetComponent<Material>()->color_ = { 1,0,0,1 };
		if (invincibleTimer_ > invincibleMaxTime)
		{
			invincible_ = false;
			invincibleTimer_ = 0;
		}
	}
}

void IFE::Player::OnColliderHit(Collider* collider)
{
	if (collider->GetObjectPtr()->GetComponent<Enemy>())
	{
		EnemyCollide();
	}
	return;
}

void IFE::Player::Move()
{
	pos_ = transform_->position_;
	oldPos_ = pos_;
	Input* input = Input::Instance();
	float x = ((float)input->KeyDown(Key_Rigth) - (float)input->KeyDown(Key_Left));
	float z = ((float)input->KeyDown(Key_Up) - (float)input->KeyDown(Key_Down));
	if (input->GetPadConnected())
	{
		x = input->GetLXAnalog(4000);
		z = input->GetLYAnalog(4000);
	}
	if(!playerCamera_) playerCamera_ = CameraManager::Instance()->GetCamera("playerCamera");
	float rota = ConvertToRadians(playerCamera_->transform_->eulerAngleDegrees_.y);
	Vector3 cameraF;
	cameraF.Set({ sinf(rota),0,cosf(rota) }, { 0,0,0 });
	cameraF.Normalize();
	Vector3 cameraR = cameraF.Cross({ 0,1,0 });
	move_ = { x, 0, z };
	move_.Normalize();
	move_ *= speed_ * IFETime::sDeltaTime_;
	pos_ -= cameraF * move_.z;
	pos_ += cameraR * move_.x;
	pos_.z = max(-90, min(pos_.z, 212));
}

void IFE::Player::Rota()
{
	Vector2 vec = Vector2{ oldPos_.x - pos_.x,oldPos_.z - pos_.z };
	if (vec == Vector2::zero)return;
	moveVec_ = vec;
	moveVec_.Normalize();
	transform_->eulerAngleDegrees_ = { 0,-ConvertToDegrees(atan2(moveVec_.y , moveVec_.x)) + 90,0 };
}

void IFE::Player::EnemyCollide()
{
	if (!invincible_)
	{
		hp_--;
		invincible_ = true;
		invincibleTimer_ = 0;
	}
}

void IFE::Player::Shoot()
{
	static float timer = 0;
	timer += IFETime::sDeltaTime_;
	if (timer > nextBulletTime_)
	{
		static Input* input = Input::Instance();
		if (input->GetPadConnected())
		{
			if (!input->PadDown(PADCODE::RSHOULDER))return;
		}
		else
		{
			if (!input->KeyDown(Key_Space))return;
		}
		Float3 pos = { pos_.x - moveVec_.x * 2,pos_.y + transform_->scale_.y / 2,pos_.z - moveVec_.y * 2 };
		auto bullet = ObjectManager::Instance()->Instantiate("Bullet", pos);
		if (bullet == nullptr)return;
		bullet->GetComponent<Bullet>()->SetMoveVector(-moveVec_);
		auto emitter = pm_->Instantiate("Bullet");
		if (emitter)
		{
			emitter->GetComponent<BulletParticle>()->GetBullet(bullet);
			emitter->isActive_ = true;
		}
		timer = 0;
	}
}

void IFE::Player::Death()
{
	deathTimer_ += IFETime::sDeltaTime_;
	auto tgb = objectPtr_->GetComponent<TransferGeometryBuffer>();
	tgb->explosion_ = deathTimer_ * 150;
	tgb->rotation_ = deathTimer_ * 25;
	tgb->gravity_ = deathTimer_ * 35;
	tgb->polygonSize_ = Lerp(5, 0, maxDeathTime_, deathTimer_);
	objectPtr_->GetComponent<Material>()->color_.z = Lerp(1, 0, maxDeathTime_, deathTimer_);
	if (deathTimer_ > maxDeathTime_)
	{
		Scene::Instance()->SetNextScene("over");
		objectPtr_->Destroy();
	}
}

void IFE::Player::Gravity()
{
	if (objectPtr_->GetComponent<Collider>()->onGround_)
	{
		static Input* input = Input::Instance();
		gravity_ = 0;
		if (sMoveFlag_ && (input->KeyTriggere(Key_Z) || input->PadTriggere(PADCODE::ABXY)))
		{
			gravity_ = -1;
		}
		else return;
	}
	gravity_ += 4.9f * IFETime::sDeltaTime_;
	pos_.y -= gravity_;
}

//void IFE::Player::Start()
//{
//	Input* input = Input::Instance();
//	if (input->GetPadConnected())
//	{
//		if (input->PadDown(PADCODE::ALL))
//		{
//			sMoveFlag_ = true;
//		}
//	}
//	else
//	{
//		if (input->KeyDown(Key_Space))
//		{
//			sMoveFlag_ = true;
//		}
//	}
//}


#ifdef NDEBUG
#else
#include "ImguiManager.h"
void IFE::Player::ComponentDebugGUI()
{
	std::string s = "HP : " + std::to_string(hp_);
	ImguiManager::Instance()->TextGUI(s);
	ImguiManager::Instance()->DragFloatGUI(&speed_, "speed", 0.05f);
	ImguiManager::Instance()->DragFloatGUI(&nextBulletTime_, "nextBulletTime", 0.05f);
}

void IFE::Player::OutputComponent(nlohmann::json& json)
{
	json["speed"] = speed_;
	json["nextBulletTime"] = nextBulletTime_;
}
#endif

void IFE::Player::LoadingComponent(nlohmann::json& json)
{
	speed_ = json["speed"];
	nextBulletTime_ = json["nextBulletTime"];
}