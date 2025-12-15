#pragma once

#include "Services.h"

class Part : public Instance {
protected:
	int RenderIndex = -1;
	RenderCubeObject_t StoredPrimitive;
public:
	RenderCubeObject_t* Primitive = &StoredPrimitive;
	string Shape = "";

	inline LuaVector GetPosition() { return LuaVector(Primitive->Position.x, Primitive->Position.y, Primitive->Position.z); }

	inline void SetPosition(LuaVector NewPosition) { Primitive->Position = glm::vec3(NewPosition.x, NewPosition.y, NewPosition.z); }

	inline LuaVector GetRotation() { return LuaVector(Primitive->Rotation.x, Primitive->Rotation.y, Primitive->Rotation.z); }

	inline void SetRotation(LuaVector NewRotation) { Primitive->Rotation = glm::vec3(NewRotation.x, NewRotation.y, NewRotation.z); }

	inline LuaVector GetSize() { return LuaVector(Primitive->Size.x, Primitive->Size.y, Primitive->Size.z); }

	inline void SetSize(LuaVector NewSize) { Primitive->Size = glm::vec3(NewSize.x, NewSize.y, NewSize.z); }

	inline LuaVector GetColor() { return LuaVector(Primitive->Color.r, Primitive->Color.g, Primitive->Color.b); }

	inline void SetColor(LuaVector NewColor) { Primitive->Color = glm::vec3(NewColor.x, NewColor.y, NewColor.z); }

	inline string GetShape() { return Shape; }

	inline void SetTransparency(double NewT) { Primitive->Transparency = (float)NewT; }

	inline double GetTransparency() { return Primitive->Transparency; }

	inline void SetShape(string NewShape) { 
		OnParentChanged(NULL);

		Shape = NewShape; 

		OnParentChanged(Parent.lock());
	}

	int SetFaceTextureLua(lua_State* L) {
		// 0 = +X, 1 = -X, 2 = +Y, 3 = -Y, 4 = +Z, 5 = -Z

		Part* Inst = (Part*)(luaL_checkinstance(L, 1).get());
		int Face = (int)luaL_checkinteger(L, 2);
		string TextureID = luaL_checkstring(L, 3);

		if (Face < 0 || Face > 5) {
			luaL_error(L, "Face number must be between 0 and 5.");

			return 0;
		}

		switch (Face) {
		case (0):
			Inst->Primitive->Texture0 = Texture::GetTextureByID(TextureID);
			break;
		case (1):
			Inst->Primitive->Texture1 = Texture::GetTextureByID(TextureID);
			break;
		case (2):
			Inst->Primitive->Texture2 = Texture::GetTextureByID(TextureID);
			break;
		case (3):
			Inst->Primitive->Texture3 = Texture::GetTextureByID(TextureID);
			break;
		case (4):
			Inst->Primitive->Texture4 = Texture::GetTextureByID(TextureID);
			break;
		case (5):
			Inst->Primitive->Texture5 = Texture::GetTextureByID(TextureID);
			break;
		}

		return 1;
	}

	int GetFaceTextureLua(lua_State* L) {
		// 0 = +X, 1 = -X, 2 = +Y, 3 = -Y, 4 = +Z, 5 = -Z

		Part* Inst = (Part*)(luaL_checkinstance(L, 1).get());
		int Face = (int)luaL_checkinteger(L, 2);
		if (Face < 0 || Face > 5) {
			luaL_error(L, "Face number must be between 0 and 5.");

			return 0;
		}

		int TextureID = 0;

		switch (Face) {
		case (0):
			TextureID = Inst->Primitive->Texture0;
			break;
		case (1):
			TextureID = Inst->Primitive->Texture1;
			break;
		case (2):
			TextureID = Inst->Primitive->Texture2;
			break;
		case (3):
			TextureID = Inst->Primitive->Texture3;
			break;
		case (4):
			TextureID = Inst->Primitive->Texture4;
			break;
		case (5):
			TextureID = Inst->Primitive->Texture5;
			break;
		}

		string TextureName = "";

		for (const auto& [key, value] : Texture::Textures)
			if (value == TextureID)
				TextureName = key;

		lua_pushstring(L, TextureName.c_str());

		return 1;
	}

	void OnParentChanged(shared_ptr<Instance> NewParent) override {
		bool IsRenderable = false;

		auto World = Services::GetService<Scene>("Scene");

		if (NewParent && this->IsAncestorOf(World)) {
			IsRenderable = true;
		}

		auto& RO = Graphics::Engine3D::RenderObjects[Shape];

		if (IsRenderable) {
			if (RenderIndex == -1) {
				RenderIndex = (int)RO.size();

				auto& T = RO.emplace_back();

				T = {
					this,
					StoredPrimitive
				};

				Primitive = &RO[RenderIndex].Object;
			}
		}
		else {
			if (RenderIndex != -1) {
				StoredPrimitive = RO[RenderIndex].Object;
				Primitive = &StoredPrimitive;

				Part* OtherPart = (Part*)RO.back().Storage;
				OtherPart->RenderIndex = RenderIndex;

				std::swap(RO[RenderIndex], RO.back());
				RO.pop_back();

				RenderIndex = -1;
			}
		}

		return;
	}

	virtual ~Part() {
		OnParentChanged(NULL);
	}

	Part() {
		Type = "Part";
		Name = "Part";
		Primitive->Size = { 2.0, 1.0, 3.0 };
		Primitive->Color = { 1.0, 1.0, 1.0 };
		Primitive->Transparency = 1.0;
		Shape = "Cube";
	}
};

auto propPartPosition = CreatePropertyDescriptor(Part, "Part", "Position", LuaVector, L_Vector, &Part::SetPosition, &Part::GetPosition);
auto propPartRotation = CreatePropertyDescriptor(Part, "Part", "Rotation", LuaVector, L_Vector, &Part::SetRotation, &Part::GetRotation);
auto propPartSize = CreatePropertyDescriptor(Part, "Part", "Size", LuaVector, L_Vector, &Part::SetSize, &Part::GetSize);
auto propPartColor = CreatePropertyDescriptor(Part, "Part", "Color", LuaVector, L_Vector, &Part::SetColor, &Part::GetColor);
auto propPartShape = CreatePropertyDescriptor(Part, "Part", "Shape", string, L_String, &Part::SetShape, &Part::GetShape);
auto propPartTransparency = CreatePropertyDescriptor(Part, "Part", "Transparency", double, L_Number, &Part::SetTransparency, &Part::GetTransparency);
auto callPartSetFaceTexture = CreateLuaNamecallDescriptor(Part, "Part", "SetFaceTexture", &Part::SetFaceTextureLua);
auto callPartGetFaceTexture = CreateLuaNamecallDescriptor(Part, "Part", "GetFaceTexture", &Part::GetFaceTextureLua);
CreateClassDescriptor(Part, "Part", "Instance");