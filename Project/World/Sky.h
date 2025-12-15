#pragma once

#include "Instance.h"

/*

game.Sky:SetFaceTexture(0, "SkyRight.png")
game.Sky:SetFaceTexture(1, "SkyLeft.png")
game.Sky:SetFaceTexture(2, "SkyTop.png")
game.Sky:SetFaceTexture(3, "SkyBottom.png")
game.Sky:SetFaceTexture(4, "SkyFront.png")
game.Sky:SetFaceTexture(5, "SkyBack.png")

*/

class Sky : public Instance {
protected:
public:
	int GetTexture(lua_State* L) {
		Sky* This = (Sky*)(luaL_checkinstance(L, 1).get());
		int TexNumber = (int)luaL_checkinteger(L, 2);

		if (TexNumber > 5 || TexNumber < 0) {
			lua_pushnil(L);
			return 1;
		}
		
		lua_pushstring(L, (Graphics::Sky::SkyTexture)[TexNumber]);

		return 1;
	}

	int SetTexture(lua_State* L) {
		Sky* This = (Sky*)(luaL_checkinstance(L, 1).get());
		int TexNumber = (int)luaL_checkinteger(L, 2);
		const char* NewTex = luaL_checkstring(L, 3);

		lua_pushnil(L);

		if (TexNumber > 5 || TexNumber < 0) {
			return 1;
		}

		string& TexSafe = *(new string);
		TexSafe = NewTex;

		(Graphics::Sky::SkyTexture)[TexNumber] = TexSafe.c_str();

		return 1;
	}

	Sky() {
		Type = "Sky";
		Name = "Sky";

		for (int I = 0; I < 6; I++) {
			Graphics::Sky::SkyTexture[I] = "";
		}
	}
};

auto callSkyGetTexture = CreateLuaNamecallDescriptor(Sky, "Sky", "GetFaceTexture", &Sky::GetTexture);
auto callSkySetTexture = CreateLuaNamecallDescriptor(Sky, "Sky", "SetFaceTexture", &Sky::SetTexture);
CreateClassDescriptor(Sky, "Sky", "Instance");