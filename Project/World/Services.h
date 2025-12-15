#pragma once

#include "Scene.h"
#include "GameWorld.h"
#include "Sky.h"
#include "UIScene.h"
#include "Input.h"

namespace Services {
	bool CreateServices() {
		shared_ptr<GameWorld> GameWorld = GetGameWorld();

		shared_ptr<Instance> Space = CreateInstanceOfType("Scene");
		if (!Space) {
			throw std::runtime_error("CreateInstanceOfType returned null for 'Scene'");
		}
		Space->SetParent(GameWorld);

		auto NewSky = CreateInstanceOfType("Sky");
		NewSky->SetParent(GameWorld);

		auto NewUIScene = CreateInstanceOfType("UIScene");
		NewUIScene->SetParent(GameWorld);

		auto NewInput = CreateInstanceOfType("Input");
		NewInput->SetParent(GameWorld);

		return true;
	}

	template <typename T>
	shared_ptr<T> GetService(string Name) {
		auto GameWorld = GetGameWorld();

		for (auto O : GameWorld->GetChildren()) {
			if (O->GetType() == Name) {
				return dynamic_pointer_cast<T>(O);
			}
		}

		shared_ptr<T> Service = dynamic_pointer_cast<T>(CreateInstanceOfType(Name));
		Service->SetParent(GameWorld);

		return Service;
	}
}