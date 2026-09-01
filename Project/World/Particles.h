#pragma once

#include "Part.h"
#include <random>

using namespace std;

class Particles : public Instance {
protected:
	size_t NumberOfAllocated = 0;

	vector<int> RenderIndices = {};
	vector<ParticleObjectExtra_t> RenderExtras = {};
	
	size_t ParticleCount = 0;

	bool IsRenderable = false;
public:
	ParticleExtraInfo_t Primitive;

	size_t GetNumberOfParticles() {
		return ceil(Primitive.Lifetime * Primitive.Rate);
	}

	void Deallocate() {
		NumberOfAllocated = 0;

		for (int H = RenderIndices.size() - 1; H >= 0; --H) { // Reallocation
			int I = RenderIndices[H];

			bool AtEnd = I == (Graphics::Engine3D::Particles.size() - 1);
			
			if (!AtEnd)
				std::swap(Graphics::Engine3D::Particles[I], Graphics::Engine3D::Particles.back());

			Graphics::Engine3D::Particles.pop_back();

			if (AtEnd)
				continue;

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
	}

	float LastTime = 0.0;

	void UpdateRender() {
		int J = 0;
		float CurrentTime = Utils::GetSecondsSinceStart();
		float TimePad = max(Primitive.Lifetime, 1.0f / Primitive.Rate);
		float TimeDiff = CurrentTime - LastTime;

		for (int I : RenderIndices) {
			if (J >= ParticleCount)
				break;

			float TimeStart = (CurrentTime - Primitive.TimeStart) - ((1.0f / Primitive.Rate) * (float)J);
			float T = fmod(TimeStart, TimePad);
			float T10 = T / Primitive.Lifetime;

			auto Render = Graphics::Engine3D::Particles[I];

			if (TimeStart >= 0 && T <= Primitive.Lifetime) {
				if (Render.Scale == 0.0f) {
					RenderExtras[J].Velocity = Primitive.StartRand +
						(Primitive.DiffRand * glm::vec3(
							(float)rand() / (float)RAND_MAX,
							(float)rand() / (float)RAND_MAX,
							(float)rand() / (float)RAND_MAX
						));
				}

				Render.Position = Primitive.Position +
					((RenderExtras[J].Velocity + Primitive.Velocity) * T) +
					(Primitive.Acceleration * 0.5f * T * T);
				Render.Scale = Primitive.Scale;

				Render.Color = (Primitive.Color * (1.0f - T10)) + (Primitive.EndColor * T10);
			}
			else {
				Render.Scale = 0.0f;
			}

			Graphics::Engine3D::Particles[I] = Render;

			J += 1;
		}

		LastTime = CurrentTime;
	}

	void UpdatePrimitives() {
		if (!IsRenderable) {
			return;
		}

		ParticleCount = GetNumberOfParticles();

		// If the current number of generated particles is greater than the allocated amount
		//			or is too small (the memory gains generated are too little)
		if ((ParticleCount > NumberOfAllocated) || (ParticleCount < (NumberOfAllocated / 2))) {
			Deallocate();

			NumberOfAllocated = ParticleCount;

			RenderExtras = {};

			for (int I = 0; I < NumberOfAllocated; I++) {
				RenderIndices.push_back(Graphics::Engine3D::Particles.size());

				Graphics::Engine3D::Particles.push_back({});
				RenderExtras.push_back({});
			}
		}

		int J = 0;
		Primitive.TimeStart = Utils::GetSecondsSinceStart();
		LastTime = Primitive.TimeStart;

		for (int I : RenderIndices) {
			if (J >= ParticleCount)
				break;

			Graphics::Engine3D::Particles[I].Storage = this;
			Graphics::Engine3D::Particles[I].Texture = Primitive.Texture;

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

	inline LuaVector GetEndColor() { return LuaVector(Primitive.EndColor.x, Primitive.EndColor.y, Primitive.EndColor.z); }

	inline void SetEndColor(LuaVector NewPosition) {
		Primitive.EndColor = glm::vec4(NewPosition.x, NewPosition.y, NewPosition.z, Primitive.Color.a);

		UpdatePrimitives();
	}

	inline double GetTransparency() { return Primitive.Color.a; }

	inline void SetTransparency(double NewPosition) {
		Primitive.Color = glm::vec4(Primitive.Color.x, Primitive.Color.y, Primitive.Color.z, NewPosition);

		UpdatePrimitives();
	}

	inline double GetEndTransparency() { return Primitive.EndColor.a; }

	inline void SetEndTransparency(double NewPosition) {
		Primitive.EndColor = glm::vec4(Primitive.EndColor.x, Primitive.EndColor.y, Primitive.EndColor.z, NewPosition);

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

	inline double GetRate() { return Primitive.Rate; }

	inline void SetRate(double NewPosition) {
		Primitive.Rate = NewPosition;

		UpdatePrimitives();
	}

	inline double GetScale() { return Primitive.Scale; }

	inline void SetScale(double NewPosition) {
		Primitive.Scale = NewPosition;

		UpdatePrimitives();
	}

	inline LuaVector GetMinimumRandomVelocity() { return LuaVector(Primitive.StartRand.x, Primitive.StartRand.y, Primitive.StartRand.z); }

	inline void SetMinimumRandomVelocity(LuaVector NewPosition) {
		Primitive.DiffRand += Primitive.StartRand;

		Primitive.StartRand = glm::vec3(NewPosition.x, NewPosition.y, NewPosition.z);

		Primitive.DiffRand -= Primitive.StartRand;

		UpdatePrimitives();
	}

	inline LuaVector GetMaximumRandomVelocity() {
		glm::vec3 Maximum = Primitive.DiffRand + Primitive.StartRand;
		return LuaVector(Maximum.x, Maximum.y, Maximum.z);
	}

	inline void SetMaximumRandomVelocity(LuaVector NewPosition) {
		Primitive.DiffRand = glm::vec3(NewPosition.x, NewPosition.y, NewPosition.z) - Primitive.StartRand;

		UpdatePrimitives();
	}

	void OnParentChanged(shared_ptr<Instance> NewParent) override {
		IsRenderable = false;

		auto World = Services::GetService<Scene>("Scene");

		if (NewParent && this->IsDescendantOf(World)) {
			IsRenderable = true;
		}

		if (IsRenderable && NumberOfAllocated == 0) {
			UpdatePrimitives();
		}
		else if (!IsRenderable && NumberOfAllocated > 0) {
			Deallocate();

			ParticleCount = 0;
		}
	}

	Particles() {
		Type = "Particles";
		Name = "Particles";
	}
};

void UpdateParticles() {
	auto World = Services::GetService<Scene>("Scene");

	for (auto Inst : World->GetDescendants()) {
		if (Inst && Inst->GetType() == "Particles") {
			shared_ptr<Particles> P = dynamic_pointer_cast<Particles>(Inst);
			P->UpdateRender();
		}
	}
}

auto propParticlesTexture = CreatePropertyDescriptor(Particles, "Particles", "Texture", string, L_String, &Particles::SetTexture, &Particles::GetTexture);
auto propParticlesLifetime = CreatePropertyDescriptor(Particles, "Particles", "Lifetime", double, L_Number, &Particles::SetLifetime, &Particles::GetLifetime);
auto propParticlesScale = CreatePropertyDescriptor(Particles, "Particles", "Scale", double, L_Number, &Particles::SetScale, &Particles::GetScale);
auto propParticlesRate = CreatePropertyDescriptor(Particles, "Particles", "Rate", double, L_Number, &Particles::SetRate, &Particles::GetRate);
auto propParticlesTransparency = CreatePropertyDescriptor(Particles, "Particles", "Transparency", double, L_Number, &Particles::SetTransparency, &Particles::GetTransparency);
auto propParticlesEndTransparency = CreatePropertyDescriptor(Particles, "Particles", "EndTransparency", double, L_Number, &Particles::SetEndTransparency, &Particles::GetEndTransparency);
auto propParticlesPosition = CreatePropertyDescriptor(Particles, "Particles", "Position", LuaVector, L_Vector, &Particles::SetPosition, &Particles::GetPosition);
auto propParticlesVelocity = CreatePropertyDescriptor(Particles, "Particles", "Velocity", LuaVector, L_Vector, &Particles::SetVelocity, &Particles::GetVelocity);
auto propParticlesAcceleration = CreatePropertyDescriptor(Particles, "Particles", "Acceleration", LuaVector, L_Vector, &Particles::SetAcceleration, &Particles::GetAcceleration);
auto propParticlesMinimumRandomVelocity = CreatePropertyDescriptor(Particles, "Particles", "MinimumRandomVelocity", LuaVector, L_Vector, &Particles::SetMinimumRandomVelocity, &Particles::GetMinimumRandomVelocity);
auto propParticlesMaximumRandomVelocity = CreatePropertyDescriptor(Particles, "Particles", "MaximumRandomVelocity", LuaVector, L_Vector, &Particles::SetMaximumRandomVelocity, &Particles::GetMaximumRandomVelocity);
auto propParticlesColor = CreatePropertyDescriptor(Particles, "Particles", "Color", LuaVector, L_Vector, &Particles::SetColor, &Particles::GetColor);
auto propParticlesEndColor = CreatePropertyDescriptor(Particles, "Particles", "EndColor", LuaVector, L_Vector, &Particles::SetEndColor, &Particles::GetEndColor);

CreateClassDescriptor(Particles, "Particles", "Instance");