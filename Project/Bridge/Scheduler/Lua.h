#pragma once

#include "Base.h"

using namespace std;

namespace Scheduler::Lua {
    lua_State* RunScript(string Script, string Name = "console", int identity = 0);

    int lua_sleep(lua_State* L) {
        double Time = luaL_checknumber(L, 1);
        long TimeMilli = (long)(Time * 1000.0);

        auto S = GetScheduler();
        S->Threads[L].cState = 2;
        S->Threads[L].returnFunction = [](lua_State* myState, void* storage) {
            lua_pushboolean(myState, 1);
            lua_pushboolean(myState, 1);
        };
        S->Threads[L].returnValues = 1;

        if (TimeMilli == 0) {
            S->Threads[L].cState = 1;
        }
        else {
            struct SleepYield {
                long long Start = 0;
                long long Length = 0;
            };

            auto StartTime = Utils::GetMilliseconds();

            SleepYield* Storage = new SleepYield;
            Storage->Start = StartTime;
            Storage->Length = TimeMilli;

            S->Threads[L].yieldStorage = Storage;

            S->Threads[L].yieldFunction = [](void* Storage) {
                if (Storage == NULL) {
                    return true;
                }

                SleepYield* Info = (SleepYield*)Storage;

                auto CurrentTime = Utils::GetMilliseconds();

                if (CurrentTime - Info->Start >= Info->Length) {
                    delete Info;

                    return true;
                }

                return false;
               };
        }

        return lua_yield(L, 0);
    }

    int lua_spawn(lua_State* L) {
        luaL_checktype(L, 1, LUA_TFUNCTION);

        auto nL = Scheduler::GenerateNewThread(L, 1);

        Scheduler::AddThreadToSchedulerLoop(nL, L);

        return 0;
    }

    map<string, lua_State*> ModuleHasBeenRequired = {};

    int lua_require(lua_State* L) {
        string moduleName = luaL_checkstring(L, 1);

        int ref = Scheduler::luaRequires[moduleName];

        if (ref == NULL) {
            string ScriptText = LoadAndSave::GetScriptByModuleName(moduleName);

            if (ScriptText.size() == 0) {
                return luaL_error(L, "Required module is empty or doesn't exist!");
            }

            if (!Scheduler::YieldOnRequires) {
                if (luaL_loadbufferx(L, ScriptText.c_str(), ScriptText.size(), ("@" + moduleName).c_str(), NULL) != LUA_OK ||
                    lua_pcall(L, 0, 1, 0) != LUA_OK) {
                    return luaL_error(L, ("Required module \"" + moduleName + "\" failed to run: " + (string)lua_tostring(L, -1)).c_str());
                }

                ref = luaL_ref(L, LUA_REGISTRYINDEX);
                Scheduler::luaRequires[moduleName] = ref;
            }
            else {
                lua_State* moduleL = ModuleHasBeenRequired[moduleName];

                if (!ModuleHasBeenRequired[moduleName]) {
                    moduleL = RunScript(ScriptText, moduleName);

                    if (moduleL == 0) {
                        luaL_error(L, ("Required module \"" + moduleName + "\" syntax errored!").c_str());

                        return 0;
                    }

                    ModuleHasBeenRequired[moduleName] = moduleL;
                }

                auto StartTime = Utils::GetMilliseconds();

                auto S = GetScheduler();

                struct RequireYield {
                    string ModuleName = "script";
                    long long Start = 0;
                    lua_State* L = NULL;
                    lua_State* moduleL = NULL;
                };

                RequireYield* Storage = new RequireYield;
                Storage->ModuleName = moduleName;
                Storage->Start = StartTime;
                Storage->L = L;
                Storage->moduleL = moduleL;

                S->Threads[L].cState = 2;

                S->Threads[L].yieldStorage = Storage;
                S->Threads[L].storage = Storage;

                S->Threads[L].yieldFunction = [](void* Storage) {
                    if (Storage == NULL) {
                        return 1;
                    }

                    auto CurrentTime = Utils::GetMilliseconds();

                    RequireYield* Info = (RequireYield*)Storage;

                    int ref = Scheduler::luaRequires[Info->ModuleName];

                    if (ref != NULL) {
                        return 1;
                    }

                    if ((CurrentTime - Info->Start) > 2000) {
                        cout << "\033[1;33m" << "Required module \"" + Info->ModuleName + "\" has taken more than 2 seconds to load. Does it return any values?" << "\033[0m" << endl; // replace with warn function evntually
                        Info->Start = LLONG_MAX;
                    }

                    auto S = GetScheduler();

                    if (S->Threads[Info->moduleL].errored) {
                        lua_Debug ar;
                        if (lua_getstack(Info->L, 1, &ar)) {
                            lua_getinfo(Info->L, "Sl", &ar);
                            lua_pushstring(Info->L, (((string)ar.source).substr(1) + ":" + to_string(ar.currentline) + ": Required module \"" + Info->ModuleName + "\" has errored!").c_str());
                        }
                        else {
                            lua_pushstring(Info->L, (": Required module \"" + Info->ModuleName + "\" has errored!").c_str());
                        }

                        S->Threads[Info->L].returnValues = 0;

                        return 2;
                    }

                    return 0;
                    };

                S->Threads[L].returnFunction = [](lua_State* L, void* Storage) {
                    if (Storage == NULL) {
                        return;
                    }

                    RequireYield* Info = (RequireYield*)Storage;

                    int ref = Scheduler::luaRequires[Info->ModuleName];

                    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

                    lua_pushvalue(L, -1);

                    return;
                    };

                S->Threads[L].returnValues = 1;

                return lua_yield(L, 0);
            }
        }

        lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

        lua_pushvalue(L, -1);

        return 1;
    }

    void register_scheduler_functions(lua_State* L) {
        lua_register(L, "sleep", lua_sleep);
        lua_register(L, "spawn", lua_spawn);
        lua_register(L, "require", lua_require);
    }

    lua_State* GetGlobalState() {
        static lua_State* L = NULL;

        if (L == NULL) {
            L = luaL_newstate();

            if (L == NULL) {
                cout << "\033[1;31mLua state failed to initialize!\033[0m" << endl;

                exit(-1);

                return NULL;
            }

            luaL_openlibs(L);
            InstanceBridge::lua_initInstance(L);
            register_scheduler_functions(L);

            auto World = dynamic_pointer_cast<Instance>(GetGameWorld());

            lua_pushinstance(L, World);
            lua_setglobal(L, "game");

            lua_newtable(L);
            lua_setglobal(L, "_SHARED");
        }

        return L;
    }

    lua_State* RunScript(string Script, string Name, int identity) {
        auto gL = GetGlobalState();

        lua_State* L = Scheduler::GenerateNewThread(gL);

        if (luaL_loadbufferx(L, Script.c_str(), Script.size(), ("@" + Name).c_str(), NULL) != LUA_OK) {
            cout << "\033[1;31m" << lua_tostring(L, -1) << "\033[0m" << endl;
            lua_pop(L, 1);
            return 0;
        }

        lua_settop(gL, 0);

        Scheduler::AddThreadToSchedulerLoop(L, identity, Name);

        return L;
    }
}