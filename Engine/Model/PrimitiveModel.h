#pragma once
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Component.h"
#include "ConstStruct.h"
#include "AddModelSettings.h"

namespace IFE
{
	class PrimitiveModel : public Component
	{
		using Component::Component;
		VertexBuffer<Vertex> vb;
		IndexBuffer ib;
		AddModelSettings setting;
	public:
		void CreateCube(bool smoothing = false);
		void CreatePolygonSquare(bool smoothing = false);
		void CreateTriangle(bool smoothing = false);
		void CreateCircle(bool smoothing = false);
		void CreateSphere(bool smoothing = false);
		void Draw()override;
		void Update()override;
		void Initialize()override {}

	private:
		void SmoothNormal(Vertex vertices[]);
		void CreateNormal(Vertex vertices[], uint32_t indices[], size_t indicesSize);
		void SmoothNormal(std::vector<Vertex>& vertices);
		void CreateNormal(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

#ifdef _DEBUG
		void DebugGUI();
		void OutputScene(std::string object3d);
#endif
		void LoadingScene(std::string object3d);
	};
}