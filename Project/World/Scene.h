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

	LuaCoordinateFrame GetCFrame() {
		auto Rot = Graphics::Engine3D::Camera::Rotation;

		return LuaCoordinateFrame(Graphics::Engine3D::Camera::Position, Gl.DirectionFromEuler(Rot.x, Rot.y, Rot.z));
	}

	void SetCFrame(LuaCoordinateFrame CF) {
		Graphics::Engine3D::Camera::Position = CF.Position;

		Graphics::Engine3D::Camera::Rotation = glm::eulerAngles(glm::quat_cast(CF.Rotation));
	}

	double GetFOV() { return Graphics::Engine3D::Camera::FOVY; }
	void SetFOV(double FOV) { Graphics::Engine3D::Camera::FOVY = (float)FOV; }

	int StudioGameModified = 0;

	Scene() {
		Type = "Scene";
		Name = "Scene";
	}
};

auto propCameraPosition = CreatePropertyDescriptor(Scene, "Scene", "CameraPosition", LuaVector, L_Vector, &Scene::SetPosition, &Scene::GetPosition);
auto propCameraRotation = CreatePropertyDescriptor(Scene, "Scene", "CameraRotation", LuaVector, L_Vector, &Scene::SetRotation, &Scene::GetRotation);
auto propCameraCFrame = CreatePropertyDescriptor(Scene, "Scene", "CameraCFrame", LuaCoordinateFrame, L_CFrame, &Scene::SetCFrame, &Scene::GetCFrame);
auto propCameraFOV = CreatePropertyDescriptor(Scene, "Scene", "CameraFOV", double, L_Number, &Scene::SetFOV, &Scene::GetFOV);

auto eventSceneStudioGameStarted = CreateLuaEventDescriptor(Scene, "Scene", "StudioGameModified", Scene::StudioGameModified);

CreateClassDescriptor(Scene, "Scene", "Instance");