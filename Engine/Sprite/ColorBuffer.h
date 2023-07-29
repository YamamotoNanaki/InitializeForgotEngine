#pragma once
#include "Component.h"
#include "ConstStruct.h"
#include "ConstBuffer.h"
#include <memory>

namespace IFE
{
	class ColorBuffer : public Component
	{
		using Component::Component;
		std::unique_ptr<ConstBuffer<ConstBufferColor>> colorBuffer_;
		ConstBufferColor* constMapColor_ = nullptr;

	private:
		Float4 color_ = {1,1,1,1};

	public:
		void Initialize()override;
		void Draw()override;
		void SetColor(const Float4& color);
		void SetColor(float r, float g, float b, float a);
#ifdef _DEBUG
		void DebugGUI();
		void OutputComponent(nlohmann::json&json)override;
#endif
		void LoadingComponent(nlohmann::json& json)override;
	};
}
