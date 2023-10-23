#include "Lantern.h"
#include "Object3D.h"
#include "Material.h"
#include "LightManager.h"
#include "Transform.h"

void IFE::Lantern::Initialize()
{
	if (objectPtr_->GetComponent<Material>())
	{
		objectPtr_->GetComponent<Material>()->bloom_ = true;
	}
	if (!useLightFlag_)
	{
		if (lightNumOffset_ + lightNum_ < LightManager::Instance()->s_PLIGHT_NUM)
		{
			useLightNum_ = lightNumOffset_ + lightNum_;
			useLightFlag_ = true;
			lightNum_++;
			LightManager::Instance()->SetPointLightAtten(useLightNum_, { 0.0025f,0.025f,0.0025f });
			LightManager::Instance()->SetPointLightColor(useLightNum_, { 1,0.55f,0 });
			LightManager::Instance()->SetPointLightActive(useLightNum_, true);
			LightManager::Instance()->SetPointLightPos(useLightNum_, transform_->position_);
		}
	}
}

void IFE::Lantern::Update()
{
	LightManager::Instance()->SetPointLightPos(useLightNum_, transform_->position_);
}
