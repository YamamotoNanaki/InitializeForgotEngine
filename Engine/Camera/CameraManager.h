#pragma once
#include "Camera.h"
#include <list>
#include <memory>

namespace IFE
{
	class CameraManager
	{
		CameraManager() {}
		CameraManager(const CameraManager&) {}
		CameraManager& operator=(const CameraManager&) {}
		~CameraManager();

		std::list<std::unique_ptr<Camera>>cameraList;

	public:
		static Camera* sActivCamera_;
		static CameraManager* Instance();
		void Initialize();
		void Update();

#ifdef _DEBUG
		void DebugGUI();
#endif
	};
}
