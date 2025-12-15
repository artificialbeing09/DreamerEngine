#pragma once

#include "Instance.h"

class Scene : public Instance {
protected:
public:
	LuaVector GetPosition() {
		auto Orig = Graphics::Engine3D::Camera::Position;
		return LuaVector(Orig.x, Orig.y, Orig.z);
	}
	void SetPosition(LuaVector Position) {
		Graphics::Engine3D::Camera::Position = glm::vec3(Position.x, Position.y, Position.z);
	}

	LuaVector GetRotation() {
		auto Orig = Graphics::Engine3D::Camera::Rotation;
		return LuaVector(Orig.x, Orig.y, Orig.z);
	}
	void SetRotation(LuaVector Rotation) {
		Graphics::Engine3D::Camera::Rotation = glm::vec3(Rotation.x, Rotation.y, Rotation.z);
	}

	double GetFOV() { return Graphics::Engine3D::Camera::FOVY; }
	void SetFOV(double FOV) { Graphics::Engine3D::Camera::FOVY = (float)FOV; }

	Scene() {
		Type = "Scene";
		Name = "Scene";
	}
};

auto propCameraPosition = CreatePropertyDescriptor(Scene, "Scene", "CameraPosition", LuaVector, L_Vector, &Scene::SetPosition, &Scene::GetPosition);
auto propCameraRotation = CreatePropertyDescriptor(Scene, "Scene", "CameraRotation", LuaVector, L_Vector, &Scene::SetRotation, &Scene::GetRotation);
auto propCameraFOV = CreatePropertyDescriptor(Scene, "Scene", "CameraFOV", double, L_Number, &Scene::SetFOV, &Scene::GetFOV);
CreateClassDescriptor(Scene, "Scene", "Instance");