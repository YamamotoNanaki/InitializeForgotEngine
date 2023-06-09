#include "Component.h"
#include "Transform.h"
#include "Object3D.h"
#include "Sprite.h"
#include "Collider.h"
//#include "Particle.h"

void IFE::Component::SetTransform()
{
	if (objectPtr_ != nullptr)transform_ = objectPtr_->GetComponent<Transform>();
	else if (spritePtr_ != nullptr)transform2D_ = spritePtr_->GetComponent<Transform2D>();
}

std::string IFE::Component::GetComponentName()
{
	return componentName_;
}

void IFE::Component::INITIALIZE()
{
	if (objectPtr_ != nullptr)transform_ = objectPtr_->GetComponent<Transform>();
	else if (spritePtr_ != nullptr)transform2D_ = spritePtr_->GetComponent<Transform2D>();
	Initialize();
}

IFE::Component::~Component()
{
}

IFE::Component::Component(ComponentManager* c)
{
	if (c->objectPtr_ != nullptr)
	{
		objectPtr_ = c->objectPtr_;
		transform_ = objectPtr_->GetComponent<Transform>();
	}
	else if (c->spritePtr_ != nullptr)
	{
		spritePtr_ = c->spritePtr_;
		transform2D_ = spritePtr_->GetComponent<Transform2D>();
	}
	//particlePtr = p;
	//transformParticle = particlePtr->GetComponent<TransformParticle>();
	//emitterPtr = e;
	//transformParticle = emitterPtr->GetComponent<TransformParticle>();
}

void IFE::Component::SetComponentName(const std::string& n)
{
	componentName_ = n;
}

void IFE::Component::SetComponents(ComponentManager* cm)
{
	if (cm->objectPtr_ != nullptr)
	{
		objectPtr_ = cm->objectPtr_;
		transform_ = objectPtr_->GetComponent<Transform>();
	}
	else if (cm->spritePtr_ != nullptr)
	{
		spritePtr_ = cm->spritePtr_;
		transform2D_ = spritePtr_->GetComponent<Transform2D>();
	}
	//particlePtr = p;
	//transformParticle = particlePtr->GetComponent<TransformParticle>();
	//emitterPtr = e;
	//transformParticle = emitterPtr->GetComponent<TransformParticle>();
}

bool IFE::Component::GetComponentDeleteFlag()
{
	return componentDeleteFlag_;
}

IFE::Object3D* IFE::Component::GetObjectPtr()
{
	return objectPtr_;
}

IFE::Sprite* IFE::Component::GetSpritePtr()
{
	return spritePtr_;
}

//void IFE::Component::OnColliderHit(ADXCollider* myCol, ADXCollider* col)
//{
//	myCol; col;
//}

void IFE::Component::Copy(Component* c) { c; }

void IFE::Component::OnColliderHit(Collider* collider)
{
	collider;
}

void IFE::Component::CopyValue(Component* component)
{
	component->componentName_ = componentName_;
	Copy(component);
}

void IFE::Component::CopyValueComponent(Component* component) { component; }

#ifdef _DEBUG
#include "ImguiManager.h"
void IFE::Component::DebugGUI()
{
	std::function<void(void)>func = [&]()
	{
		ImguiManager::Instance()->TextGUI(componentName_.c_str());
		ComponentDebugGUI();
	};
	std::function<void(void)>deleteFunc = [&]()
	{
		componentDeleteFlag_ = true;
	};
	ImguiManager::Instance()->ComponentGUI(func, deleteFunc, componentName_.c_str());
}

void IFE::Component::OutputScene(nlohmann::json& j)
{
	OutputComponent(j[componentName_]);
}
void IFE::Component::OutputComponent(nlohmann::json& j)
{
	j;
}
#endif

void IFE::Component::LoadingScene(nlohmann::json& j)
{
	LoadingComponent(j);
}

void IFE::Component::LoadingComponent(nlohmann::json& j)
{
	j;
}
