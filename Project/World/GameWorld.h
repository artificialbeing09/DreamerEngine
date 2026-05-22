#pragma once

#include "Instance.h"

namespace Serializer {
	void LoadMap(string Name, bool ClearMap);
}

class GameWorld : public Instance {
protected:
	int64_t GameID = 0;
public:
	int64_t GetGameID() { return GameID; }

	void SetGameID(int64_t NewGameID) { GameID = NewGameID; }

	int SetMap(lua_State* L) {
		string MapPath = luaL_checkstring(L, 2);
		bool ClearMap = lua_toboolean(L, 3);

		Serializer::LoadMap(MapPath, ClearMap);

		return 0;
	}

	GameWorld() {
		Type = "GameWorld";
		Name = "Game";
	}
};

auto propGameWorldGameID = CreatePropertyDescriptor(GameWorld, "GameWorld", "GameID", int64_t, L_Int, &GameWorld::SetGameID, &GameWorld::GetGameID);
auto callGameWorldSetMap = CreateLuaNamecallDescriptor(GameWorld, "GameWorld", "SetMap", &GameWorld::SetMap);
CreateClassDescriptor(GameWorld, "GameWorld", "Instance");

shared_ptr<GameWorld> GetGameWorld() {
	static shared_ptr<GameWorld> World = NULL;

	if (World == NULL)
		World = make_shared<GameWorld>();

	return World;
}