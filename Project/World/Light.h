#pragma once

#include "Services.h"

class SpotLight : public Instance {
protected:
	int RenderIndex = -1;
	LightObject_t StoredPrimitive;
public:
	LightObject_t* Primitive = &StoredPrimitive;

	glm::vec3 Rotation = glm::vec3(0.0f, -1.0f, 0.0f);
	
	inline LuaVector GetPosition() { return LuaVector(Primitive->Position.x, Primitive->Position.y, Primitive->Position.z); }

	inline void SetPosition(LuaVector NewPosition) { Primitive->Position = glm::vec3(NewPosition.x, NewPosition.y, NewPosition.z); }

	inline LuaVector GetColor() { return LuaVector(Primitive->Color.x, Primitive->Color.y, Primitive->Color.z); }

	inline void SetColor(LuaVector NewColor) { Primitive->Color = glm::vec3(NewColor.x, NewColor.y, NewColor.z); }

	inline LuaVector GetRotation() { return LuaVector(Rotation.x, Rotation.y, Rotation.z); }

	inline void SetRotation(LuaVector NewRotation) {
		Rotation = glm::vec3(NewRotation.x, NewRotation.y, NewRotation.z);
		Primitive->Direction = Graphics::Engine3D::Camera::DirectionFromEuler(NewRotation.x, NewRotation.y, NewRotation.z);
	}

	inline double GetDistance() { return (double)Primitive->farPlane; }

	inline void SetDistance(double NewDistance) {
		Primitive->farPlane = NewDistance;
	}

	inline double GetFOV() { return (double)Primitive->FOV; }

	inline void SetFOV(double NewFOV) {
		Primitive->FOV = NewFOV;
	}

	void OnParentChanged(shared_ptr<Instance> NewParent) override {
		bool IsRenderable = false;

		auto World = Services::GetService<Scene>("Scene");

		if (NewParent && this->IsAncestorOf(World)) {
			IsRenderable = true;
		}

		auto& RO = Graphics::Engine3D::Lights;

		if (IsRenderable) {
			if (RenderIndex == -1) {
				RenderIndex = (int)RO.size();

				auto& T = RO.emplace_back();

				T = {
					this,
					StoredPrimitive
				};

				Primitive = &RO[RenderIndex].Light;
			}
		}
		else {
			if (RenderIndex != -1) {
				StoredPrimitive = RO[RenderIndex].Light;
				Primitive = &StoredPrimitive;

				SpotLight* OtherPart = (SpotLight*)RO.back().Storage;
				OtherPart->RenderIndex = RenderIndex;

				std::swap(RO[RenderIndex], RO.back());
				RO.pop_back();

				RenderIndex = -1;
			}
		}

		return;
	}

	SpotLight() {
		Type = "SpotLight";
		Name = "SpotLight";
		Primitive->Position = glm::vec3(0.0f, 0.0f, 0.0f);
		Primitive->Direction = glm::vec3(0.0f, -1.0f, 0.0f);
		Primitive->FOV = glm::radians(80.0f);
		Primitive->farPlane = 100.0f;
		Primitive->nearPlane = 1.0f;
	}
};

auto propSpotLightPosition = CreatePropertyDescriptor(SpotLight, "SpotLight", "Position", LuaVector, L_Vector, &SpotLight::SetPosition, &SpotLight::GetPosition);
auto propSpotLightRotation = CreatePropertyDescriptor(SpotLight, "SpotLight", "Rotation", LuaVector, L_Vector, &SpotLight::SetRotation, &SpotLight::GetRotation);
auto propSpotLightColor = CreatePropertyDescriptor(SpotLight, "SpotLight", "Color", LuaVector, L_Vector, &SpotLight::SetColor, &SpotLight::GetColor);
auto propSpotLightFOV = CreatePropertyDescriptor(SpotLight, "SpotLight", "FOV", double, L_Number, &SpotLight::SetFOV, &SpotLight::GetFOV);
auto propSpotLightDistance = CreatePropertyDescriptor(SpotLight, "SpotLight", "Distance", double, L_Number, &SpotLight::SetDistance, &SpotLight::GetDistance);
CreateClassDescriptor(SpotLight, "SpotLight", "Instance");