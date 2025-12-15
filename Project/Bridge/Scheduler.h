#pragma once

#include <thread>
#include <map>
#include "InstanceBridge.h"
#include "../World/GameWorld.h"

namespace Scheduler {
    struct SchedulerYieldValue {
        int cState = 0;
        // 0 -> null/deleted
        // 1 -> ready to run
        // 2 -> yielding
        int returnValues = 0; // number of values to return after yielding
        void* storage = NULL;
        std::function<void(lua_State*, void*)> returnFunction = [](...) {};

        void* yieldStorage = NULL;
        std::function<bool(void*)> yieldFunction = [](...) { return true; };
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

            if (state.cState == 2)
                if (state.yieldFunction(state.yieldStorage))
                    state.cState = 1;

            if (state.cState == 1) {
                int nres = 0;

                int status = lua_status(L);

                if (lua_status(L) == LUA_OK || lua_status(L) == LUA_YIELD) {
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
                    Scheduler->Threads[L].cState = 0; /* Erase thread as the code has finished running. */
                }
                else {
                    string Error = lua_tostring(L, -1);

                    cout << "ERROR: " << Error << endl;
                    
                    lua_settop(L, 0);

                    Scheduler->Threads[L].cState = 0; /* Erase thread as the code has halted. */
                }

                Executed++;
            }
        }

        return Executed;
    }

    namespace Lua {
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

        void register_scheduler_functions(lua_State* L) {
            lua_register(L, "sleep", lua_sleep);
            lua_register(L, "spawn", lua_spawn);
        }

        lua_State* GetGlobalState() {
            static lua_State* L = NULL;

            if (L == NULL) {
                L = luaL_newstate();

                if (L == NULL)
                    throw std::exception("Lua state failed to initialize!");

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

        int RunScript(string Script) {
            auto gL = GetGlobalState();

            lua_State* L = lua_newthread(gL);

            int ref = luaL_ref(gL, LUA_REGISTRYINDEX);
            
            if (luaL_loadstring(L, Script.c_str()) != LUA_OK) {
                cout << "FATAL ERROR: FAILED TO LOADSTRING" << endl;
                lua_pop(L, 1);
                return 0;
            }

            lua_settop(gL, 0);

            auto Scheduler = GetScheduler();

            Scheduler->Threads.emplace(L, 1);

            return 0;
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

                SchedulerYieldValue Value = { 1, Parameters.size() };

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