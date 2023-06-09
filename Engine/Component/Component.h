#pragma once
#include <list>
#include <string>
#include <typeinfo>
#include "JsonManager.h"

namespace IFE
{
	class Object3D;
	class Sprite;
	class Transform;
	class Transform2D;
	class TransformParticle;
	class ComponentManager;
	class Collider;
	class Component
	{
	public:
		Object3D* objectPtr_ = nullptr;
		Sprite* spritePtr_ = nullptr;
		std::string componentName_;
		bool componentDeleteFlag_ = false;
	protected:
		virtual void Initialize() {};
	public:
		Transform* transform_ = nullptr;
		TransformParticle* transformParticle_ = nullptr;
		Transform2D* transform2D_ = nullptr;
		void INITIALIZE();
		virtual void Update() {};
		virtual void Draw() {};
		virtual ~Component();
		Component(ComponentManager* component);
		Component() {}
		void SetTransform();
		std::string GetComponentName();
		void SetComponentName(const std::string& componentName);
		void SetComponents(ComponentManager* cm);
		//void SetComponents(Particle* components);
		//void SetComponents(Emitter* components);
		bool GetComponentDeleteFlag();
		Object3D* GetObjectPtr();
		Sprite* GetSpritePtr();
		//virtual void OnColliderHit(ADXCollider* myCol, ADXCollider* col);
		virtual void Copy(Component* component);
		virtual void OnColliderHit(Collider* collider);
		void CopyValue(Component* component);
		virtual void CopyValueComponent(Component* component);
#ifdef _DEBUG
		virtual void DebugGUI();
		virtual void ComponentDebugGUI() {};
		void OutputScene(nlohmann::json& json);
		virtual void OutputComponent(nlohmann::json& json);
#endif
		void LoadingScene(nlohmann::json& json);
		virtual void LoadingComponent(nlohmann::json& json);
	};
}
