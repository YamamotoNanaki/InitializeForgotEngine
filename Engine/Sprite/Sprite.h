#pragma once
#include "IFMath.h"
#include "ComponentManager.h"
#include "Component.h"
#include "VertexBuffer.h"
#include "Texture.h"
#include "GraphicsPipeline.h"
#include "ConstStruct.h"
#include <string>

namespace IFE
{
	class Transform2D;
	class Sprite :public ComponentManager
	{
	protected:
		static GraphicsPipeline* gp_;

	protected:
		VertexBuffer<Vertex2D> vb_;
	public:
		//アフィン変換情報
		//Float2 scale = { 1,1 };
		//float rotation = 0;
		//Float2 position = { 0,0 };
		Float2 size_ = { 100,100 };
		//ワールド変換行列
		//Matrix matWorld;
		Float2 texBase_ = { 0,0 };
		Float2 texSize_ = { 100,100 };
		Float2 anchorpoint_ = { 0.5,0.5 };
		bool flipX_ = false;
		bool flipY_ = false;
		std::string spriteName_;
		Texture* tex_ = nullptr;
		bool drawFlag_ = true;
		bool deleteFlag_ = false;
		bool isActive_ = true;
		Transform2D* transform_ = nullptr;
	public:
		Sprite();
		static void StaticInitialize();
		void BufferInitialize();
		void SPRITEInitialize(const std::string& texName = "white.png", const Float2& size = { 100,100 }, const Float2& anchorpoint = { 0.5,0.5 }, bool flipX = false, bool flipY = false);
		void Initialize(const std::string& texName = "white.png", const Float2& size = { 100,100 }, const Float2& anchorpoint = { 0.5,0.5 }, bool flipX = false, bool flipY = false);
		void TransferVertex();
		static void DrawBefore();
		void Update();
		void Draw();
		void SetSize(const Float2& size);
		void SetTextureRect(const Float2& texBase, const Float2& texSize);
		void SetComponent(std::unique_ptr<Component> component);
		void SetComponentFront(std::unique_ptr<Component> component);
		void SetTexture(const std::string& texName);
		void LoadingScene(nlohmann::json& j);

#ifdef _DEBUG
		virtual void DebugGUI(bool fdelete, bool fmove, std::string* str);
		void ComponentGUI();
		void DebugUpdate();
		void OutputScene(nlohmann::json& j);
#endif
	};
}