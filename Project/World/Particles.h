#pragma once

#include "Part.h"

using namespace std;

// TODO: Add random particle directions, check transparency

class Particles : public Instance {
protected:
	size_t NumberOfAllocated = 0;

	vector<int> RenderIndices = {};
	
	size_t ParticleCount = 0;
public:
	float Rate = 1.0; // per second
	ParticleObject_t Primitive;

	size_t GetNumberOfParticles() {
		return (Primitive.Lifetime * Rate) + 1;
	}

	void UpdatePrimitives() {
		ParticleCount = GetNumberOfParticles();

		Primitive.Storage = (Particles*)this;

		// If the current number of generated particles is greater than the allocated amount
		//			or is too small (the memory gains generated are too little)
		if ((ParticleCount > NumberOfAllocated) || (ParticleCount < (NumberOfAllocated / 2))) {
			NumberOfAllocated = ParticleCount;

			for (int I : RenderIndices) { // Reallocation
				std::swap(Graphics::Engine3D::Particles[I], Graphics::Engine3D::Particles.back());

				Graphics::Engine3D::Particles.pop_back();

				Particles* P = (Particles*)Graphics::Engine3D::Particles[I].Storage;

				if (P == this)
					continue;

				auto ItIndices = P->RenderIndices;

				for (int J = 0; J < ItIndices.size(); J++) {
					if (ItIndices[J] == Graphics::Engine3D::Particles.size()) {
						ItIndices[J] = I;
						break;
					}
				}
			}

			RenderIndices = {};

			for (int I = 0; I < ParticleCount; I++) {
				RenderIndices.push_back(Graphics::Engine3D::Particles.size());

				Graphics::Engine3D::Particles.push_back({});
			}
		}

		int J = 0;

		for (auto I : RenderIndices) {
			if (J >= ParticleCount)
				break;

			Primitive.TimeStart = Utils::GetSecondsSinceStart();
			Primitive.RateOffset = (1.0 / Rate) * (float)J;

			Graphics::Engine3D::Particles[I] = Primitive;

			J += 1;
		}
	}

	inline LuaVector GetPosition() { return LuaVector(Primitive.Position.x, Primitive.Position.y, Primitive.Position.z); }

	inline void SetPosition(LuaVector NewPosition) {
		Primitive.Position = glm::vec3(NewPosition.x, NewPosition.y, NewPosition.z);

		UpdatePrimitives();
	}

	inline LuaVector GetVelocity() { return LuaVector(Primitive.Velocity.x, Primitive.Velocity.y, Primitive.Velocity.z); }

	inline void SetVelocity(LuaVector NewPosition) {
		Primitive.Velocity = glm::vec3(NewPosition.x, NewPosition.y, NewPosition.z);

		UpdatePrimitives();
	}

	inline LuaVector GetAcceleration() { return LuaVector(Primitive.Acceleration.x, Primitive.Acceleration.y, Primitive.Acceleration.z); }

	inline void SetAcceleration(LuaVector NewPosition) {
		Primitive.Acceleration = glm::vec3(NewPosition.x, NewPosition.y, NewPosition.z);

		UpdatePrimitives();
	}

	inline LuaVector GetColor() { return LuaVector(Primitive.Color.x, Primitive.Color.y, Primitive.Color.z); }

	inline void SetColor(LuaVector NewPosition) {
		Primitive.Color = glm::vec4(NewPosition.x, NewPosition.y, NewPosition.z, Primitive.Color.a);

		UpdatePrimitives();
	}

	inline double GetTransparency() { return Primitive.Color.a; }

	inline void SetTransparency(double NewPosition) {
		Primitive.Color = glm::vec4(Primitive.Color.x, Primitive.Color.y, Primitive.Color.z, NewPosition);

		UpdatePrimitives();
	}

	string StoredTexture = "";

	void SetTexture(string NewTexture) { StoredTexture = NewTexture; Primitive.Texture = Texture::Textures[NewTexture]; UpdatePrimitives(); }
	string GetTexture() { return StoredTexture; }

	inline double GetLifetime() { return Primitive.Lifetime; }

	inline void SetLifetime(double NewPosition) {
		Primitive.Lifetime = NewPosition;

		UpdatePrimitives();
	}

	inline double GetRate() { return Rate; }

	inline void SetRate(double NewPosition) {
		Rate = NewPosition;

		UpdatePrimitives();
	}

	Particles() {
		Type = "Particles";
		Name = "Particles";
	}
};

auto propParticlesTexture = CreatePropertyDescriptor(Particles, "Particles", "Texture", string, L_String, &Particles::SetTexture, &Particles::GetTexture);
auto propParticlesLifetime = CreatePropertyDescriptor(Particles, "Particles", "Lifetime", double, L_Number, &Particles::SetLifetime, &Particles::GetLifetime);
auto propParticlesRate = CreatePropertyDescriptor(Particles, "Particles", "Rate", double, L_Number, &Particles::SetRate, &Particles::GetRate);
auto propParticlesTransparency = CreatePropertyDescriptor(Particles, "Particles", "Transparency", double, L_Number, &Particles::SetTransparency, &Particles::GetTransparency);
auto propParticlesPosition = CreatePropertyDescriptor(Particles, "Particles", "Position", LuaVector, L_Vector, &Particles::SetPosition, &Particles::GetPosition);
auto propParticlesVelocity = CreatePropertyDescriptor(Particles, "Particles", "Velocity", LuaVector, L_Vector, &Particles::SetVelocity, &Particles::GetVelocity);
auto propParticlesAcceleration = CreatePropertyDescriptor(Particles, "Particles", "Acceleration", LuaVector, L_Vector, &Particles::SetAcceleration, &Particles::GetAcceleration);
auto propParticlesColor = CreatePropertyDescriptor(Particles, "Particles", "Color", LuaVector, L_Vector, &Particles::SetColor, &Particles::GetColor);

CreateClassDescriptor(Particles, "Particles", "Instance");