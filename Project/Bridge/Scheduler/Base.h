#pragma once

#include <thread>
#include <map>
#include "../InstanceBridge.h"
#include "../../World/GameWorld.h"
#include "../../LoadAndSave/LoadAndSave.h"

using namespace std;

namespace Scheduler {
    map<int, map<string, int>> luaRequires = {}; 
    // Index 1 = Identity number, Index 2 = name, value 2 = require return value lua ref index

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

        int threadIdentity = 0;
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

                        luaRequires[state.threadIdentity][state.name] = ReturnReference;
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

    void AddThreadToSchedulerLoop(lua_State* thread, int identity = 0, string name = "none", int returnValues = 0) {
        auto S = Scheduler::GetScheduler();

        Scheduler::SchedulerYieldValue Value;

        Value.cState = 1;
        Value.threadIdentity = identity;
        Value.name = name;
        Value.returnValues = returnValues;

        S->Threads.emplace(thread, Value);
    }

    void AddThreadToSchedulerLoop(lua_State* thread, lua_State* parentThread, string name = "none") {
        auto S = Scheduler::GetScheduler();

        auto ParentIdentity = S->Threads[parentThread].threadIdentity;

        Scheduler::SchedulerYieldValue Value;

        Value.cState = 1;
        Value.threadIdentity = ParentIdentity;
        Value.name = name;

        S->Threads.emplace(thread, Value);
    }

    lua_State* GenerateNewThread(lua_State* sourceState, int functionCopyFromStack = 0, bool removeFunctionCopyAfter = false) {
        if (sourceState == NULL) {
            return NULL;
        }

        lua_State* nL = lua_newthread(sourceState);

        int thread_ref = luaL_ref(sourceState, LUA_REGISTRYINDEX); // So the thread doesn't die due to garbage collection.

        if (functionCopyFromStack != 0) {
            lua_pushvalue(sourceState, functionCopyFromStack);
            lua_xmove(sourceState, nL, 1);

            // These two lines will move the function that was at parameter 1 (bottom of the stack + 1),
            // into the new thread so that it can be called and actually do something.

            if (removeFunctionCopyAfter) {
                lua_pop(sourceState, functionCopyFromStack);
            }
        }

        return nL;
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

    void ExitAllThreadsWithIdentity(int Identity) {
        auto Scheduler = GetScheduler();

        for (auto& Index : Scheduler->Threads) {
            if (Index.second.threadIdentity == Identity) {
                Scheduler->Threads.erase(Index.first);
            }
        }
    }
}