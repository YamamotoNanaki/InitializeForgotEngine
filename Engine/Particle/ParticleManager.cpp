#include "ParticleManager.h"
#include "Transform.h"
#include "TextureManager.h"

using namespace IFE;
using namespace std;

ParticleManager* IFE::ParticleManager::Instance()
{
	static ParticleManager* inst = new ParticleManager;
	return inst;
}

void IFE::ParticleManager::Finalize()
{
	delete Instance();
}

void IFE::ParticleManager::Initialize()
{
	for (unique_ptr<Emitter>& itr : emitterList_)
	{
		itr->Initialize();
	}
}

void IFE::ParticleManager::Update()
{
	for (unique_ptr<Emitter>& itr : emitterList_)
	{
		itr->Update();
	}
}

void IFE::ParticleManager::Draw()
{
	//Emitter::DrawBefore();
	for (unique_ptr<Emitter>& itr : emitterList_)
	{
		itr->Draw();
	}
}

void IFE::ParticleManager::AddInitialize(const std::string& emitterName, const std::string& textureName)
{
	std::unique_ptr<Emitter> ptr = make_unique<Emitter>();
	ptr->Initialize();
	ptr->emitterName_ = emitterName;
	ptr->SetTexture(textureName);
	emitterList_.push_back(std::move(ptr));
}

Emitter* IFE::ParticleManager::Add(const std::string& emitterName)
{
	std::unique_ptr<Emitter> ptr = make_unique<Emitter>();
	ptr->emitterName_ = emitterName;
	emitterList_.push_back(std::move(ptr));
	return emitterList_.back().get();
}

Emitter* IFE::ParticleManager::GetEmitterPtr(const std::string& emitterName)
{
	for (unique_ptr<Emitter>& itr : emitterList_)
	{
		if (itr->emitterName_ == emitterName)
		{
			return itr.get();
		}
	}
	return nullptr;
}

Emitter* IFE::ParticleManager::Instantiate(Emitter* gameObject, const Float3& position, float rotation)
{
	std::unique_ptr<Emitter> ptr = make_unique<Emitter>();
	ptr->deleteFlag_ = gameObject->deleteFlag_;
	ptr->emitterDeleteFlag_ = gameObject->emitterDeleteFlag_;
	ptr->transform_ = gameObject->transform_;
	ptr->isActive_ = gameObject->isActive_;
	ptr->DrawFlag_ = gameObject->DrawFlag_;
	ptr->tex_ = gameObject->tex_;
	string objectName = GetNewName(gameObject->emitterName_);
	ptr->emitterName_ = objectName;
	emitterList_.push_back(std::move(ptr));
	Emitter* obj = emitterList_.back().get();
	obj->transform_->position_ = position;
	//obj->transform_->rotation_ = 
		rotation;
	return obj;
}

Emitter* IFE::ParticleManager::Instantiate(Emitter* gameObject, const Float3& position)
{
	position;
	std::unique_ptr<Emitter> ptr = make_unique<Emitter>();
	//*ptr = *gameObject;
	ptr->deleteFlag_ = gameObject->deleteFlag_;
	ptr->emitterDeleteFlag_ = gameObject->emitterDeleteFlag_;
	ptr->transform_ = gameObject->transform_;
	ptr->isActive_ = gameObject->isActive_;
	ptr->DrawFlag_ = gameObject->DrawFlag_;
	ptr->tex_ = gameObject->tex_;
	std::string emitterName;
	string objectName = GetNewName(gameObject->emitterName_);
	ptr->emitterName_ = objectName;
	emitterList_.push_back(std::move(ptr));
	Emitter* obj = emitterList_.back().get();
	obj->transform_->position_ = position;
	return obj;
}

