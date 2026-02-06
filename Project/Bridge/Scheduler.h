#pragma once

#include <thread>
#include <map>
#include "InstanceBridge.h"
#include "../World/GameWorld.h"
#include "../LoadAndSave/LoadAndSave.h"

namespace Scheduler {
    map<string, int> luaRequires = {};

    bool YieldOnRequires = true;

    struct SchedulerYieldValue {
        int cState = 0;
        // 0 -> null/deleted
        // 1 -> ready to run
        // 2 -> yielding
        
        int returnValues = 0; // number of values to return after yielding
        void* storage = NULL;
        std::function<void(lua_State*, void*)> returnFunction = [](...) {};

        bool errored = false;
        
        string name = "none"; // Only used for requires and on the main thread of a script. DO NOT set it for other stuff

        void* yieldStorage = NULL;
        std::function<int(void*)> yieldFunction = [](...) { return true; };
        // 0 -> keep yielding
        // 1 -> run
        // 2 -> error
    };

    class SchedulerT {
    public:
        map<lua_State*, SchedulerYieldValue> Threads = {};

        bool Active = true;
        int Rate = 10; // number of milliseconds between each scheduler step.

        SchedulerT() {}
    };

    SchedulerT* GetScheduler() {
        static SchedulerT* S = new SchedulerT();

        return S;
    }

    int LuaYieldSchedulerStep() {
        auto Scheduler = GetScheduler();

        int Executed = 0;

        for (auto pair : Scheduler->Threads) {
            lua_State* L = pair.first;
            SchedulerYieldValue state = pair.second;

            int status = lua_status(L);

            if (state.cState == 2) {
                int result = state.yieldFunction(state.yieldStorage);

                if (result != 0)
                    state.cState = 1;

                if (result == 2)
                    status = LUA_ERRRUN;
            }

            if (state.cState == 1) {
                int nres = 0;

                if ((status == LUA_OK || status == LUA_YIELD)) {
                    if (state.returnValues > 0) {
                        state.returnFunction(L, state.storage);

                        Scheduler->Threads[L].returnValues = 0;
                        state.returnFunction = [](...) {};
                        state.yieldFunction = [](...) { return true; };
                    }

                    status = lua_resume(L, 0, state.returnValues, &nres);
                }

                if (status == LUA_YIELD) { /* Do nothing, as we trust the function that yielded. */ }
                else if (status == LUA_OK) {
                    if (state.name != "none" && nres > 0 && Scheduler::YieldOnRequires) {
                        int ReturnReference = luaL_ref(L, LUA_REGISTRYINDEX);

                        luaRequires[state.name] = ReturnReference;
                    }

                    Scheduler->Threads[L].cState = 0; /* Erase thread as the code has finished running. */
                }
                else {
                    string Error = lua_tostring(L, -1);

                    cout << "\033[1;31m" + Error + "\033[0m" << endl;
                    
                    lua_settop(L, 0);

                    Scheduler->Threads[L].cState = 0; /* Erase thread as the code has halted. */
                    Scheduler->Threads[L].errored = true;
                }

                Executed++;
            }
        }

        return Executed;
    }

    namespace Lua {
        lua_State* RunScript(string Script, string Name);

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

            lua_State* nL = lua_newthread(L);

            int thread_ref = luaL_ref(L, LUA_REGISTRYINDEX); // Later add something to collect finished threads like this
            
            lua_pushvalue(L, 1);
            lua_xmove(L, nL, 1);
            
            auto Scheduler = GetScheduler();

            Scheduler->Threads.emplace(nL, 1);

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

        lua_State* RunScript(string Script, string Name = "console") {
            auto gL = GetGlobalState();

            lua_State* L = lua_newthread(gL);

            int ref = luaL_ref(gL, LUA_REGISTRYINDEX);

            if (luaL_loadbufferx(L, Script.c_str(), Script.size(), ("@" + Name).c_str(), NULL) != LUA_OK) {
                cout << "\033[1;31m" << lua_tostring(L, -1) << "\033[0m" << endl;
                lua_pop(L, 1);
                luaL_unref(gL, LUA_REGISTRYINDEX, ref);
                return 0;
            }

            lua_settop(gL, 0);

            auto Scheduler = GetScheduler();

            SchedulerYieldValue Value = { 1 };
            Value.name = Name;

            Scheduler->Threads.emplace(L, Value);

            return L;
        }

    }

    namespace Event {
        enum FiredEventParameterType {
            TInstance,
            TNumber,
            TString,
            TBoolean,
            TInteger,
            TVector
        };

        struct FiredEventParameter {
            FiredEventParameterType Type;
            LuaVector v;
            shared_ptr<Instance> o;
            double n = 0.0;
            long i = 0;
            string s;
        };

