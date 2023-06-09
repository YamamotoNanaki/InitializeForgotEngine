#pragma once
#include "Component.h"
#include "Vector3.h"

namespace IFE
{
	class Player : public Component
	{
	private:
		float speed_ = 0.3f;
		Vector3 move_;
		Float3 oldPos_;
		Float3 pos_;
		Vector2 moveVec_;
	public:
		void Initialize()override;
		void Update()override;
		void OnColliderHit(Collider* collider)override;

#ifdef _DEBUG
		void ComponentDebugGUI()override;
		void OutputComponent(nlohmann::json& json)override;
#endif
		void LoadingComponent(nlohmann::json& json)override;

	private:
		void Move();
		void Rota();
		void CameraFollow();
		void EnemyCollide();
		void Shoot();
	};
}