Emitter* IFE::ParticleManager::Instantiate(Emitter* gameObject)
{
	std::unique_ptr<Emitter> ptr = make_unique<Emitter>();
	ptr->deleteFlag_ = gameObject->deleteFlag_;
	ptr->emitterDeleteFlag_ = gameObject->emitterDeleteFlag_;
	ptr->transform_ = gameObject->transform_;
	ptr->isActive_ = gameObject->isActive_;
	ptr->DrawFlag_ = gameObject->DrawFlag_;
	ptr->tex_ = gameObject->tex_;
	string objectName = GetNewName(gameObject->emitterName_);
	ptr->emitterName_ = objectName;
	emitterList_.push_back(std::move(ptr));
	Emitter* obj = emitterList_.back().get();
	return obj;
}

void IFE::ParticleManager::SetTexture(const std::string& emitterName, const std::string& texName)
{
	texName;
	for (unique_ptr<Emitter>& itr : emitterList_)
	{
		if (itr->emitterName_ == emitterName)
		{
			itr->SetTexture(texName);
			return;
		}
	}
}

std::list<std::unique_ptr<Emitter>>* IFE::ParticleManager::GetEmitterList()
{
	return &emitterList_;
}

void IFE::ParticleManager::Reset()
{
	emitterList_.clear();
}

bool IFE::ParticleManager::SearchName(const std::string& name)
{
	for (unique_ptr<Emitter>& itr : emitterList_)
	{
		if (itr->emitterName_ == name)return true;
	}
	return false;
}

#include "JsonManager.h"
#ifdef _DEBUG
#include "ImguiManager.h"
#include "Imgui.h"
void IFE::ParticleManager::DebugGUI()
{
	static bool add = false;
	static bool fdelete = false;
	static bool fmove = false;
	ImGui::Begin("EmitterManager", (bool*)false, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("add"))
		{
			ImGui::MenuItem("Add", "", &add);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("delete"))
		{
			ImGui::MenuItem("delete", "", &fdelete);
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	if (add)
	{
		if (ImGui::CollapsingHeader("New Emitter"))
		{
			static char name[256];
			static char tex[256];
			ImGui::InputText("New Emitter Name", name, sizeof(name));
			ImGui::InputText("Set Texture Name", tex, sizeof(tex));
			if (ImGui::Button("Add Emitter"))
			{
				if (SearchName(name))
				{
					ImGui::Text("error : an object with the same name exists");
				}
				else
				{
					AddInitialize(name, tex);
					ImGui::Text("success");
				}
			}
		}
	}
	for (unique_ptr<Emitter>& itr : emitterList_)
	{
		itr->DebugGUI(fdelete);
	}
	ImGui::End();
	for (unique_ptr<Emitter>& itr : emitterList_)
	{
		itr->ComponentGUI();
	}
}

void IFE::ParticleManager::OutputScene()
{
	JsonManager* jm = JsonManager::Instance();
	nlohmann::json& j = jm->GetJsonData();
	uint32_t i = 0;
	for (auto& itr : emitterList_)
	{
		itr->OutputScene(j[i]);
		i++;
	}
	jm->Output("ParticleManager");
}

void IFE::ParticleManager::DebugUpdate()
{
	emitterList_.remove_if([](unique_ptr<Emitter>& obj) {return obj->deleteFlag_; });
	for (unique_ptr<Emitter>& itr : emitterList_)
	{
		itr->DebugUpdate();
	}
}
#endif
#include "ComponentHelp.h"

void IFE::ParticleManager::LoadingScene()
{
	//JsonManager* jm = JsonManager::Instance();
	//jm->Input("ObjectManager");
	//nlohmann::json js = jm->GetJsonData();
	//for (auto& j : js)
	//{
	//	Object3D* obj;
	//	if (j["prefab"])obj = AddPrefab(j["name"]);
	//	else obj = Add(j["name"]);
	//	obj->SetModel(ModelManager::Instance()->GetModel(j["model"]));
	//	obj->LoadingScene(j);
	//	obj->Initialize();
	//}
}

std::string IFE::ParticleManager::GetNewName(const std::string& emitterName)
{
	int i = 0;
	string r;
	while (true)
	{
		string s = emitterName + to_string(i);
		if (SearchName(s))
		{
			i++;
		}
		else
		{
			r = s;
			break;
		}
	}
	return r;
}