        template <typename T>
        FiredEventParameter GetEventParamFromT(T Value) {
            FiredEventParameter Object;

            if (typeid(T).hash_code() == typeid(LuaVector).hash_code()) {
                Object.Type = TVector;
                Object.v = *(LuaVector*)&Value;
            }
            else if (typeid(T).hash_code() == typeid(shared_ptr<Instance>).hash_code()) {
                Object.Type = TInstance;
                Object.o = *(shared_ptr<Instance>*)&Value;
            }
            else if (typeid(T).hash_code() == typeid(string).hash_code()) {
                Object.Type = TString;
                Object.s = *(string*)&Value;
            }
            else if (typeid(T).hash_code() == typeid(double).hash_code()) {
                Object.Type = TNumber;
                Object.n = *(double*)&Value;
            }
            else if (typeid(T).hash_code() == typeid(long).hash_code()) {
                Object.Type = TInteger;
                Object.i = *(long*)&Value;
            }
            else if (typeid(T).hash_code() == typeid(int).hash_code()) {
                Object.Type = TInteger;
                Object.i = *(int*)&Value;
            }
            else if (typeid(T).hash_code() == typeid(bool).hash_code()) {
                Object.Type = TBoolean;
                Object.i = *(bool*)&Value;
            }
            else {
                string what = typeid(T).name();

                what = "Incompatible type: " + what;

                throw std::runtime_error(what);
            }

            return Object;
        }

        void lua_pusheventparam(lua_State* L, FiredEventParameter Param) {
            if (Param.Type == TInstance)
                lua_pushinstance(L, Param.o);
            else if (Param.Type == TInteger)
                lua_pushinteger(L, Param.i);
            else if (Param.Type == TNumber)
                lua_pushinteger(L, Param.n);
            else if (Param.Type == TString)
                lua_pushstring(L, Param.s.c_str());
            else if (Param.Type == TBoolean)
                lua_pushboolean(L, (int)Param.i);
            else if (Param.Type == TVector)
                lua_pushvector(L, Param.v);
            else
                throw std::runtime_error("Invalid event parameter type.");
        }

        int FireListenerID(int ListenerID, vector<FiredEventParameter> Parameters = {}) {
            auto Scheduler = Scheduler::GetScheduler();

            auto gL = Lua::GetGlobalState();

            if (LuaEvent::ListeningFunctions[ListenerID].size() == 0) {
                return 0;
            }

            // Note: whatever you do to this code, or any code with global state for that matter,
            //       be extremely careful because it can lead to the program corrupting and crashing
            //       with crash logs that don't make sense and that'll just be a waste of time.
            //       Thanks -- lightersmash

            for (auto f : (LuaEvent::ListeningFunctions[ListenerID])) {
                lua_rawgeti(gL, LUA_REGISTRYINDEX, f.function_reference);

                lua_State* nL = lua_newthread(gL);

                int thread_ref = luaL_ref(gL, LUA_REGISTRYINDEX); // Later add something to collect finished threads like this

                lua_pushvalue(gL, -1);
                lua_xmove(gL, nL, 1);

                lua_settop(gL, 0);

                for (auto o : Parameters) {
                    lua_pusheventparam(nL, o);
                }

                SchedulerYieldValue Value = { 1, int(Parameters.size()) };

                Scheduler->Threads.emplace(nL, Value);
            }

            return 0;
        }

        void FireListenerInstance(Instance* Object, const char* EventName, vector<FiredEventParameter> Parameters = {}) {
            if (Object == NULL) {
                return;
            }

            string Type = Object->GetType();

            EventDescriptor* LOL = ClassEventDescriptorList[Type][EventName];

            if (!LOL) {
                return;
            }

            int ListeningID = *(LOL->GetFunction(Object));

            if (ListeningID != 0) {
                FireListenerID(ListeningID, Parameters);
            }
        }
    }

    int SchedulerStep() {
        auto Scheduler = GetScheduler();

        if (Scheduler && Scheduler->Active) {
            LuaYieldSchedulerStep();
        }

        return 0;
    }

    int Start() {
        GetScheduler()->Active = true;

        // Don't start a new thread anymore because it just won't be thread-safe with how things are working right now

        return 1;
    }

    int Stop() {
        GetScheduler()->Active = false;

        return 1;
    }
}

void ChildAddedEventFirer(shared_ptr<Instance> Object, shared_ptr<Instance> Child) {
    Scheduler::Event::FireListenerInstance(Object.get(), "ChildAdded", {Scheduler::Event::GetEventParamFromT(Child)});
}

void ChildRemovedEventFirer(shared_ptr<Instance> Object, shared_ptr<Instance> Child) {
    Scheduler::Event::FireListenerInstance(Object.get(), "ChildRemoved", { Scheduler::Event::GetEventParamFromT(Child) });
}

void DescendantAddedEventFirer(shared_ptr<Instance> Object, shared_ptr<Instance> Child) {
    Scheduler::Event::FireListenerInstance(Object.get(), "DescendantAdded", { Scheduler::Event::GetEventParamFromT(Child) });
}

void DescendantRemovedEventFirer(shared_ptr<Instance> Object, shared_ptr<Instance> Child) {
    Scheduler::Event::FireListenerInstance(Object.get(), "DescendantRemoved", { Scheduler::Event::GetEventParamFromT(Child) });
}

void DestroyingEventFirer(shared_ptr<Instance> Object) {
    Scheduler::Event::FireListenerInstance(Object.get(), "Destroying");
}

void ChangedEventFirer(shared_ptr<Instance> Object, string PropertyName) {
    Scheduler::Event::FireListenerInstance(Object.get(), "Changed", {Scheduler::Event::GetEventParamFromT(PropertyName)});
}