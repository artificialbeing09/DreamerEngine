#pragma once

#include "Instance.h"

class GameWorld : public Instance {
protected:
	int64_t GameID = 0;
public:
	int64_t GetGameID() { return GameID; }

	void SetGameID(int64_t NewGameID) { GameID = NewGameID; }

	GameWorld() {
		Type = "GameWorld";
		Name = "Game";
	}
};

auto propGameWorldGameID = CreatePropertyDescriptor(GameWorld, "GameWorld", "GameID", int64_t, L_Int, &GameWorld::SetGameID, &GameWorld::GetGameID);
CreateClassDescriptor(GameWorld, "GameWorld", "Instance");

shared_ptr<GameWorld> GetGameWorld() {
	static shared_ptr<GameWorld> World = NULL;

	if (World == NULL)
		World = make_shared<GameWorld>();

	return World;
}